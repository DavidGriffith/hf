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

//#include <asm/byteorder.h>
#include <sys/soundcard.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>
//#include <endian.h>
#include <pthread.h>
#include <syslog.h>

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"

/* --------------------------------------------------------------------- */
/*
 * Sample output
 */
 
//static pthread_t thr_l1;

/* --------------------------------------------------------------------- */

// for exiting when endless loop at xrun
// brake = xruncounter, exits after brakepoint counts.
// It is counted in this file, but is 
// defined in util.h to prevent compiler error 
// because of complicated include with hfkernel_link....
// int brake = 0, 
extern int brake;
int brakepoint = 70;
// flag: fdx possible, hdx chosen by option
int fdx_possible_hdx_chosen = 0;



union {
	caddr_t v;
	short *s;
} ibuf;
union {
	caddr_t v;
	short *s;
} obuf;
union {
	caddr_t v;
	short *s;
} buf;
unsigned int size, isize, osize;

/* --------------------------------------------------------------------- */

#if __BYTE_ORDER == __BIG_ENDIAN
#define AUDIO_FMT AFMT_S16_BE
#else
#define AUDIO_FMT AFMT_S16_LE
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif
#define EXCESS_FRAGS 3
#define NTESTS 4
#define FULL_MMAP

int probe_oss(const char *name_audio)
{
        size_t pagesize = getpagesize();
	int apar, i;
	struct audio_buf_info iinfo, oinfo;
	struct timeval t1, t2;
        unsigned int times[NTESTS];
        unsigned int isize;
        caddr_t ibuf, ibufe;
        union {
	    caddr_t v;
	    short *s;
	} testbuf;
	unsigned int avg;
	struct timespec my_time_spec_nano;

	my_time_spec_nano.tv_sec = 0;
	my_time_spec_nano.tv_nsec = 100;  // 100 nanoseconds	
	
        if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
                errstr(SEV_FATAL, "probe_oss: open");
	//printf("probe_oss: fd_audio is %d\n", fd_audio);
        /*
         * configure audio
         */
        apar = AUDIO_FMT;
        if (ioctl(fd_audio, SNDCTL_DSP_SETFMT, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_SETFMT");
        if (apar != AUDIO_FMT) 
		errprintf(SEV_FATAL, "audio driver does not support the S16 format\n");
        apar = 0;
        if (ioctl( fd_audio, SNDCTL_DSP_STEREO, &apar) == -1)
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
	osize = oinfo.fragstotal * oinfo.fragsize;
	if (ioctl(fd_audio, SNDCTL_DSP_GETISPACE, &iinfo) == -1)
		errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETISPACE");
	isize = iinfo.fragstotal * iinfo.fragsize;
	if (EXCESS_FRAGS * iinfo.fragsize > pagesize)
		errprintf(SEV_FATAL, "OSS: input: fragment size %d times excess frags > page size %d\n",
			  iinfo.fragsize, pagesize);
	
        /*
         * determine if the driver is capable to support our access method
         */
        if (ioctl(fd_audio, SNDCTL_DSP_GETCAPS, &apar) == -1)
                errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETCAPS");
        if (!(apar & DSP_CAP_TRIGGER) || !(apar & DSP_CAP_MMAP)) {
    	    errprintf(SEV_WARNING, "Sound driver does not support mmap and/or trigger\n");
	    nommap = 1;	
	}

	/* a second mmap test,  as sometimes the first is false-positive */
	
	if ((testbuf.v = mmap(NULL, isize, PROT_READ, 
			   MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
  		    /* sometimes mmap fails although the above test is o.k. */
            errprintf(SEV_WARNING, "probe_oss: testbuf.v: mmap: MAP_ANONYMOUS failed.\n");
	    nommap = 1;
	    if (close(fd_audio))
                errstr(SEV_FATAL, "close");
	    }
	if (!nommap)
	    if (munmap(testbuf.v, isize))
		errstr(SEV_FATAL, "fdx: munmap: testbuf.v: MAP_ANONYMOUS");
	close(fd_audio);
	
#ifndef OSS_DISABLE_FDX
    if (apar & DSP_CAP_DUPLEX) {
	errprintf(SEV_INFO, "Your soundcard is full duplex capable.\n");
	if (force_halfduplex == 1) {
	errprintf(SEV_INFO, "But you have selected the option -h, \n"
	    "so I will run in half duplex mode.\n");
	     fdx_possible_hdx_chosen = 1;
	}
	else {
	errprintf(SEV_INFO, 
	    "Good; the reference time will be derived from the soundcard sample clock,\n"
	    "which should be accurate within +-20ppm.\n"
	    "So I hope your soundcard uses 'real' crystals instead of cheap ceramic\n"
	    "resonators, and you adjusted the clock correctly with\n" 
	    "your soundcard correction factor %10.8f.\n", snd_corr);
	return 1;
	}
    }
#endif /* OSS_DISABLE_FDX */

	for (i = 0; i < 10; i++) {
		if (gettimeofday(&t1, NULL))
			errstr(SEV_FATAL, "gettimeofday");
//i inserted following line because of error on faaaast machines			
//simple version:	nanosleep(100, NULL);	
// better, tnx Remi!
    		nanosleep(&my_time_spec_nano, NULL);	
		if (gettimeofday(&t2, NULL))
			errstr(SEV_FATAL, "gettimeofday");
		if (((1000000+t2.tv_usec-t1.tv_usec) % 1000000) > 1)
			break;
	}
	if (i >= 10)
		errprintf(SEV_FATAL, 
		    "Your gettimeofday resolution seems to be inappropriate\n");
        for (i = 0; i < NTESTS; i++) {
                if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
                        errstr(SEV_FATAL, "probe_oss: open");
		//printf("probe_oss: fd_audio is %d\n", fd_audio);
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
			
	        if (!(apar & DSP_CAP_TRIGGER) || !(apar & DSP_CAP_MMAP)) {
        	    errprintf(SEV_WARNING, "Sound driver does not support mmap and/or trigger\n");
		    nommap = 1;
		    errprintf(SEV_WARNING, "So I can not calculate soundcard switch times for now.\n");

	            if (close(fd_audio))
                        errstr(SEV_FATAL, "close");
	
		    return 0;
		}
/*
		original
                if (!(apar & DSP_CAP_TRIGGER) || !(apar & DSP_CAP_MMAP))
                        errprintf(SEV_FATAL, "Sound driver does not support mmap and/or trigger");
*/

#ifdef FULL_MMAP
                if ((ibuf = mmap(NULL, pagesize+isize, PROT_READ | PROT_WRITE, 
				 MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED){
		    /* sometimes mmap fails although the above test is o.k. */
		    
           	    errprintf(SEV_WARNING, "probe_oss: ibuf: mmap: MAP_ANONYMOUS failed.\n");
		    nommap = 1;
		    errprintf(SEV_WARNING, "So I can not calculate soundcard switch times for now.\n");
	            if (close(fd_audio))
                        errstr(SEV_FATAL, "close");
		    return 0;
		}

//			errstr(SEV_FATAL, "probe: ibuf: mmap: MAP_ANONYMOUS");
			
                ibufe = ibuf + isize + pagesize;
                if (munmap(ibuf, isize))
                        errstr(SEV_FATAL, "probe: ibuf: munmap: MAP_ANONYMOUS");
                if ((ibuf = mmap(ibuf, isize, PROT_READ, MAP_FILE | MAP_SHARED | MAP_FIXED,  
				 fd_audio, 0)) == MAP_FAILED)
                        errstr(SEV_FATAL, "probe: ibuf: mmap: PROT_READ");
#else /* FULL_MMAP */
                if ((ibuf = mmap(NULL, isize, PROT_READ, MAP_FILE | MAP_SHARED | MAP_FIXED,  
				 fd_audio, 0)) == MAP_FAILED)
                        errstr(SEV_FATAL, "probe: ibuf: mmap: PROT_READ");
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
/*		errprintf(SEV_INFO, 
	          "\ntesting soundcard tx delay by gettimeofday: "
		  "* * *  %d microseconds * * * \n", times[i]); 
*/
                if (munmap(ibuf, isize))
                        errstr(SEV_FATAL, "munmap ibuf");

/*#ifdef FULL_MMAP
//makes errorbreak so outcommented by dl4mge
                if (munmap(ibufe, pagesize))
                        errstr(SEV_FATAL, "munmap ibufe");
#endif  FULL_MMAP 
*/

                if (close(fd_audio))
                        errstr(SEV_FATAL, "close");
	}
        for (avg = 0, i = 1; i < NTESTS; i++)
/*
these i do not understand, also they give postal code on my machine
so i changed it 
Guenther
	avg += (tm = (1000000+times[i]-times[i-1]) % 1000000);
	avg = (avg + NTESTS/2) / NTESTS;
*/
	avg += ((times[i] - times[i-1]) / 1000);
	avg = avg  / (NTESTS -1);
	
	if (fdx_possible_hdx_chosen == 0) {
	    errprintf(SEV_INFO, 
	      "Your soundcard is not full duplex capable. \n"
	      "This is bad; but we will try half duplex mode. \n");
	}
	errprintf(SEV_INFO, 
	  "Now, the soundcard is switched between RX and TX.\n"
	  "This lasts * * *  %d milliseconds * * * on average.\n"
	  "I will try to hide this latency within the txdelay, \n"
	  "so set it to some value larger than this. And let's hope \n"
	  "propagation and txdelay of your peers also lasts longer...\n", avg);
	return 0;
}

/* --------------------------------------------------------------------- */

void l1_oss_fdx_mmap_cleanup(void *dummy) 
{
	//int res = 0;
	if (munmap(ibuf.v, isize))
		errstr(SEV_FATAL, "fdx: munmap: ibuf: MAP_ANONYMOUS");
	if (munmap(obuf.v, osize))
		errstr(SEV_FATAL, "fdx: munmap: obuf: MAP_ANONYMOUS");
	if (close(fd_audio))
		errstr(SEV_FATAL, "close");
	fd_audio = -1;

}

/* --------------------------------------------------------------------- */


void *oss_fdx_driver(void *name_audio)
{
        size_t pagesize = getpagesize();
        int apar, i;
	caddr_t ibuf_ext;
	struct audio_buf_info iinfo, oinfo;
	unsigned int ifragptr, ofragptr;
	fd_set rmask, wmask;
	struct count_info cinfo;
	unsigned int curfrag, lastfrag, nfrags;
	unsigned long long itime, otime, inc_fragment;
	l1_time_t inc_sample;
	int ptt_frames;
	short *s;
	void *m = NULL;
	
	pthread_cleanup_push(l1_oss_fdx_mmap_cleanup, NULL);

        if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
                errstr(SEV_FATAL, "oss_fdx_driver: open");
	//printf("oss_fdx_driver: fd_audio is %d\n", fd_audio);
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
	if ((ibuf.v = mmap(NULL, isize, PROT_READ, 
			   MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		errstr(SEV_FATAL, "fdx: mmap: ibuf: MAP_ANONYMOUS");
	ibuf_ext = ibuf.v + isize;
	if (munmap(ibuf.v, isize))
		errstr(SEV_FATAL, "fdx: munmap: ibuf: MAP_ANONYMOUS");
	if ((ibuf.v = mmap(ibuf.v, isize , PROT_READ , 
			    MAP_FILE  | MAP_SHARED | MAP_FIXED, 
			  fd_audio, 0)) == MAP_FAILED)
		errstr(SEV_FATAL, "fdx: mmap: ibuf: PROT_READ");

	if ((obuf.v = mmap(NULL, osize, PROT_WRITE /* | PROT_READ */, 
			   MAP_FILE | MAP_SHARED, fd_audio, 0)) == MAP_FAILED)
		errstr(SEV_FATAL, "fdx: mmap: obuf: PROT_WRITE");

	errprintf(SEV_INFO, 
		    "soundcard clock correction: %f\n"
		    "OSS:  input: #frag: %d  fragsz: %d  totbuf: %d\n"
		    "bufaddr: %p  mempage: %p\n"
		    "OSS: output: #frag: %d  fragsz: %d  totbuf: %d  "
		    "\nbufaddr: %p\n"
		    "OSS: sample time increment: %u  fragment time increment: %u\n",
		    snd_corr,
		    iinfo.fragstotal, iinfo.fragsize, isize, ibuf.s, ibuf_ext, 
		    oinfo.fragstotal, oinfo.fragsize, osize, obuf.s,
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

		if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL))
		    errstr(SEV_FATAL, "pthread_setcancelstate");

		FD_ZERO(&rmask);
		FD_ZERO(&wmask);
		FD_SET(fd_audio, &rmask);
		FD_SET(fd_audio, &wmask);
		i = select(fd_audio+1, &rmask, &wmask, NULL, NULL);
		if (i < 0) 
			errstr(SEV_FATAL, "select");
		
		if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL))
		    errstr(SEV_FATAL, "pthread_setcancelstate");
		
		if (!FD_ISSET(fd_audio, &rmask) && !FD_ISSET(fd_audio, &wmask))
			continue;
		
		/*
		 * process input
		 */
		if (ioctl(fd_audio, SNDCTL_DSP_GETIPTR, &cinfo) == -1)
			errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETIPTR");
		curfrag = cinfo.ptr / iinfo.fragsize;
//		printf( "fdx in:  count info: bytes = %d, blocks = %d, ptr = %d\n", cinfo.bytes, cinfo.blocks, cinfo.ptr );
		if (cinfo.blocks > 2) {
			errprintf(SEV_WARNING, "in:  %d fragments passed since last wakeup\n", cinfo.blocks);
			brake ++;
			}
			
		if (brake >= brakepoint) {
			errprintf(SEV_FATAL, "l1/user/oss.c: fdx_driver: %d xruns. Exiting.\n", brake);
			exit (1);
			}
		
		while (ifragptr != curfrag) {

			if (!ifragptr) 
			
			{
				m = malloc(pagesize);
				if (m == NULL)
	   			    errstr(SEV_FATAL, "fdx_driver: malloc failed");
				//	s = (short *)(ibuf.v + isize );
				s = (short *) m;
		//		printf("\nifragptr: %d, curfrag %d, ibuf.v: %p, s: %p \n",
		//		ifragptr, curfrag, ibuf.v, s);  			 
		//		printf ("shift mem malloced in %p\n", m);
				memcpy(s, ibuf.v, iinfo.fragsize);
			} 
			
			else
				s = (short *)(ibuf.v + iinfo.fragsize * ifragptr);
		//	printf("=\nifragptr: %d, curfrag %d, ibuf.v: %p, s: %p \n", 
		//	        ifragptr, curfrag, ibuf.v, s);  			
			
			if (samples_remain > 0) {
				i = 2*samples_remain;
				if (i > iinfo.fragsize)
					i = iinfo.fragsize;
				memcpy(samples_ptr, s, i);
		
		//		printf("\nsamples_remain: %d, samples_ptr: %p, i: %d", samples_remain, samples_ptr, i);  			
								
				samples_ptr += i/2;
				samples_remain -= i/2;
			}
			
			l1_input_samples(itime >> 24, inc_sample, s, iinfo.fragsize/2);
			itime += inc_fragment;			
			if ((!ifragptr) && (m!= NULL)) free(m);
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
//		printf( "fdx out: count info: bytes = %d, blocks = %d, ptr = %d\n", cinfo.bytes, cinfo.blocks, cinfo.ptr );
		nfrags = oinfo.fragstotal + curfrag - lastfrag;
		lastfrag = curfrag;
		if (nfrags >= oinfo.fragstotal)
			nfrags -= oinfo.fragstotal;
		if (nfrags != cinfo.blocks)
			errprintf(SEV_WARNING, "OSS sound driver lost interrupt!\n");
		if (nfrags > 2) {
			errprintf(SEV_WARNING, "out: %d fragments passed since last wakeup\n", nfrags);
			brake ++;
			}
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
	pthread_cleanup_pop(0);	
}

/* --------------------------------------------------------------------- */

void l1_oss_hdx_mmap_cleanup(void *dummy) 
{
	if (munmap(buf.v, size))
		errstr(SEV_FATAL, "munmap");
	if (close(fd_audio))
		errstr(SEV_FATAL, "close");
	fd_audio = -1;

}


/* --------------------------------------------------------------------- */

void *oss_hdx_driver(void *name_audio)
{
        size_t pagesize = getpagesize();
        int apar, i;
	caddr_t buf_ext;
	struct audio_buf_info info;
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
	void *m = NULL;

	pthread_cleanup_push(l1_oss_hdx_mmap_cleanup, NULL);

	refclock_init();


	for (;;) {
		/*
		 * start receiver
		 */
		output_ptt(0);
		if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
			errstr(SEV_FATAL, "oss_hdx_driver: open");
		//printf("oss_hdx: fd_audio is %d\n", fd_audio);
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
		errprintf (SEV_INFO, 
			"\nOSS: input: #frag: %d  fragsz: %d  totbuf: %d "
			"\nbufaddr: %p  mempage: %p\n"
			"OSS: sample time increment: %u  fragment time increment: %u "
			"\ntime: %8ld\n",
			  info.fragstotal, info.fragsize, size, buf.s, buf_ext, 
			  (unsigned int)inc_sample, (unsigned int)inc_fragment, l1time * 1000000);
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
	

			if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL))
			    errstr(SEV_FATAL, "hdx: pthread_setcancelstate");

	
			FD_ZERO(&mask);
			FD_SET(fd_audio, &mask);
			i = select(fd_audio+1, &mask, NULL, NULL, NULL);
			if (i < 0) 
				errstr(SEV_FATAL, "select");

			if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL))
			    errstr(SEV_FATAL, "hdx: pthread_setcancelstate");

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
/*					s = (short *)(buf.v + size);
*/
					m = malloc(pagesize);
					s = (short *)m;
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
				if ((!fragptr) && (m!=NULL)) free(m);
				fragptr++;
				if (fragptr >= info.fragstotal)
					fragptr = 0;
			}
			/*
			 * check for fsk output
			 */
			if (l1_fsk_next_tx_event(l1time) <= (long)inc_fragment/2)
				break;
			/*
			 * check for mt63 output
			 */
			if (mt63_data_waiting)
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
			errstr(SEV_FATAL, "oss_hdx_driver: open");
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
		l1_fsk_finish_pending_rx_requests();
		/*
		 * loop
		 */
		for (;;) {
		
			if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL))
			    errstr(SEV_FATAL, "hdx: pthread_setcancelstate");

			FD_ZERO(&mask);
			FD_SET(fd_audio, &mask);
			i = select(fd_audio+1, NULL, &mask, NULL, NULL);
			if (i < 0) 
				errstr(SEV_FATAL, "select");
			
			if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL))
			    errstr(SEV_FATAL, "hdx: pthread_setcancelstate");

			
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
			if (nfrags > 2) {
				errprintf(SEV_WARNING, "%d fragments passed since last wakeup\n", nfrags);
				brake++;
			}
				
			if (brake >= brakepoint) {
				errprintf(SEV_FATAL, "l1/user/oss.c: hdx_driver: %d xruns. Exiting.\n", brake);
			exit (1);
			}
		
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
	pthread_cleanup_pop(0);	
}

