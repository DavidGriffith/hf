/*****************************************************************************/

/*
 *      reffreq.c  --  Linux calibration using reference frequency.
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
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>
#include <sched.h>
#include <sys/ipc.h>
#include <math.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/soundcard.h>
#include <asm/page.h>
#include <endian.h>
#endif

/* --------------------------------------------------------------------- */

static const char *name_audio = "/dev/dsp";

static int sample_rate;
static float cpu_mhz;
static float tvcorr;
static float scorr;
static float inp_freq;

/* --------------------------------------------------------------------- */

static void die(const char *func) __attribute__ ((noreturn));

static void die(const char *func)
{
	perror(func);
	exit(1);
}

/* --------------------------------------------------------------------- */
#ifdef __i386__

static int rdtsc_ok = 0;
static int dis_rdtsc = 0;

static void sig_ill(int sig)
{
	exit(1);
}

static void rdtsc_probe(void) 
{
	unsigned int tmp0, tmp1, tmp2, tmp3;
	pid_t pid;
	int i;

	if (!dis_rdtsc) {
		switch (pid = fork()) {
		case -1:
			die("fork");
			
		case 0:
			signal(SIGILL, sig_ill);
			__asm__("rdtsc" : "=a" (tmp0), "=d" (tmp1));
			__asm__("rdtsc" : "=a" (tmp2), "=d" (tmp3));
			/*__asm__(".byte 0xf,0xff"); this is the illegal instruction */
			signal(SIGILL, SIG_DFL);
			if (tmp0 == tmp2 && tmp1 == tmp3) 
				exit(2);
			exit(0);
			
		default:
			if (-1 == wait4(pid, &i, 0, NULL))
				die("wait4");
		}
		rdtsc_ok = !i;
		switch (i) {
		case 0:
			fprintf(stderr, "Your CPU supports the RDTSC instruction.\n");
			return;

		case 2:
			fprintf(stderr, "RDTSC present, but does not change!\n");
			break;
			
		default:
			fprintf(stderr, "Illegal instruction exception caught at trying RDTSC\n");
			break;
		}
	}
}

extern __inline__ unsigned long long rdtsc_get(void)
{
	unsigned int tmp0, tmp1;

	if (!rdtsc_ok)
		return 0;
	__asm__("rdtsc" : "=a" (tmp0), "=d" (tmp1));
	return ((unsigned long long)tmp1 << 32) | tmp0;
}

#else /* __i386__ */

extern __inline__ unsigned long long rdtsc_get(void)
{
	return 0;
}

extern __inline__ void rdtsc_probe(void) 
{
}

#endif /* __i386__ */
/* --------------------------------------------------------------------- */

extern __inline__ unsigned int hweight32(unsigned int w) 
{
        unsigned int res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
        res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
        res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
        res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
        return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
}

extern __inline__ unsigned int hweight16(unsigned short w)
{
        unsigned short res = (w & 0x5555) + ((w >> 1) & 0x5555);
        res = (res & 0x3333) + ((res >> 2) & 0x3333);
        res = (res & 0x0F0F) + ((res >> 4) & 0x0F0F);
        return (res & 0x00FF) + ((res >> 8) & 0x00FF);
}

extern __inline__ unsigned int hweight8(unsigned char w)
{
        unsigned short res = (w & 0x55) + ((w >> 1) & 0x55);
        res = (res & 0x33) + ((res >> 2) & 0x33);
        return (res & 0x0F) + ((res >> 4) & 0x0F);
}

/* --------------------------------------------------------------------- */


/*
 * This fft routine is from ~gabriel/src/filters/fft/fft.c;
 * I am unsure of the original source.  The file contains no
 * copyright notice or description.
 * The declaration is changed to the prototype form but the
 * function body is unchanged.  (J. T. Buck)
 */

#define SWAP(a, b) tempr=(a); (a)=(b); (b)=tempr

/*
 * Replace data by its discrete Fourier transform, if isign is
 * input as 1, or by its inverse discrete Fourier transform, if 
 * "isign" is input as -1.  "data'"is a complex array of length "nn",
 * input as a real array data[0..2*nn-1]. "nn" MUST be an integer
 * power of 2 (this is not checked for!?)
 */

