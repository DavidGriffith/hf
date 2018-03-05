/*****************************************************************************/

/*
 *      audioin.c  --  Linux soundcard DCF77 receiver.
 *
 *      Copyright (C) 1996  Thomas Sailer (sailer@ife.ee.ethz.ch)
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
 */

/*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_AUDIOIO_H
#include <sys/audioio.h>
#endif
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif
#ifdef HAVE_SYS_CONF_H
#ifdef __FreeBSD__
#include <sys/param.h>
#endif
#include <sys/conf.h>
#endif

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/mman.h>
#include <sched.h>
#include <sys/soundcard.h>
#ifdef __linux__
#include <endian.h>
#endif
#ifdef __FreeBSD__ 
#include <sys/endian.h>
#endif
#endif

#include "dcf77.h"

/* --------------------------------------------------------------------- */

#ifdef HAVE_SYS_AUDIOIO_H

void audio_in(const char *name_audio, const struct demodulator *dem)
{
        audio_info_t audioinfo;
        audio_info_t audioinfo2;
	audio_device_t audiodev;
	int fd, i;
	short in[1024];
	
        if (!name_audio)
                name_audio = "/dev/audio";
	if ((fd = open(name_audio, O_RDONLY)) < 0) 
		vlprintf(-1, "Error, cannot open \"%s\"\n", name_audio);
        if (ioctl(fd, AUDIO_GETDEV, &audiodev) == -1) 
		die("ioctl(AUDIO_GETDEV)");
	vlprintf(2, "Audio device: name %s, ver %s, config %s\n",
		 audiodev.name, audiodev.version, audiodev.config);
        AUDIO_INITINFO(&audioinfo);
        audioinfo.record.sample_rate = SAMPLE_RATE;
        audioinfo.record.channels = 1;
        audioinfo.record.precision = 16;
        audioinfo.record.encoding = AUDIO_ENCODING_LINEAR;
#if 0
        audioinfo.record.gain = 0x20;
        audioinfo.record.port = AUDIO_LINE_IN;
        audioinfo.monitor_gain = 0;
#endif
        if (ioctl(fd, AUDIO_SETINFO, &audioinfo) == -1) 
		die("ioctl(AUDIO_SETINFO)");
        if (ioctl(fd, I_FLUSH, FLUSHR) == -1) 
		die("ioctl(AUDIO_SETINFO)");
        if (ioctl(fd, AUDIO_GETINFO, &audioinfo2) == -1)
		die("ioctl(AUDIO_SETINFO)");
	if (dem->init)
		dem->init(audioinfo.record.sample_rate);
	tc_init(audioinfo.record.sample_rate);
	tc_start();
	for (;;) {
		tc_now(0);
		i = read(fd, in, sizeof(in));
		if (i < sizeof(in))
			die("read");
		dem->demodulator(in, sizeof(in)/sizeof(in[0]));
		tc_advance_samples(sizeof(in)/sizeof(in[0]));
	}
	close(fd);
}

#endif

/* --------------------------------------------------------------------- */

#ifdef HAVE_SYS_SOUNDCARD_H

#if __BYTE_ORDER == __BIG_ENDIAN
#define AUDIO_FMT AFMT_S16_BE
#else
#define AUDIO_FMT AFMT_S16_LE
#endif

/* --------------------------------------------------------------------- */

