#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h> 
#include <stdio.h>

#ifndef HAVE_SYS_SOUNDCARD_H

int main(int argc, char *argv[])
{
	fprintf(stderr, "Platform does not support OSS\n");
	exit(1);
}

#else

#include <sys/time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define AUDIOPATH "/dev/dsp"

#define NTESTS 16
#define SAMPLE_RATE 8000

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)(-1))
#endif

#define FULL_MMAP

static char *testit(void)
{
        size_t pagesize = getpagesize();
	struct sched_param schp;
	unsigned int times[NTESTS];
	int i, fd, apar;
        struct audio_buf_info iinfo;
	caddr_t ibuf, ibufe;
        unsigned int isize;
	struct timeval tv;
	unsigned int avg, tm;

	if ((schp.sched_priority = sched_get_priority_min(SCHED_RR)) == -1)
		return "sched_get_priority_min";
	if (sched_setscheduler(0, SCHED_RR, &schp))
		return "sched_setscheduler";
	
	for (i = 0; i < NTESTS; i++) {
		if ((fd = open(AUDIOPATH, O_RDWR, 0)) < 0)
			return "open";
		apar = AFMT_S16_LE;
		if (ioctl(fd, SNDCTL_DSP_SETFMT, &apar) == -1)
			return "ioctl: SNDCTL_DSP_SETFMT";
		if (apar != AFMT_S16_LE) 
			return "audio driver does not support AFMT_S16_LE";
		apar = 0;
		if (ioctl(fd, SNDCTL_DSP_STEREO, &apar) == -1)
			return "ioctl: SNDCTL_DSP_STEREO";
		if (apar != 0)
			return "audio driver does not support mono";
		apar = SAMPLE_RATE;
		if (ioctl(fd, SNDCTL_DSP_SPEED, &apar) == -1)
			return "ioctl: SNDCTL_DSP_SPEED";
		if (apar != SAMPLE_RATE) 
			if (abs(apar-SAMPLE_RATE) >= SAMPLE_RATE/100)
				return "audio driver does not support 8kHz sampling frequency";
		apar = 0xffff0007U;
		if (ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &apar) == -1)
			return "ioctl: SNDCTL_DSP_SETFRAGMENT";
		if (ioctl(fd, SNDCTL_DSP_GETISPACE, &iinfo) == -1)
			return "ioctl: SNDCTL_DSP_GETISPACE";
		isize = iinfo.fragstotal * iinfo.fragsize;
		if (ioctl(fd, SNDCTL_DSP_GETCAPS, &apar) == -1)
			return "ioctl: SNDCTL_DSP_GETCAPS";
		if (!(apar & DSP_CAP_TRIGGER) || !(apar & DSP_CAP_MMAP))
			return "Sound driver does not support mmap and/or trigger";
#ifdef FULL_MMAP
		if ((ibuf = mmap(NULL, pagesize+isize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return "mmap: MAP_ANONYMOUS";
		ibufe = ibuf + isize;
		if (munmap(ibuf, isize))
			return "munmap: MAP_ANONYMOUS";
		if ((ibuf = mmap(ibuf, isize, PROT_READ, MAP_FILE | MAP_SHARED | MAP_FIXED,  fd, 0)) == MAP_FAILED)
			return "mmap: PROT_READ";
#else /* FULL_MMAP */
		if ((ibuf = mmap(NULL, isize, PROT_READ, MAP_FILE | MAP_SHARED | MAP_FIXED,  fd, 0)) == MAP_FAILED)
			return "mmap: PROT_READ";
#endif /* FULL_MMAP */
		apar = 0;
                if (ioctl(fd, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
                        return "ioctl: SNDCTL_DSP_SETTRIGGER";
                apar = PCM_ENABLE_INPUT;
                if (ioctl(fd, SNDCTL_DSP_SETTRIGGER, &apar) == -1)
                        return "ioctl: SNDCTL_DSP_SETTRIGGER";

		if (gettimeofday(&tv, NULL))
			return "gettimeofday";
		times[i] = tv.tv_usec;

		if (munmap(ibuf, isize))
			return "munmap";
#ifdef FULL_MMAP
		if (munmap(ibufe, pagesize))
			return "munmap";
#endif /* FULL_MMAP */
		if (close(fd))
			return "close";
	}
	printf("Results (us):");
	for (avg = 0, i = 1; i < NTESTS; i++) {
		printf(" %d", (tm = (1000000+times[i]-times[i-1]) % 1000000));
		avg += tm;
	}
	printf("\nAverage (us): %d\n", (avg + NTESTS/2) / NTESTS);
	return NULL;
}

int main(int argc, char *argv[])
{
	char *c;

	if ((c = testit()))
		perror(c);
	exit(0);
}

#endif
