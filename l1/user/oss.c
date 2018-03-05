/*****************************************************************************/

/*
 *      oss.c  --  Linux sound I/O.
 *
 *      Copyright (C) 1997  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *        Swiss Federal Institute of Technology (ETH), Electronics Lab
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  This is the Linux realtime sound output driver
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <asm/byteorder.h>
#include <sys/soundcard.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>
#include <endian.h>
#include <pthread.h>
#include <stdlib.h>
#include "os.h"
#include "l1.h"
#include "l1user.h"

/* --------------------------------------------------------------------- */
/*
 * Sample output
 */
static int samples_remain = 0;
static int samples_count = 0;
static short *samples_ptr = NULL;

static int invert_ptt = 0;

static pthread_t thr_l1;

static float snd_corr;

/* --------------------------------------------------------------------- */

static int fd_ptt = -1;
static int fd_mixer = -1;

/* --------------------------------------------------------------------- */

#if __BYTE_ORDER == __BIG_ENDIAN
#define AUDIO_FMT AFMT_S16_BE
#else
#define AUDIO_FMT AFMT_S16_LE
#endif

/* --------------------------------------------------------------------- */

extern __inline__ void output_ptt(int ptt)
{
	int par = TIOCM_RTS;

	if (fd_ptt < 0)
		return;
	if (ioctl(fd_ptt, ((ptt && !invert_ptt) || (!ptt && invert_ptt)) ? TIOCMBIS : TIOCMBIC, &par))
		errstr(SEV_FATAL, "ioctl: TIOCMBI[CS]");
}

/* --------------------------------------------------------------------- */

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif

#define NTESTS 4
#define FULL_MMAP