void audio_in(const char *name_audio, const struct demodulator *dem)
{
	int i, apar;
	int fd;
	struct audio_buf_info info;
	struct count_info cinfo;
	unsigned int size;
	short *abuf;
	fd_set mask;
	unsigned int fragptr;
	unsigned int curfrag;
        struct sched_param schp;

        if (!name_audio)
                name_audio = "/dev/dsp";
	/*
	 * set realtime sched
	 */
	memset(&schp, 0, sizeof(schp));
        schp.sched_priority = sched_get_priority_min(SCHED_RR);
        if (sched_setscheduler(0, SCHED_RR, &schp) != 0) 
                perror("sched_setscheduler");
	/*
	 * start receiver
	 */
	if ((fd = open(name_audio, O_RDWR, 0)) < 0)
		die("open");
	/*
	 * configure audio
	 */
	apar = AUDIO_FMT;
	if (ioctl(fd, SNDCTL_DSP_SETFMT, &apar) == -1)
		die("ioctl: SNDCTL_DSP_SETFMT");
	if (apar != AUDIO_FMT)
		vlprintf(-1, "audio driver does not support the S16 format\n");
	apar = 0;
	if (ioctl(fd, SNDCTL_DSP_STEREO, &apar) == -1)
		die("ioctl: SNDCTL_DSP_STEREO");
	if (apar != 0)
		vlprintf(-1, "audio driver does not support mono\n");
	apar = SAMPLE_RATE;
	if (ioctl(fd, SNDCTL_DSP_SPEED, &apar) == -1)
		die("ioctl: SNDCTL_DSP_SPEED");
	if (apar != SAMPLE_RATE) {
		if (abs(apar-SAMPLE_RATE) >= SAMPLE_RATE/100) 
			vlprintf(-1, "audio driver does not support the required "
				 "sampling frequency\n");
		vlprintf(2, "audio driver inexact sampling rate: %d Hz\n", apar);
	}
	if (dem->init)
		dem->init(apar);
	tc_init(apar);
	/*
	 * set fragment sizes
	 */
	apar = 0xffff0007U;
	if (ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &apar) == -1)
		die("ioctl: SNDCTL_DSP_SETFRAGMENT");
	if (ioctl(fd, SNDCTL_DSP_GETISPACE, &info) == -1)
		die("ioctl: SNDCTL_DSP_GETISPACE");
	size = info.fragsize * info.fragstotal;
	/*
	 * mmap buffers
	 */
	if ((abuf = (void *)mmap(NULL, size, PROT_READ, MAP_FILE | MAP_SHARED, 
			 fd, 0)) == (void *)-1)
		die("mmap: PROT_READ");
	vlprintf(1, "OSS: input: #frag: %d  fragsz: %d  bufaddr: %p\n",
		info.fragstotal, info.fragsize, abuf);
	/*
	 * start recording
	 */
	apar = 0;
	if (ioctl(fd, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
		die("ioctl: SNDCTL_DSP_SETTRIGGER");
	apar = PCM_ENABLE_INPUT;
	if (ioctl(fd, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
		die("ioctl: SNDCTL_DSP_SETTRIGGER");
	fragptr = 0;
	/*
	 * first CPU speed estimate
	 */
	if (ioctl(fd, SNDCTL_DSP_GETIPTR, &cinfo) == -1)
		die("ioctl: SNDCTL_DSP_GETIPTR");
	tc_start();
	/*
	 * loop
	 */
	for (;;) {
		/*
		 * process input
		 */
		curfrag = cinfo.ptr / info.fragsize;
		while (fragptr != curfrag) {
			dem->demodulator(abuf + (fragptr * info.fragsize / 2), 
					 info.fragsize / 2);
			fragptr = (fragptr + 1) % info.fragstotal;
			tc_advance_samples(info.fragsize / 2);
		}
		/*
		 * wait for next interrupt (fragment)
		 */
		FD_ZERO(&mask);
		FD_SET(fd, &mask);
		i = select(fd+1, &mask, NULL, NULL, NULL);
		if (i < 0) 
			die("select");
		if (!FD_ISSET(fd, &mask))
			continue;
		if (ioctl(fd, SNDCTL_DSP_GETIPTR, &cinfo) == -1)
			die("ioctl: SNDCTL_DSP_GETIPTR");
		tc_now(((size + cinfo.ptr - fragptr * info.fragsize) % size) / 2);
	}
	if (munmap((caddr_t)abuf, info.fragsize * info.fragstotal))
		die("munmap");
	if (close(fd))
		die("close");
}

#endif
/* --------------------------------------------------------------------- */

void oss_nommap_audio_in(const char *name_audio, const struct demodulator *dem)
{
// experimental!

	int i, apar;
	int fd;
	struct audio_buf_info info;
//	struct count_info cinfo;
	unsigned int size;
//	short *abuf;
//	fd_set mask;
//	unsigned int fragptr;
//	unsigned int curfrag;
        struct sched_param schp;
	short in[1024]; // by günther

        if (!name_audio)
                name_audio = "/dev/dsp";
	/*
	 * set realtime sched
	 */
	memset(&schp, 0, sizeof(schp));
        schp.sched_priority = sched_get_priority_min(SCHED_RR);
        if (sched_setscheduler(0, SCHED_RR, &schp) != 0) 
                perror("sched_setscheduler");
	/*
	 * start receiver
	 */
	if ((fd = open(name_audio, O_RDWR, 0)) < 0)
		die("open");
	/*
	 * configure audio
	 */
	apar = AUDIO_FMT;
	if (ioctl(fd, SNDCTL_DSP_SETFMT, &apar) == -1)
		die("ioctl: SNDCTL_DSP_SETFMT");
	if (apar != AUDIO_FMT)
		vlprintf(-1, "audio driver does not support the S16 format\n");
	apar = 0;
	if (ioctl(fd, SNDCTL_DSP_STEREO, &apar) == -1)
		die("ioctl: SNDCTL_DSP_STEREO");
	if (apar != 0)
		vlprintf(-1, "audio driver does not support mono\n");
	apar = SAMPLE_RATE;
	if (ioctl(fd, SNDCTL_DSP_SPEED, &apar) == -1)
		die("ioctl: SNDCTL_DSP_SPEED");
	if (apar != SAMPLE_RATE) {
		if (abs(apar-SAMPLE_RATE) >= SAMPLE_RATE/100) 
			vlprintf(-1, "audio driver does not support the required "
				 "sampling frequency\n");
		vlprintf(2, "audio driver inexact sampling rate: %d Hz\n", apar);
	}
	if (dem->init)
		dem->init(apar);
	tc_init(apar);
	/*
	 * set fragment sizes
	 */
	apar = 0xffff0007U;
	if (ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &apar) == -1)
		die("ioctl: SNDCTL_DSP_SETFRAGMENT");
	if (ioctl(fd, SNDCTL_DSP_GETISPACE, &info) == -1)
		die("ioctl: SNDCTL_DSP_GETISPACE");
	size = info.fragsize * info.fragstotal;
	tc_start();
	/*
	 * loop
	 */
	 	for (;;) {

		tc_now(0);
		i = read(fd, in, sizeof(in));
		if (i < sizeof(in))
			die("read");
		dem->demodulator(in, sizeof(in)/sizeof(in[0]));
		tc_advance_samples(sizeof(in)/sizeof(in[0]));
	}

	if (close(fd))
		die("close");
}



/* --------------------------------------------------------------------- */

void audio_stdin(const char *name_audio, const struct demodulator *dem)
{
	short in[1024];
	
	if (dem->init)
		dem->init(SAMPLE_RATE);
	tc_init(SAMPLE_RATE);
	tc_start();
	for (;;) {
		tc_now(0);
		if (1 != fread(in, sizeof(in), 1, stdin))
			die("fread");
		dem->demodulator(in, sizeof(in)/sizeof(in[0]));
		tc_advance_samples(sizeof(in)/sizeof(in[0]));
	}
}

/* --------------------------------------------------------------------- */
