/*****************************************************************************/

/*
 *      audioout.c  --  Linux soundcard DCF77 signal generator, audio output.
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
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

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

void audio_out(const char *name_audio, const struct modulator *mod)
{
        audio_info_t audioinfo;
        audio_info_t audioinfo2;
	audio_device_t audiodev;
	int fd, i;
	short out[1024];
	
	if (!name_audio)
		name_audio = "/dev/audio";
        if ((fd = open(name_audio, O_WRONLY)) < 0)
                vlprintf(-1, "Error, cannot open \"%s\"\n", name_audio);
        if (ioctl(fd, AUDIO_GETDEV, &audiodev) == -1) 
		die("ioctl(AUDIO_GETDEV)");
        vlprintf(1, "Audio device: name %s, ver %s, config %s\n",
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
	if (mod->init)
		mod->init(audioinfo.record.sample_rate);
	for (;;) {
		mod->modulator(out, sizeof(out)/sizeof(out[0]));
		i = write(fd, out, sizeof(out));
		if (i < sizeof(out))
			die("write");
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

void audio_out(const char *name_audio, const struct modulator *mod)
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
		vlprintf(1, "sched_setscheduler: %s", strerror(errno));
        /*
         * start transmitter
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
                vlprintf(1, "audio driver inexact sampling rate: %d Hz\n", apar);
        }
	if (mod->init)
		mod->init(apar);
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
        if ((abuf = (void *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, 
                         fd, 0)) == (void *)-1)
                die("mmap: PROT_READ");
        vlprintf(1, "OSS: output: #frag: %d  fragsz: %d  bufaddr: %p\n",
		 info.fragstotal, info.fragsize, abuf);
        /*
	 * prefill audio buffer
	 */
	for (fragptr = 0; fragptr < info.fragstotal-1; fragptr++)
		mod->modulator(abuf + (fragptr * info.fragsize / 2), info.fragsize / 2);
	/*
         * start recording
         */
        apar = 0;
        if (ioctl(fd, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
                die("ioctl: SNDCTL_DSP_SETTRIGGER");
        apar = PCM_ENABLE_OUTPUT;
        if (ioctl(fd, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
                die("ioctl: SNDCTL_DSP_SETTRIGGER");
	/*
         * loop
         */
        for (;;) {
                /*
                 * wait for next interrupt (fragment)
                 */
                FD_ZERO(&mask);
                FD_SET(fd, &mask);
                i = select(fd+1, NULL, &mask, NULL, NULL);
                if (i < 0) 
                        die("select");
                if (!FD_ISSET(fd, &mask))
                        continue;
                if (ioctl(fd, SNDCTL_DSP_GETOPTR, &cinfo) == -1)
                        die("ioctl: SNDCTL_DSP_GETOPTR");
		curfrag = cinfo.ptr / info.fragsize;
		/*
                 * generate output
                 */
                while (fragptr != curfrag) {
			mod->modulator(abuf + (fragptr * info.fragsize / 2), info.fragsize / 2);
			fragptr = (fragptr + 1) % info.fragstotal;
		}
	}
        if (munmap((caddr_t)abuf, info.fragsize * info.fragstotal))
                die("munmap");
        if (close(fd))
                die("close");
}
#endif

/* --------------------------------------------------------------------- */

void oss_nommap_audio_out(const char *name_audio, const struct modulator *mod)
{
// experimental !

        int i, apar;
        int fd;
        struct audio_buf_info info;
//        struct count_info cinfo;
        unsigned int size;
//        short *abuf;
//	fd_set mask;
//        unsigned int fragptr;
//        unsigned int curfrag;
        struct sched_param schp;
	short out[1024]; // new by günther


	if (!name_audio)
		name_audio = "/dev/dsp";
        /*
         * set realtime sched
         */
        memset(&schp, 0, sizeof(schp));
        schp.sched_priority = sched_get_priority_min(SCHED_RR);
        if (sched_setscheduler(0, SCHED_RR, &schp) != 0) 
		vlprintf(2, "sched_setscheduler: %s", strerror(errno));
        /*
         * start transmitter
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
	if (mod->init)
		mod->init(apar);
	/*
         * set fragment sizes
         */
        apar = 0xffff0007U;
        if (ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &apar) == -1)
                die("ioctl: SNDCTL_DSP_SETFRAGMENT");
        if (ioctl(fd, SNDCTL_DSP_GETISPACE, &info) == -1)
                die("ioctl: SNDCTL_DSP_GETISPACE");
        size = info.fragsize * info.fragstotal;

	for (;;) {
		mod->modulator(out, sizeof(out)/sizeof(out[0]));
		i = write(fd, out, sizeof(out));
		if (i < sizeof(out))
			die("write");
	}
        if (close(fd))
                die("close");
}


/* --------------------------------------------------------------------- */

void audio_stdout(const char *name_audio, const struct modulator *mod)
{
	short out[1024];
	
	if (mod->init)
		mod->init(SAMPLE_RATE);
	for (;;) {
		mod->modulator(out, sizeof(out)/sizeof(out[0]));
		if (1 != fwrite(out, sizeof(out), 1, stdout))
			die("fwrite");
	}
}

/* --------------------------------------------------------------------- */