static void fft_rif(float *data, int nn, int isign) 
{
        int n;
        int mmax;
        int m, j, istep, i;
        float wtemp, wr, wpr, wpi, wi, theta;
        float tempr, tempi;

        data--;
        n = nn << 1;
        j = 1;

        for (i = 1; i < n; i += 2) {
                if(j > i) {
                        SWAP(data[j], data[i]);
                        SWAP(data[j+1], data[i+1]);
                }
                m= n >> 1;
                while (m >= 2 && j >m) {
                        j -= m;
                        m >>= 1;
                }
                j += m;
        }
        mmax = 2;
        while (n > mmax) {
                istep = 2*mmax;
                theta = -6.28318530717959/(isign*mmax);
                wtemp = sin(0.5*theta);
                wpr = -2.0*wtemp*wtemp;
                wpi = sin(theta);
                wr = 1.0;
                wi = 0.0;
                for (m = 1; m < mmax; m += 2) {
                        for (i = m; i < n; i += istep) {
                                j = i + mmax;
                                tempr = wr*data[j] - wi*data[j+1];
                                tempi = wr*data[j+1] + wi*data[j];
                                data[j] = data[i] - tempr;
                                data[j+1] = data[i+1] - tempi;
                                data[i] += tempr;
                                data[i+1] += tempi;
                        }
                        wr = (wtemp=wr)*wpr - wi*wpi+wr;
                        wi = wi*wpr + wtemp*wpi + wi;
                }
                mmax = istep;
        }
}
        
#undef SWAP

/* --------------------------------------------------------------------- */

extern __inline__ float fsqr(float x)
{
	return x*x;
}

/* --------------------------------------------------------------------- */

#define FFT_SIZE (1<<18)

static int process_input(short *s, unsigned int samples,
			 unsigned long long tm1, unsigned int tm1inc, 
			 unsigned long long tm2, unsigned int tm2inc,
			 unsigned long long tm3, unsigned int tm3inc)
{
        struct sched_param schp;
	int i, j, idx;
	static float fft_data[2*FFT_SIZE];
	static unsigned int fft_ptr = 0;
	static unsigned long long tm1start;
	static unsigned long long tm2start;
	static unsigned long long tm3start;
	float f, pwr;

	if (!fft_ptr) {
		tm1start = tm1;
		tm2start = tm2;
		tm3start = tm3;
	}
	for (; samples > 0; samples--, s++, tm1 += tm1inc, tm2 += tm2inc, tm3 += tm3inc) {
		if (fft_ptr >= FFT_SIZE) {
			/*
			 * set non-realtime sched
			 */
			memset(&schp, 0, sizeof(schp));
			schp.sched_priority = sched_get_priority_min(SCHED_OTHER);
			if (sched_setscheduler(0, SCHED_OTHER, &schp) != 0) 
				perror("sched_setscheduler");
			for (i = 0; i < FFT_SIZE; i++)
				fft_data[2*i] *= 0.54 - 0.46 * cos(M_PI * 2.0 * i / (FFT_SIZE-1));
			fft_rif(fft_data, FFT_SIZE, 1);
			i = FFT_SIZE * inp_freq / (float)sample_rate;
			for (j = i/10, idx = 0, pwr = 0; j >= 0; j--) {
				f = fsqr(fft_data[2*(i-j)]) + fsqr(fft_data[2*(i-j)+1]);
				if (f > pwr) {
					idx = i-j;
					pwr = f;
				}
				f = fsqr(fft_data[2*(i+j)]) + fsqr(fft_data[2*(i+j)+1]);
				if (f > pwr) {
					idx = i+j;
					pwr = f;
				}
			}
			printf("peak index: %d\n", idx);
			f = (float)idx / inp_freq;
			tvcorr = (float)idx / inp_freq / (tm2 - tm2start) * 1000000;
			cpu_mhz = (tm3 - tm3start) / (float)idx * inp_freq;
			printf("gettimeofday correction: %10.7f  CPU clock: %10.1f\n"
			       "(kernel driver command line params: scale_tvusec=%u scale_rdtsc=%u)\n",
			       tvcorr, cpu_mhz, (int)((float)(1<<24)*tvcorr), (int)((float)(1ULL<<32)/cpu_mhz));
			return 1;
		}
		fft_data[2*fft_ptr] = *s;
		fft_data[2*fft_ptr+1] = 0;
		fft_ptr++;
	}
	return 0;
}