static int probe_oss(const char *name_audio)
{
        size_t pagesize = getpagesize();
	int fd_audio, apar, i;
	struct audio_buf_info iinfo, oinfo;
	struct timeval t1, t2;
        unsigned int times[NTESTS];
        unsigned int isize;
        caddr_t ibuf, ibufe;
        unsigned int avg, tm;

        if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
                errstr(SEV_FATAL, "open");
        /*
         * configure audio
         */
        apar = AUDIO_FMT;
        if (ioctl(fd_audio, SNDCTL_DSP_SETFMT, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFMT");
        if (apar != AUDIO_FMT) 
		errprintf(SEV_FATAL, "audio driver does not support the S16 format\n");
        apar = 0;
        if (ioctl(fd_audio, SNDCTL_DSP_STEREO, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_STEREO");
        if (apar != 0)
		errprintf(SEV_FATAL, "audio driver does not support mono\n");
        apar = SAMPLE_RATE;
        if (ioctl(fd_audio, SNDCTL_DSP_SPEED, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SPEED");
	if (apar != SAMPLE_RATE) {
		if (abs(apar-SAMPLE_RATE) >= SAMPLE_RATE/100)
			errprintf(SEV_FATAL, "audio driver does not support 8kHz sampling frequency\n");
		errprintf(SEV_WARNING, "audio driver inexact sampling rate: %d Hz (%d requested)\n", 
			  apar, SAMPLE_RATE);
	}
        /*
         * set fragment sizes
         */
        apar = 0xffff0007U;
        if (ioctl(fd_audio, SNDCTL_DSP_SETFRAGMENT, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFRAGMENT");
	if (ioctl(fd_audio, SNDCTL_DSP_GETOSPACE, &oinfo) == -1)
		errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETOSPACE");
	if (ioctl(fd_audio, SNDCTL_DSP_GETISPACE, &iinfo) == -1)
		errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETISPACE");
	if (iinfo.fragsize > pagesize)
		errprintf(SEV_FATAL, "OSS: input: fragment size %d > page size %d\n",
			  iinfo.fragsize, pagesize);
        /*
         * determine if the driver is capable to support our access method
         */
        if (ioctl(fd_audio, SNDCTL_DSP_GETCAPS, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETCAPS");
        if (!(apar & DSP_CAP_TRIGGER) || !(apar & DSP_CAP_MMAP))
                errprintf(SEV_FATAL, "Sound driver does not support mmap and/or trigger\n");
	close(fd_audio);
#ifndef OSS_DISABLE_FDX
	if (apar & DSP_CAP_DUPLEX) {
		errprintf(SEV_INFO, "Your soundcard is full duplex capable. Good; the reference time will be\n"
			  "derived from the sample clock, which should be accurate within +-20ppm.\n"
			  "So I hope your soundcard uses 'real' crystals instead of cheap ceramic\n"
			  "resonators. Adjust the clock with the soundcard correction factor.\n");
		return 1;
	}
#endif /* OSS_DISABLE_FDX */
	for (i = 0; i < 10; i++) {
		if (gettimeofday(&t1, NULL))
			errstr(SEV_FATAL, "gettimeofday");
		if (gettimeofday(&t2, NULL))
			errstr(SEV_FATAL, "gettimeofday");
		if (((1000000+t2.tv_usec-t1.tv_usec) % 1000000) > 1)
			break;
	}
	if (i >= 10)
		errprintf(SEV_FATAL, "Your gettimeofday resolution seems to be inappropriate\n");
        for (i = 0; i < NTESTS; i++) {
                if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
                        errstr(SEV_FATAL, "open");
                apar = AUDIO_FMT;
                if (ioctl(fd_audio, SNDCTL_DSP_SETFMT, &apar) == -1)
                        errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFMT");
                if (apar != AUDIO_FMT) 
                        errprintf(SEV_FATAL, "audio driver does not support AFMT_S16_LE");
                apar = 0;
                if (ioctl(fd_audio, SNDCTL_DSP_STEREO, &apar) == -1)
                        errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_STEREO");
                if (apar != 0)
                        errprintf(SEV_FATAL, "audio driver does not support mono");
                apar = SAMPLE_RATE;
                if (ioctl(fd_audio, SNDCTL_DSP_SPEED, &apar) == -1)
                        errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SPEED");
                if (apar != SAMPLE_RATE) 
                        if (abs(apar-SAMPLE_RATE) >= SAMPLE_RATE/100)
                                errprintf(SEV_FATAL, "audio driver does not support required sampling frequency");
                apar = 0xffff0007U;
                if (ioctl(fd_audio, SNDCTL_DSP_SETFRAGMENT, &apar) == -1)
                        errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFRAGMENT");
                if (ioctl(fd_audio, SNDCTL_DSP_GETISPACE, &iinfo) == -1)
                        errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETISPACE");
                isize = iinfo.fragstotal * iinfo.fragsize;
                if (ioctl(fd_audio, SNDCTL_DSP_GETCAPS, &apar) == -1)
                        errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETCAPS");
                if (!(apar & DSP_CAP_TRIGGER) || !(apar & DSP_CAP_MMAP))
                        errprintf(SEV_FATAL, "Sound driver does not support mmap and/or trigger");
#ifdef FULL_MMAP
                if ((ibuf = mmap(NULL, pagesize+isize, PROT_READ | PROT_WRITE, 
				 MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
                        errstr(SEV_FATAL, "mmap: MAP_ANONYMOUS");
                ibufe = ibuf + isize;
                if (munmap(ibuf, isize))
                        errstr(SEV_FATAL, "munmap: MAP_ANONYMOUS");
                if ((ibuf = mmap(ibuf, isize, PROT_READ, MAP_FILE | MAP_SHARED | MAP_FIXED,  
				 fd_audio, 0)) == MAP_FAILED)
                        errstr(SEV_FATAL, "mmap: PROT_READ");
#else /* FULL_MMAP */
                if ((ibuf = mmap(NULL, isize, PROT_READ, MAP_FILE | MAP_SHARED | MAP_FIXED,  
				 fd_audio, 0)) == MAP_FAILED)
                        errstr(SEV_FATAL, "mmap: PROT_READ");
#endif /* FULL_MMAP */
                apar = 0;
                if (ioctl(fd_audio, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
                        errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETTRIGGER");
                apar = PCM_ENABLE_INPUT;
                if (ioctl(fd_audio, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
                        errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETTRIGGER");

                if (gettimeofday(&t1, NULL))
                        errstr(SEV_FATAL, "gettimeofday");
                times[i] = t1.tv_usec;

                if (munmap(ibuf, isize))
                        errstr(SEV_FATAL, "munmap");
#ifdef FULL_MMAP
                if (munmap(ibufe, pagesize))
                        errstr(SEV_FATAL, "munmap");
#endif /* FULL_MMAP */
                if (close(fd_audio))
                        errstr(SEV_FATAL, "close");
	}
        for (avg = 0, i = 1; i < NTESTS; i++)
		avg += (tm = (1000000+times[i]-times[i-1]) % 1000000);
	avg = (avg + NTESTS/2) / NTESTS;
	errprintf(SEV_INFO, "Your soundcard is not full duplex capable. This is bad; but we will try half\n"
		  "duplex mode. Now, the soundcard is switched between RX and TX. This lasts %dms\n"
		  "on average. I will try to hide this latency within the txdelay, so set it to\n"
		  "some value larger than this. And let's hope propagation and txdelay of your\n"
		  "peers also lasts longer...\n", avg);
	return 0;
}

/* --------------------------------------------------------------------- */

#define EXCESS_FRAGS 3

static void *fdx_driver(void *name_audio)
{
        size_t pagesize = getpagesize();
        int apar, fd_audio, i;
	union {
		caddr_t v;
		short *s;
	} ibuf;
	caddr_t ibuf_ext;
	union {
		caddr_t v;
		short *s;
	} obuf;
	struct audio_buf_info iinfo, oinfo;
	unsigned int isize, osize;
	unsigned int ifragptr, ofragptr;
	fd_set rmask, wmask;
	struct count_info cinfo;
	unsigned int curfrag, lastfrag, nfrags;
	unsigned long long itime, otime, inc_fragment;
	l1_time_t inc_sample;
	int ptt_frames;
	short *s;
	
        if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
                errstr(SEV_FATAL, "open");
        /*
         * configure audio
         */
        apar = AUDIO_FMT;
        if (ioctl(fd_audio, SNDCTL_DSP_SETFMT, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFMT");
        if (apar != AUDIO_FMT) 
		errprintf(SEV_FATAL, "audio driver does not support the S16 format\n");
        apar = 0;
        if (ioctl(fd_audio, SNDCTL_DSP_STEREO, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_STEREO");
        if (apar != 0)
		errprintf(SEV_FATAL, "audio driver does not support mono\n");
        apar = SAMPLE_RATE;
        if (ioctl(fd_audio, SNDCTL_DSP_SPEED, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SPEED");
	if (apar != SAMPLE_RATE) {
		if (abs(apar-SAMPLE_RATE) >= SAMPLE_RATE/100)
			errprintf(SEV_FATAL, "audio driver does not support 8kHz sampling frequency\n");
	}
	inc_sample = 1000000.0*snd_corr/apar;
	inc_fragment = 64*1000000.0*snd_corr/apar*(1<<24);
        ifragptr = 0;
	itime = 0;
	ofragptr = 1;
	otime = ofragptr * inc_fragment;
	ptt_frames = 0;
	lastfrag = 0;
        /*
         * set fragment sizes
         */
        apar = 0xffff0007U;
        if (ioctl(fd_audio, SNDCTL_DSP_SETFRAGMENT, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFRAGMENT");
	if (ioctl(fd_audio, SNDCTL_DSP_GETOSPACE, &oinfo) == -1)
		errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETOSPACE");
	osize = oinfo.fragstotal * oinfo.fragsize;
	if (ioctl(fd_audio, SNDCTL_DSP_GETISPACE, &iinfo) == -1)
		errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETISPACE");
	isize = iinfo.fragstotal * iinfo.fragsize;
	if (EXCESS_FRAGS * iinfo.fragsize > pagesize)
		errprintf(SEV_FATAL, "OSS: input: fragment size %d times excess frags > page size %d\n",
			  iinfo.fragsize, pagesize);
	/*
         * mmap buffers
         *
	 * BSD people attention: you may need to uncomment the PROT_READ
	 * feedback welcome: sailer@ife.ee.ethz.ch
	 */
	if ((ibuf.v = mmap(NULL, pagesize+isize, PROT_READ | PROT_WRITE, 
			   MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		errstr(SEV_FATAL, "mmap: MAP_ANONYMOUS");
	ibuf_ext = ibuf.v + isize;
	if (munmap(ibuf.v, isize))
		errstr(SEV_FATAL, "munmap: MAP_ANONYMOUS");
	if ((ibuf.v = mmap(ibuf.v, isize, PROT_READ, MAP_FILE | MAP_SHARED | MAP_FIXED, 
			  fd_audio, 0)) == MAP_FAILED)
		errstr(SEV_FATAL, "mmap: PROT_READ");
	if ((obuf.v = mmap(NULL, osize, PROT_WRITE /* | PROT_READ*/, 
			   MAP_FILE | MAP_SHARED, fd_audio, 0)) == MAP_FAILED)
		errstr(SEV_FATAL, "mmap: PROT_WRITE");
	errprintf(SEV_INFO, "OSS: output: #frag: %d  fragsz: %d  totbuf: %d  bufaddr: %p\n"
		  "OSS: input: #frag: %d  fragsz: %d  totbuf: %d  bufaddr: %p  mempage: %p\n"
		  "OSS: sample time increment: %u  fragment time increment: %u\n",
		  oinfo.fragstotal, oinfo.fragsize, osize, obuf.s,
		  iinfo.fragstotal, iinfo.fragsize, isize, ibuf.s, ibuf_ext, 
		  (unsigned int)inc_sample, (unsigned int)(inc_fragment >> 24));
        /*
         * start playback/recording
         */
        apar = 0;
        if (ioctl(fd_audio, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETTRIGGER");
	apar = PCM_ENABLE_OUTPUT | PCM_ENABLE_INPUT;
        if (ioctl(fd_audio, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETTRIGGER");
	/*
	 * loop
	 */
	for (;;) {
		FD_ZERO(&rmask);
		FD_ZERO(&wmask);
		FD_SET(fd_audio, &rmask);
		FD_SET(fd_audio, &wmask);
		i = select(fd_audio+1, &rmask, &wmask, NULL, NULL);
		if (i < 0) 
			errstr(SEV_FATAL, "select");
		if (!FD_ISSET(fd_audio, &rmask) && !FD_ISSET(fd_audio, &wmask))
			continue;
		/*
		 * process input
		 */
		if (ioctl(fd_audio, SNDCTL_DSP_GETIPTR, &cinfo) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETIPTR");
		curfrag = cinfo.ptr / iinfo.fragsize;
		if (cinfo.blocks > 2)
			errprintf(SEV_WARNING, "%d fragments passed since last wakeup\n", cinfo.blocks);
		while (ifragptr != curfrag) {
			if (!ifragptr) {
				s = (short *)(ibuf.v + isize + iinfo.fragsize * ifragptr);
				memcpy(s, ibuf.v + iinfo.fragsize * ifragptr, iinfo.fragsize);
			} else
				s = (short *)(ibuf.v + iinfo.fragsize * ifragptr);
			if (samples_remain > 0) {
				i = 2*samples_remain;
				if (i > iinfo.fragsize)
					i = iinfo.fragsize;
				memcpy(samples_ptr, s, i);
				samples_ptr += i/2;
				samples_remain -= i/2;
			}
			l1_input_samples(itime >> 24, inc_sample, s, iinfo.fragsize/2);
			itime += inc_fragment;			
			ifragptr++;
			if (ifragptr >= iinfo.fragstotal)
				ifragptr = 0;
		}
		/*
		 * process output
		 */
		if (ioctl(fd_audio, SNDCTL_DSP_GETOPTR, &cinfo) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETOPTR");
		curfrag = cinfo.ptr / oinfo.fragsize;
		nfrags = oinfo.fragstotal + curfrag - lastfrag;
		lastfrag = curfrag;
		if (nfrags >= oinfo.fragstotal)
			nfrags -= oinfo.fragstotal;
		if (nfrags != cinfo.blocks)
			errprintf(SEV_WARNING, "OSS sound driver lost interrupt!\n");
		if (nfrags > 2)
			errprintf(SEV_WARNING, "%d fragments passed since last wakeup\n", nfrags);
		ptt_frames -= nfrags;
		if (ptt_frames < 0) {
			otime += (-ptt_frames) * inc_fragment;
			ofragptr -= ptt_frames;
			while (ofragptr >= oinfo.fragstotal)
				ofragptr -= oinfo.fragstotal;
			ptt_frames = 0;
			memset(obuf.s, 0, oinfo.fragsize * oinfo.fragstotal);
		}
		/* sanity check */
		if (!ptt_frames && ofragptr != (curfrag + 1) % oinfo.fragstotal)
			errprintf(SEV_FATAL, "output pointers inconsistent %u %u %lu\n",
				  ofragptr, curfrag, (unsigned long)(otime >> 24));
		while (ptt_frames < oinfo.fragstotal && ptt_frames <= 4 && 
		       l1_output_samples(otime >> 24, inc_sample, (short *)(obuf.v + oinfo.fragsize * ofragptr), 
					 oinfo.fragsize/2)) {
			ofragptr++;
			if (ofragptr >= oinfo.fragstotal)
				ofragptr = 0;
			ptt_frames++;
			otime += inc_fragment;
		}
		output_ptt(ptt_frames > 0);
	}
}

/* --------------------------------------------------------------------- */

static void *hdx_driver(void *name_audio)
{
        size_t pagesize = getpagesize();
        int apar, fd_audio, i;
	union {
		caddr_t v;
		short *s;
	} buf;
	caddr_t buf_ext;
	struct audio_buf_info info;
	unsigned int size;
	unsigned int fragptr;
	l1_time_t inc_sample;
	l1_time_t inc_fragment;
	fd_set mask;
	struct count_info cinfo;
	unsigned int curfrag, lastfrag, nfrags;
	l1_time_t l1time = 0;
	unsigned int last_dmaptr;
	int ptt_frames;
	short *s;

	refclock_init();
	for (;;) {
		/*
		 * start receiver
		 */
		output_ptt(0);
		if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
			errstr(SEV_FATAL, "open");
		/*
		 * configure audio
		 */
		apar = AUDIO_FMT;
		if (ioctl(fd_audio, SNDCTL_DSP_SETFMT, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFMT");
		if (apar != AUDIO_FMT) 
			errprintf(SEV_FATAL, "audio driver does not support the S16 format\n");
		apar = 0;
		if (ioctl(fd_audio, SNDCTL_DSP_STEREO, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_STEREO");
		if (apar != 0)
			errprintf(SEV_FATAL, "audio driver does not support mono\n");
		apar = SAMPLE_RATE;
		if (ioctl(fd_audio, SNDCTL_DSP_SPEED, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SPEED");
		if (apar != SAMPLE_RATE) {
			if (abs(apar-SAMPLE_RATE) >= SAMPLE_RATE/100)
				errprintf(SEV_FATAL, "audio driver does not support the required sampling frequency\n");
			errprintf(SEV_WARNING, "audio driver inexact sampling rate: %d Hz\n", apar);
		}
		inc_sample = (1000000+apar/2)/apar;
		inc_fragment = (64*1000000+apar/2)/apar;
		fragptr = 0;
		/*
		 * set fragment sizes
		 */
		apar = 0xffff0007U;
		if (ioctl(fd_audio, SNDCTL_DSP_SETFRAGMENT, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFRAGMENT");
		if (ioctl(fd_audio, SNDCTL_DSP_GETISPACE, &info) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETISPACE");
		size = info.fragstotal * info.fragsize;
		if (info.fragsize > pagesize)
			errprintf(SEV_FATAL, "OSS: input: fragment size %d > page size %d\n",
				  info.fragsize, pagesize);
		/*
		 * mmap buffers
		 */
		if ((buf.v = mmap(NULL, pagesize+size, PROT_READ | PROT_WRITE, 
				   MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			errstr(SEV_FATAL, "mmap: MAP_ANONYMOUS");
		buf_ext = buf.v + size;
		if (munmap(buf.v, size))
			errstr(SEV_FATAL, "munmap: MAP_ANONYMOUS");
		if ((buf.v = mmap(buf.v, size, PROT_READ, MAP_FILE | MAP_SHARED | MAP_FIXED, 
				   fd_audio, 0)) == MAP_FAILED)
			errstr(SEV_FATAL, "mmap: PROT_READ");
		errprintf(SEV_INFO, "OSS: input: #frag: %d  fragsz: %d  totbuf: %d  bufaddr: %p  mempage: %p\n"
			  "OSS: sample time increment: %u  fragment time increment: %u  time: %8ld\n",
			  info.fragstotal, info.fragsize, size, buf.s, buf_ext, 
			  (unsigned int)inc_sample, (unsigned int)inc_fragment, l1time);
		/*
		 * start recording
		 */
		apar = 0;
		if (ioctl(fd_audio, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETTRIGGER");
		apar = PCM_ENABLE_INPUT;
		if (ioctl(fd_audio, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETTRIGGER");
		/*
		 * clock adjust
		 */
		if (ioctl(fd_audio, SNDCTL_DSP_GETIPTR, &cinfo) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETIPTR");
		last_dmaptr = cinfo.ptr;
		refclock_current(0, 0);
		/*
		 * loop
		 */
		for (;;) {
			FD_ZERO(&mask);
			FD_SET(fd_audio, &mask);
			i = select(fd_audio+1, &mask, NULL, NULL, NULL);
			if (i < 0) 
				errstr(SEV_FATAL, "select");
			if (!FD_ISSET(fd_audio, &mask))
				continue;
			/*
			 * process input
			 */
			if (ioctl(fd_audio, SNDCTL_DSP_GETIPTR, &cinfo) == -1)
				errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETIPTR");
			l1time = refclock_current(((size + cinfo.ptr - last_dmaptr) % size) / 2 * inc_sample, 1);
			curfrag = (last_dmaptr = cinfo.ptr) / info.fragsize;
			l1time -= inc_sample * ((size + cinfo.ptr - fragptr * info.fragsize) % size) / 2;
			if (cinfo.blocks > 2)
				errprintf(SEV_WARNING, "%d fragments passed since last wakeup\n", cinfo.blocks);
			while (fragptr != curfrag) {
				if (!fragptr) {
					s = (short *)(buf.v + size);
					memcpy(s, buf.v, info.fragsize);
				} else
					s = (short *)(buf.v + info.fragsize * fragptr);
				if (samples_remain > 0) {
					i = 2*samples_remain;
					if (i > info.fragsize)
						i = info.fragsize;
					memcpy(samples_ptr, s, i);
					samples_ptr += i/2;
					samples_remain -= i/2;
				}
				l1_input_samples(l1time, inc_sample, s, info.fragsize/2);
				l1time += inc_fragment;
				fragptr++;
				if (fragptr >= info.fragstotal)
					fragptr = 0;
			}
			/*
			 * check for output
			 */
			if (l1_next_tx_event(l1time) <= (long)inc_fragment/2)
				break;
		}
		if (munmap(buf.v, size))
			errstr(SEV_FATAL, "munmap");
		if (munmap(buf_ext, pagesize))
			errstr(SEV_FATAL, "munmap");
		if (close(fd_audio))
			errstr(SEV_FATAL, "close");
		/*
		 * start transmitter
		 */
		output_ptt(1);
		if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
			errstr(SEV_FATAL, "open");
		/*
		 * configure audio
		 */
		apar = AUDIO_FMT;
		if (ioctl(fd_audio, SNDCTL_DSP_SETFMT, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFMT");
		if (apar != AUDIO_FMT) 
			errprintf(SEV_FATAL, "audio driver does not support the S16 format\n");
		apar = 0;
		if (ioctl(fd_audio, SNDCTL_DSP_STEREO, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_STEREO");
		if (apar != 0)
			errprintf(SEV_FATAL, "audio driver does not support mono\n");
		apar = SAMPLE_RATE;
		if (ioctl(fd_audio, SNDCTL_DSP_SPEED, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SPEED");
		if (apar != SAMPLE_RATE) {
			if (abs(apar-SAMPLE_RATE) >= SAMPLE_RATE/100)
				errprintf(SEV_FATAL, "audio driver does not support the required sampling frequency\n");
		}
		inc_sample = (1000000+apar/2)/apar;
		inc_fragment = (64*1000000+apar/2)/apar;
		/*
		 * set fragment sizes
		 */
		apar = 0xffff0007U;
		if (ioctl(fd_audio, SNDCTL_DSP_SETFRAGMENT, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFRAGMENT");
		if (ioctl(fd_audio, SNDCTL_DSP_GETOSPACE, &info) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETOSPACE");
		size = info.fragstotal * info.fragsize;
		/*
		 * mmap buffers
		 *
		 * BSD people attention: you may need to uncomment the PROT_READ
		 * feedback welcome: sailer@ife.ee.ethz.ch
		 */
		if ((buf.v = mmap(NULL, size, PROT_WRITE /* | PROT_READ*/, 
				   MAP_FILE | MAP_SHARED, fd_audio, 0)) == MAP_FAILED)
			errstr(SEV_FATAL, "mmap: PROT_WRITE");
		errprintf(SEV_INFO, "OSS: output: #frag: %d  fragsz: %d  totbuf: %d  bufaddr: %p\n"
			  "OSS: sample time increment: %u  fragment time increment: %u  time: %8ld\n",
			  info.fragstotal, info.fragsize, size, buf.s,
			  (unsigned int)inc_sample, (unsigned int)inc_fragment, l1time);
		/*
		 * start playback
		 */
		apar = 0;
		if (ioctl(fd_audio, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETTRIGGER");
		apar = PCM_ENABLE_OUTPUT;
		if (ioctl(fd_audio, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETTRIGGER");
		/*
		 * clock adjust
		 */
		if (ioctl(fd_audio, SNDCTL_DSP_GETOPTR, &cinfo) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETOPTR");
		l1time = refclock_current(0, 0);
		last_dmaptr = cinfo.ptr;
		l1time -= cinfo.ptr / 2 * inc_sample;
		/*
		 * fill first two fragments
		 */
		ptt_frames = 1;
		for (i = 0; i < 4 && i < info.fragstotal; i++) 
			if (l1_output_samples(l1time+i*inc_fragment, inc_sample, 
					      (short *)(buf.v + info.fragsize * i), 
					      info.fragsize/2))
				ptt_frames = i+1;
		lastfrag = 0;
		/*
		 * finish already pending rx requests
		 */
		l1_finish_pending_rx_requests();
		/*
		 * loop
		 */
		for (;;) {
			FD_ZERO(&mask);
			FD_SET(fd_audio, &mask);
			i = select(fd_audio+1, NULL, &mask, NULL, NULL);
			if (i < 0) 
				errstr(SEV_FATAL, "select");
			if (!FD_ISSET(fd_audio, &mask))
				continue;
			/*
			 * process output
			 */
			if (ioctl(fd_audio, SNDCTL_DSP_GETOPTR, &cinfo) == -1)
				errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETOPTR");
			l1time = refclock_current(((size + cinfo.ptr - last_dmaptr) % size) / 2 * inc_sample, 1);
			curfrag = (last_dmaptr = cinfo.ptr) / info.fragsize;
			l1time -= inc_sample * ((size + cinfo.ptr - curfrag * info.fragsize) % size) / 2;
			nfrags = info.fragstotal + curfrag - lastfrag;
			lastfrag = curfrag;
			if (nfrags >= info.fragstotal)
				nfrags -= info.fragstotal;
			if (nfrags != cinfo.blocks)
				errprintf(SEV_WARNING, "OSS sound driver lost interrupt!\n");
			if (nfrags > 2)
				errprintf(SEV_WARNING, "%d fragments passed since last wakeup\n", nfrags);
			ptt_frames -= nfrags;
			while (ptt_frames < info.fragstotal && ptt_frames < 4 && 
			       l1_output_samples(l1time+ptt_frames*inc_fragment, inc_sample, 
						 (short *)(buf.v + info.fragsize * ((curfrag + ptt_frames) % info.fragstotal)), 
						 info.fragsize/2))
				ptt_frames++;
			if (ptt_frames < 0)
				break;
		}
		if (munmap(buf.v, size))
			errstr(SEV_FATAL, "munmap");
		if (close(fd_audio))
			errstr(SEV_FATAL, "close");
	}
}

/* --------------------------------------------------------------------- */

void l1_init(const char *name_audio, const char *name_ptt, const char *name_mixer, 
	     float sndcorr, float tvuseccorr, float cpumhz, int dis_rdtsc, int invptt)
{
	static struct sched_param schp;
	pthread_attr_t thr_attr;
	int fdx, apar;

	if (!name_audio)
		name_audio = "/dev/dsp";
	snd_corr = sndcorr;
	invert_ptt = invptt;
	fdx = probe_oss(name_audio);
	l1_modem_init();
	if (name_ptt) {
		if ((fd_ptt = open(name_ptt, O_RDWR, 0)) < 0)
			errstr(SEV_WARNING, "open");
	} else
		fd_ptt = -1;
	output_ptt(0);
	if (name_mixer) {
		if ((fd_mixer = open(name_mixer, O_RDWR, 0)) < 0)
			errstr(SEV_WARNING, "open");
	} else
		fd_mixer = -1;
	if (fd_mixer >= 0) {
		apar = 0x101 * 20;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_VOLUME, &apar);
		apar = 0x101 * 50;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_TREBLE, &apar);
		apar = 0x101 * 50;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_BASS, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_SYNTH, &apar);
		apar = 0x101 * 100;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_PCM, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_SPEAKER, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_LINE, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_LINE1, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_LINE2, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_LINE3, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_MIC, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_CD, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_IMIX, &apar);
		apar = 0x101 * 0;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_ALTPCM, &apar);
		apar = 0x101 * 50;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_RECLEV, &apar);
		apar = 0x101 * 50;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_IGAIN, &apar);
		apar = 0x101 * 50;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_OGAIN, &apar);

	}
	/*
	 * start the l1 thread
	 */
	memset(&schp, 0, sizeof(schp));
	if (pthread_attr_init(&thr_attr))
		errstr(SEV_FATAL, "pthread_attr_init");
	if (pthread_attr_setschedpolicy(&thr_attr, SCHED_RR))
		errstr(SEV_FATAL, "pthread_attr_setschedpolicy");
	schp.sched_priority = sched_get_priority_min(SCHED_RR)+2;
	if (pthread_attr_setschedparam(&thr_attr, &schp))
		errstr(SEV_FATAL, "pthread_attr_setschedparam");
	if (fdx) {
		if (pthread_create(&thr_l1, &thr_attr, fdx_driver, (char *)name_audio))
			errstr(SEV_FATAL, "pthread_create");
		if (pthread_attr_destroy(&thr_attr))
			errstr(SEV_WARNING, "pthread_attr_destroy");
		return;
	}
	refclock_probe(tvuseccorr, cpumhz, dis_rdtsc);
	if (pthread_create(&thr_l1, &thr_attr, hdx_driver, (char *)name_audio))
		errstr(SEV_FATAL, "pthread_create");	
	if (pthread_attr_destroy(&thr_attr))
		errstr(SEV_WARNING, "pthread_attr_destroy");
}

/* --------------------------------------------------------------------- */

void l1_set_mixer(unsigned int src, int igain, int ogain)
{
	int apar, i;

	if (fd_mixer < 0)
		return;
	if (src >= 0 && src <= 2 && !ioctl(fd_mixer, SOUND_MIXER_READ_RECMASK, &apar)) {
		switch (src) {
		case 0:
			if (apar & (1<<SOUND_MIXER_LINE))
				i = (1<<SOUND_MIXER_LINE);
			else
				i = (1<<SOUND_MIXER_LINE1);
			break;

		case 1:
			i = (1<<SOUND_MIXER_MIC);
			break;

		case 2:
			if (apar & (1<<SOUND_MIXER_CD))
				i = (1<<SOUND_MIXER_CD);
			else
				i = (1<<SOUND_MIXER_LINE2);
			break;
		}
		ioctl(fd_mixer, SOUND_MIXER_WRITE_RECSRC, &i);
	}
	if (igain >= 0 && igain <= 255) {
		i = 100*igain/255;
		i *= 0x101;
		apar = i;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_RECLEV, &apar);
		apar = i;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_IGAIN, &apar);
	}
	if (ogain >= 0 && ogain <= 255) {
		i = 100*ogain/255;
		i *= 0x101;
		apar = i;
		ioctl(fd_mixer, SOUND_MIXER_WRITE_VOLUME, &apar);
	}
}

/* --------------------------------------------------------------------- */

void l1_start_sample(short *data, unsigned int len)
{
	if (!data || !len)
		return;
	samples_ptr = data;
	samples_count = len;
	samples_remain = len;
}

/* --------------------------------------------------------------------- */

int l1_sample_finished(void)
{
	if (samples_remain == 0 && samples_count > 0) {
		samples_count = 0;
		return 1;
	}
	return 0;
}

/* --------------------------------------------------------------------- */