/* --------------------------------------------------------------------- */

void l1_oss_fdx_nommap_cleanup(void *dummy) 
{
	if ( close (fd_audio))
	    errstr(SEV_FATAL, 
	    "l1_oss_fdx_nommap_cleanup: error in closing fd_audio: ");
        fd_audio = -1;
}

/* --------------------------------------------------------------------- */

void *oss_nommap_fdx_driver(void *name_audio)
{
        size_t pagesize = getpagesize();
        int apar, i, j;
	union {
		caddr_t v;
		short *s;
	} ibuf;
	caddr_t ibuf_ext = NULL;
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
	int ptt_frames, silent_frames, frames, block, loop;
	short in[128];
	short out[128];

        pthread_cleanup_push(l1_oss_fdx_nommap_cleanup, NULL);
	
        if ((fd_audio = open(name_audio, O_RDWR, 0)) < 0)
                errstr(SEV_FATAL, "oss_nommap_fdx: open");
	//printf("oss nommap fdx : fd_audio is %d\n", fd_audio);
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
	silent_frames = 0;
        frames = 0;
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

	errprintf(SEV_INFO, 
	    "\nOSS:  input: #frag: %d  fragsz: %d  totbuf: %d\n"
	    "bufaddr: %p  mempage: %p\n"
	    "OSS: output: #frag: %d  fragsz: %d  totbuf: %d  "
	    "\nbufaddr: %p\n"
	    "OSS: sample time increment: %u  fragment time increment: %lu\n",
	    iinfo.fragstotal, iinfo.fragsize, isize, ibuf.s, ibuf_ext, 
	    oinfo.fragstotal, oinfo.fragsize, osize, obuf.s,
	    (unsigned int)inc_sample, (unsigned long)(inc_fragment >> 24));
	/*
	 * loop
	 */
        for(;;) {
	
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
            block = 1;

        /*
         * pre-fill input
         */

            if (!ifragptr) {
                j = read(fd_audio, in, iinfo.fragsize * block);
	        if (j < iinfo.fragsize * block) errstr(SEV_FATAL, "initial read");
                memset (in, 0, sizeof(in));
            }

	    if (ioctl(fd_audio, SNDCTL_DSP_GETIPTR, &cinfo) == -1)
		errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETIPTR");
	    curfrag = cinfo.ptr / ((iinfo.fragsize)*block);
//	    printf( "fdx in:  count info: bytes = %d, blocks = %d, ptr = %d\n", 
//		cinfo.bytes, cinfo.blocks, cinfo.ptr );
//	    printf("ifragptr: %d\n", ifragptr);
//	    printf("curfrag: %d\n", curfrag);

/* 
	    if (cinfo.blocks > 2 * block ) { // was 2
		errprintf(SEV_WARNING, 
		    "in:  %d fragments passed since last wakeup\n", 
                    cinfo.blocks);
		brake ++;
    
                for (i = cinfo.blocks; i < 2 ; i--) {
                    j = read(fd_audio, in, iinfo.fragsize * block);
		    if (j < iinfo.fragsize * block) errstr(SEV_FATAL, "read");
                } 
                printf ("xrun reset.\n");
	    }
*/
//	    if (brake >= brakepoint) {
//		errprintf(SEV_FATAL, 
//                "l1/user/oss.c: fdx_driver: %d xruns. Exiting.\n", brake);
//		exit (1);
//	    }
            loop = 0;
            if ( (ifragptr != curfrag && loop < 2)  ) {  
//	        printf("ifragptr: %d", ifragptr);
//	        printf(" != curfrag: %d\n", curfrag);
		j = read(fd_audio, in, iinfo.fragsize * block);
		if (j < iinfo.fragsize * block) errstr(SEV_FATAL, "read");
//		printf("simple fdx driver: read %d bytes\n", j);
//                printf("r");
		l1_input_samples
	    	    (itime >> 24, inc_sample, in, (iinfo.fragsize * block)/2);
		itime += (inc_fragment * block);			
		ifragptr++; 
                loop ++;
		while  (ifragptr > (iinfo.fragstotal / block)) // was while  >= 
			ifragptr -= (iinfo.fragstotal / block);

                if (samples_remain > 0) {
		    i = 2*samples_remain;
		    if (i > iinfo.fragsize)	i = iinfo.fragsize;
		    memcpy(samples_ptr, in, iinfo.fragsize);
//		    printf("\nsamples_remain: %d, samples_ptr: %p, bytes: %d", 
//	    		samples_remain, samples_ptr, j);  			
		    samples_ptr += i/2;
		    samples_remain -= i/2;
                }
            }
            memset (in, 0, sizeof(in));
                        
	    /*
	     * process output
	     */
	    if (ioctl(fd_audio, SNDCTL_DSP_GETOPTR, &cinfo) == -1)
		errstr(SEV_FATAL, "ioctl: SNDCTL_DSP_GETOPTR");
	    curfrag = cinfo.ptr / oinfo.fragsize;
	    nfrags = oinfo.fragstotal + curfrag - lastfrag;
	    if (nfrags >= oinfo.fragstotal)
	    	nfrags -= oinfo.fragstotal;
	    lastfrag = curfrag;            

//	    printf( "simple fdx out: count info: bytes = %d, blocks = %d, ptr = %d\n", 
//		cinfo.bytes, cinfo.blocks, cinfo.ptr );
//	    printf( "fdx out: curfrag: %u\n", curfrag);
//	    printf( "fdx out: nfrags: %d\n", nfrags);
//	    printf( "fdx out: waiting data fragments: %d\n", ptt_frames);
//	    printf( "fdx out: waiting silence fragments: %d\n", frames - ptt_frames);
//	    printf("fdx out:  otime: %lu\n", (unsigned long)(otime >> 24));

//	    if (nfrags != cinfo.blocks)
//			errprintf(SEV_WARNING, "simple fdx out: OSS sound driver lost interrupt!\n");
	    if (nfrags > 2) {
		errprintf(SEV_WARNING, 
                    "simple fdx out: %d fragments passed since last wakeup\n", 
                    nfrags);
		brake ++;
	    }
	    frames -= nfrags;
            ptt_frames -= nfrags;
	    if (frames < 0) {
		otime += (-frames) * inc_fragment;
		frames = 0;
		memset(out, 0, sizeof(out));
                errprintf(SEV_WARNING,
			  "simple fdx out: written more than there - count error?\n");
	    }
	    if (ptt_frames < 0) {
		ptt_frames = 0;
	    }
            while   (ptt_frames < oinfo.fragstotal && 
                    ptt_frames <= 4 &&
                    frames <= 4 &&
		    l1_output_samples(itime >> 24, inc_sample, 
		        out, oinfo.fragsize/2)) {
                j = write(fd_audio, out, oinfo.fragsize);
		if (j == -1) 
		    errstr(SEV_FATAL, "simple fdx out: write");
//		printf("simple fdx out: written %d data bytes\n", j);
//              printf("t");
		if (j < oinfo.fragsize) 
		    errstr(SEV_INFO, 
                    "simple fdx out: not written all data.\n");
		memset(out, 0, sizeof(out));
		ptt_frames++;
                frames++;
		otime += (inc_fragment);
		
	    }
            while (frames <= 4) {
                j = write(fd_audio, out, oinfo.fragsize);
		    if (j == -1) 
		        errstr(SEV_FATAL, "simple fdx out: write silence");
//		printf("simple fdx out: written %d bytes of silence\n", j);
//              printf("s");
                if (j < oinfo.fragsize) 
		    errstr(SEV_INFO, 
                    "simple fdx out: not written all silence.\n");
                frames++;
                otime +=  inc_fragment;
            	
            }
	    if (frames != 5) {
                errprintf(SEV_WARNING,
			  "simple fdx out: stack of tx frames %d != 5\n", frames);
	    }
            output_ptt(ptt_frames > 0);
        }
	pthread_cleanup_pop(0);	
}

/* --------------------------------------------------------------------- */

void *oss_nommap_hdx_driver(void *name_audio)
{
// dummy
	errstr(SEV_FATAL, 
	    "oss_nommap_hdx_driver: this function is being developed, "
	    "but not yet implemented. "
	    "Please look at the hftermhackers mailinglist!");
	return(0);
}

/* --------------------------------------------------------------------- */

void oss_prepare_mixer(const char *name_mixer)
{
	int apar = 0;	

	if (name_mixer) {
	    if ((fd_mixer = open(name_mixer, O_RDWR, 0)) < 0)
		errstr(SEV_WARNING, "oss_prepare_mixer: open");
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