/* --------------------------------------------------------------------- */

#if __BYTE_ORDER == __BIG_ENDIAN
#define AUDIO_FMT AFMT_S16_BE
#else
#define AUDIO_FMT AFMT_S16_LE
#endif
#define SAMPLE_RATE 8000

/* --------------------------------------------------------------------- */

extern __inline__ int ld2(unsigned int x)
{
	int res = 0;

	if (!x)
		return -1;
	if (x >= 0x10000) {
		res += 16;
		x >>= 16;
	}
	if (x >= 0x100) {
		res += 8;
		x >>= 8;
	}
	if (x >= 0x10) {
		res += 4;
		x >>= 4;
	}
	if (x >= 0x4) {
		res += 2;
		x >>= 2;
	}
	if (x >= 0x2)
		res += 1;
	return res;
}

/* --------------------------------------------------------------------- */

static void sound_input(void)
{
	int i, apar;
	int fd;
	struct audio_buf_info info;
	struct count_info cinfo;
	unsigned int size;
	short *abuf, *s;
	fd_set mask;
	unsigned int fragptr;
	unsigned int curfrag, sampdelay;
        struct sched_param schp;
	/* timing variables */
	unsigned long long tm1;
	unsigned long long tm2;
	unsigned int tm2inc;
	unsigned long long tm3;
	unsigned int tm3inc;
	unsigned int last_usec;
	struct timeval tv;

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
	if (apar != AUDIO_FMT) {
		fprintf(stderr, "audio driver does not support the S16 format\n");
		exit(1);
	}
	apar = 0;
	if (ioctl(fd, SNDCTL_DSP_STEREO, &apar) == -1)
		die("ioctl: SNDCTL_DSP_STEREO");
	if (apar != 0) {
		fprintf(stderr, "audio driver does not support mono\n");
		exit(1);
	}
	sample_rate = 44100;
	if (ioctl(fd, SNDCTL_DSP_SPEED, &sample_rate) == -1)
		die("ioctl: SNDCTL_DSP_SPEED");
	if (sample_rate != 44100) {
		if (abs(sample_rate-44100) >= 44100/100) {
				fprintf(stderr, "audio driver does not support the required "
					"sampling frequency\n");
				exit(1);
		}
		fprintf(stderr, "audio driver inexact sampling rate: %d Hz\n", sample_rate);
	}
	tm2inc = (1000000 + sample_rate / 2) / sample_rate;
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
	fprintf(stderr, "OSS: input: #frag: %d  fragsz: %d  bufaddr: %p\n",
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
	tm2 = rdtsc_get();
	curfrag = cinfo.ptr;
	/* wait one interrupt */
	do {
		FD_ZERO(&mask);
		FD_SET(fd, &mask);
		i = select(fd+1, &mask, NULL, NULL, NULL);
		if (i < 0) 
			die("select");
	} while (!FD_ISSET(fd, &mask));
	if (ioctl(fd, SNDCTL_DSP_GETIPTR, &cinfo) == -1)
		die("ioctl: SNDCTL_DSP_GETIPTR");
	tm3 = rdtsc_get();
        if (gettimeofday(&tv, NULL))
		die("gettimeofday");
	curfrag = (cinfo.ptr - curfrag) / 2;
	tm3inc = (tm3 - tm2 + curfrag / 2) / curfrag;
	printf("first CPU clock guess: %9d Hz  tsc diff: %d  sample diff: %d\n", tm3inc * sample_rate, 
	       (unsigned int)(tm3 - tm2), curfrag);
	tm2 = 0;
	last_usec = tv.tv_usec;
	tm1 = 0;
	curfrag = cinfo.ptr / info.fragsize;
	/*
	 * loop
	 */
	for (;;) {
		/*
		 * process input
		 */
		sampdelay = ((size + cinfo.ptr - fragptr * info.fragsize) % size) / 2;
		while (fragptr != curfrag) {
			s = abuf + (fragptr * info.fragsize / 2);
			if (process_input(s, info.fragsize / 2, tm1, 1, 
					  tm2 - tm2inc * sampdelay, tm2inc, tm3  - tm3inc * sampdelay, tm3inc))
				goto loop_end;
			fragptr = (fragptr + 1) % info.fragstotal;
			tm1 += info.fragsize / 2;
			sampdelay -= info.fragsize / 2;
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
		tm3 = rdtsc_get();
		if (gettimeofday(&tv, NULL))
			die("gettimeofday");
		curfrag = cinfo.ptr / info.fragsize;
		tm2 += (1000000 + tv.tv_usec - last_usec) % 1000000;
		last_usec = tv.tv_usec;
	}
loop_end:
	if (munmap((caddr_t)abuf, info.fragsize * info.fragstotal))
		die("munmap");
	if (close(fd))
		die("close");
	/*
	 * try to calibrate the sample correction factor at 8kHz
	 */
	/*
	 * set realtime sched
	 */
	memset(&schp, 0, sizeof(schp));
        schp.sched_priority = sched_get_priority_min(SCHED_RR);
        if (sched_setscheduler(0, SCHED_RR, &schp) != 0) 
                perror("sched_setscheduler");
	if ((fd = open(name_audio, O_RDWR, 0)) < 0)
		die("open");
	/*
	 * configure audio
	 */
	apar = AUDIO_FMT;
	if (ioctl(fd, SNDCTL_DSP_SETFMT, &apar) == -1)
		die("ioctl: SNDCTL_DSP_SETFMT");
	if (apar != AUDIO_FMT) {
		fprintf(stderr, "audio driver does not support the S16 format\n");
		exit(1);
	}
	apar = 0;
	if (ioctl(fd, SNDCTL_DSP_STEREO, &apar) == -1)
		die("ioctl: SNDCTL_DSP_STEREO");
	if (apar != 0) {
		fprintf(stderr, "audio driver does not support mono\n");
		exit(1);
	}
	sample_rate = SAMPLE_RATE;
	if (ioctl(fd, SNDCTL_DSP_SPEED, &sample_rate) == -1)
		die("ioctl: SNDCTL_DSP_SPEED");
	if (sample_rate != SAMPLE_RATE) {
		if (abs(sample_rate-SAMPLE_RATE) >= SAMPLE_RATE/100) {
				fprintf(stderr, "audio driver does not support the required "
					"sampling frequency\n");
				exit(1);
		}
		fprintf(stderr, "audio driver inexact sampling rate: %d Hz\n", sample_rate);
	}
	tm2inc = (1000000 + sample_rate / 2) / sample_rate;
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
	fprintf(stderr, "OSS: input: #frag: %d  fragsz: %d  bufaddr: %p\n",
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
        if (gettimeofday(&tv, NULL))
		die("gettimeofday");
	tm2 = 0;
	last_usec = tv.tv_usec;
	tm1 = 0;
	curfrag = cinfo.ptr;
	/*
	 * loop
	 */
	do {
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
		if (gettimeofday(&tv, NULL))
			die("gettimeofday");
		tm1 += (size + cinfo.ptr - curfrag) % size;
		curfrag = cinfo.ptr;
		tm2 += (1000000 + tv.tv_usec - last_usec) % 1000000;
		last_usec = tv.tv_usec;
	} while (tm2 < 1000000);
	scorr = 2.0 * (tm2 * tvcorr) / (tm1 * 1000000.0 / sample_rate);
	if (munmap((caddr_t)abuf, info.fragsize * info.fragstotal))
		die("munmap");
	if (close(fd))
		die("close");
	printf("Sample corr: %10.7f\n", scorr);
}

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
        int c;
        int err = 0;

        while ((c = getopt(argc, argv, "a:f:")) != -1) 
                switch (c) {
		case 'a':
			name_audio = optarg;
			break;

		case 'f':
			inp_freq = strtod(optarg, NULL);
			if (inp_freq < 20 || inp_freq > 22000) {
				fprintf(stderr, "input frequency out of range\n");
				err++;
			}
			break;

		default:
                        err++;
                        break;
                }
        if (err) {
                fprintf(stderr, "usage: reffreq [-a <audio device>]\n\n"
			"  -a: audio device path (default: /dev/dsp)\n"
			"  -f: frequency\n");
                exit(1);
        }
	rdtsc_probe();
	sound_input();
	exit(0);
}

/* --------------------------------------------------------------------- */
