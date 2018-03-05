/*****************************************************************************/

/*
 *      channel.c  --  Channel - connect two hfkernel_link instances with two
 *                     Watterson simulated channels
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
 * This file now implements a real watterson channel model. There can
 * be an arbitrary number of path components (maximum number compile
 * time constant NUMPATH) with a maximum relative delay of MAXDELAY
 * samples (currently 10ms). Each path component is raleigh-faded
 * independently with an individual fading rate (selectable as integer
 * divider of SAMPLE_RATE/100, i.e. 80Hz). It has its own gain and
 * doppler shift. There is currently only a gaussian noise source 
 * (i.e. no impulsive noise) of bandwidth 4kHz.
 *
 * There are default simulator parameters corresponding to the well known
 * CCIR "good", "moderate", "poor" and "flutter fading" channels (at
 * least that's how I understand them :-))
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "chan_tables.h"

#include <fcntl.h>

#ifdef HAVE_SYS_AUDIOIO_H
#include <sys/audioio.h>
#include <stropts.h>
#include <sys/conf.h>
#endif

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#ifdef __linux__
#include <endian.h>
#endif
#ifdef __FreeBSD__
#include <sys/endian.h>
#endif
#endif

#ifdef DISABLE_REALTIME
#undef HAVE_SCHED_H
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

/* --------------------------------------------------------------------- */

#define SAMPLE_RATE 8000
#define BLOCKSZ 32            /* do not increase blindly - see l1/linked/ */
#define SIM_SLOWDOWN 1
#define NUMPATH 3
#define MAXDELAY (SAMPLE_RATE/100)      /* this gives max. 10ms delay */
#define HILBERTLEN 48

/* --------------------------------------------------------------------- */

static const __complex__ float hilbert_filter[HILBERTLEN] = {
       0.000851 -0.000851i,
      -0.001650 -0.001650i,
      -0.000394 +0.000394i,
      -0.002127 -0.002127i,
      -0.002768 +0.002768i,
      -0.000850 -0.000850i,
      -0.005556 +0.005556i,
       0.004049 +0.004049i,
      -0.005146 +0.005146i,
       0.011408 +0.011408i,
       0.002819 -0.002819i,
       0.014791 +0.014791i,
       0.018076 -0.018076i,
       0.005156 +0.005156i,
       0.031430 -0.031430i,
      -0.021585 -0.021585i,
       0.026261 -0.026261i,
      -0.056852 -0.056852i,
      -0.014070 +0.014070i,
      -0.076459 -0.076459i,
      -0.101653 +0.101654i,
      -0.034235 -0.034235i,
      -0.291626 +0.291626i,
       0.499675 +0.499675i,
       0.499675 -0.499675i,
      -0.291626 -0.291626i,
      -0.034235 +0.034235i,
      -0.101653 -0.101654i,
      -0.076459 +0.076459i,
      -0.014070 -0.014070i,
      -0.056852 +0.056852i,
       0.026261 +0.026261i,
      -0.021585 +0.021585i,
       0.031430 +0.031430i,
       0.005156 -0.005156i,
       0.018076 +0.018076i,
       0.014791 -0.014791i,
       0.002819 +0.002819i,
       0.011408 -0.011408i,
      -0.005146 -0.005146i,
       0.004049 -0.004049i,
      -0.005556 -0.005556i,
      -0.000850 +0.000850i,
      -0.002768 -0.002768i,
      -0.002127 +0.002127i,
      -0.000394 -0.000394i,
      -0.001650 +0.001650i,
       0.000851 +0.000851i
};

/* --------------------------------------------------------------------- */

struct gaussgen_state {
	float rndv[GAUSSLPFLEN/GAUSSLPFOVER];
	float gval, gvalold;
	unsigned int rndcnt;
	unsigned int boxcnt, boxmax;
	float boxgcorr;
};

/* --------------------------------------------------------------------- */

struct path_par {
	unsigned int tap;
	unsigned int doppler;
	unsigned int fade_freq;
	float gain;
};

static struct sim_par {
	struct path_par path[NUMPATH];
	float gauss_noise_gain;
	float direct_gain;
} sim_par[2];

/* --------------------------------------------------------------------- */

struct path_state {
	struct gaussgen_state gs[2];
	unsigned int dphase;
};

static struct sim_state {
	float inf[BLOCKSZ+HILBERTLEN];
	__complex__ float dly[BLOCKSZ+MAXDELAY];
	struct path_state path[NUMPATH];
	float pwrinp, pwrnoise;
	unsigned int pwrcnt;
} sim_st[2];

/* --------------------------------------------------------------------- */

static void die(const char *str) __attribute__ ((noreturn));

static void die(const char *str)
{
	perror(str);
	exit(1);
}

/* --------------------------------------------------------------------- */

static __inline__ int gauss_noise(void)
{
	int sum, i;
	
	for (sum = i = 0; i < 16; i++)
		sum += random() & 0x7ff;
	return sum - 0x4000;
}

/* --------------------------------------------------------------------- */

/*
 * Approximation of a normally distributed random number generator
 * with unit variance
 *
 * division factor: randommax * sqrt(nrand / 12)
 * with nrand = 16 and randommax = 0x1000
 */

extern __inline__ float randn(void)
{
        int sum = 0, i;

        for (i = 0; i < 16; i++)
                sum += random() & 0xfff;
        return (sum - 0x8000) / 4729.7;
}

/* --------------------------------------------------------------------- */

static void gaussgen_init(struct gaussgen_state *gs, unsigned int fsdiv)
{
	unsigned int u;

	memset(gs, 0, sizeof(*gs));
	gs->boxcnt = gs->boxmax = fsdiv / GAUSSLPF3DBFDIV;
	gs->boxgcorr = 1.0 / gs->boxmax;
	for (u = 0; u < GAUSSLPFLEN/GAUSSLPFOVER; u++)
		gs->rndv[u] = randn();
}

/* --------------------------------------------------------------------- */

static float gaussgen_value(struct gaussgen_state *gs)
{
	const float *f1, *f2;
	float f;
	unsigned int u;

	if ((++gs->boxcnt) < gs->boxmax)
		return (gs->gval * gs->boxcnt + gs->gvalold * (gs->boxmax - gs->boxcnt)) * gs->boxgcorr;
	gs->boxcnt = 0;
#if 1
	if (!gs->rndcnt) {
		memmove(gs->rndv+1, gs->rndv, sizeof(gs->rndv)-sizeof(gs->rndv[0]));
		gs->rndv[0] = randn();
	}
	f1 = gaussfilttbl[gs->rndcnt];
	gs->rndcnt = (gs->rndcnt + 1) % GAUSSLPFOVER;
	f2 = gs->rndv;
	for (f = u = 0; u < GAUSSLPFLEN/GAUSSLPFOVER; u++)
		f += (*f1++) * (*f2++);
#else
	f = gs->gval > 0 ? 0 : 1;
#endif
	gs->gvalold = gs->gval;
	gs->gval = f;
	return gs->gvalold;
}

/* --------------------------------------------------------------------- */

extern __inline__ float gain2db(float f)
{
	if (f < 0.0000000001)
		return -200;
	else 
		return 20*log10(f);
}

/* --------------------------------------------------------------------- */

static void sim_init(struct sim_state *st, struct sim_par *par)
{
	unsigned int i;

	memset(st, 0, sizeof(*st));
	for (i = 0; i < NUMPATH; i++) {
		if (par->path[i].gain <= 0)
			continue;
		if (par->path[i].tap > MAXDELAY) {
			fprintf(stderr, "Simulation parameter error (tap delay too long)\n");
			exit(1);
		}
		if (par->path[i].fade_freq < (SAMPLE_RATE/10)) {
			fprintf(stderr, "Simulation parameter error (fading frequency too high)\n");
			exit(1);
		}
		gaussgen_init(st->path[i].gs, par->path[i].fade_freq);
		gaussgen_init(st->path[i].gs+1, par->path[i].fade_freq);
	}
	printf("Simulation parameters\n"
	       "Gaussian noise gain: %10.6fdB\n"
	       "Direct signal gain:  %10.6fdB\n",
	       gain2db(par->gauss_noise_gain), gain2db(par->direct_gain));
	for (i = 0; i < NUMPATH; i++) {
		if (par->path[i].gain <= 0)
			continue;		
		printf("Tap %1u: delay: %5.2fms  fading freq: %5.2fHz  "
		       "doppler: %6.2fHz  gain: %10.6fdB\n",
		       i, par->path[i].tap * (1000.0 / SAMPLE_RATE),
		       (float)(SAMPLE_RATE)/par->path[i].fade_freq,
		       (par->path[i].doppler & 0xffff) * (SAMPLE_RATE / 65536.0),
		       gain2db(par->path[i].gain));
	}
}

/* --------------------------------------------------------------------- */

static struct sim_par ch_default = {
	{ { 0, 0, 0, 0 }, },
	0, 1
};

static struct sim_par ch_ccir_good = {
	{ { 0, 0, 10*SAMPLE_RATE, 1 }, { SAMPLE_RATE/2000, 0, 10*SAMPLE_RATE, 1 }, },
	0, 0
};

static struct sim_par ch_ccir_moderate = {
	{ { 0, 0, 2*SAMPLE_RATE, 1 }, { SAMPLE_RATE/1000, 0, 2*SAMPLE_RATE, 1 }, },
	0, 0
};

static struct sim_par ch_ccir_poor = {
	{ { 0, 0, SAMPLE_RATE, 1 }, { SAMPLE_RATE/500, 0, SAMPLE_RATE, 1 }, },
	0, 0
};

static struct sim_par ch_ccir_flutter = {
	{ { 0, 0, SAMPLE_RATE/10, 1 }, { SAMPLE_RATE/2000, 0, SAMPLE_RATE/10, 1 }, },
	0, 0
};

static struct sim_par ch_singlepath_good = {
	{ { 0, 0, 10*SAMPLE_RATE, 1 }, },
	0, 0
};

static struct sim_par ch_singlepath_moderate = {
	{ { 0, 0, 2*SAMPLE_RATE, 1 }, },
	0, 0
};

static struct sim_par ch_singlepath_poor = {
	{ { 0, 0, SAMPLE_RATE, 1 }, },
	0, 0
};

/* --------------------------------------------------------------------- */

static void hfchannel(struct sim_state *st, struct sim_par *par, 
		      short *in, short *out)
{
	__complex__ float cs, cft, *dp;
	__complex__ const float *cp;
	float s, *fp;
	unsigned int u, k;
	
	/* convert input to float */
	memmove(st->inf, st->inf+BLOCKSZ, HILBERTLEN*sizeof(st->inf[0]));
	for (u = 0, fp = st->inf+HILBERTLEN; u < BLOCKSZ; u++, fp++, in++)
		*fp = *in;
	/* calculate hilbert filter */
	memmove(st->dly, st->dly+BLOCKSZ, MAXDELAY*sizeof(st->dly[0]));
	dp = st->dly+MAXDELAY;
	for (u = 0; u < BLOCKSZ; u++, in++, dp++) {
		for (cs = 0, cp = hilbert_filter, fp = st->inf+u, k = 0; k < HILBERTLEN; k++)
			cs += (*fp++) * (*cp++);
		*dp = cs;
	}
	/* calculate output samples */
	dp = st->dly;
       	for (u = 0; u < BLOCKSZ; u++, out++, dp++) {
		s = par->gauss_noise_gain * randn() * (32768.0 / 1.414);
		st->pwrnoise += s * s;
		st->pwrinp += __real__ dp[0] * __real__ dp[0];
		s += par->direct_gain * (__real__ *dp);	
		/* for every path */
		for (k = 0; k < NUMPATH; k++) {
			if (par->path[k].gain <= 0)
				continue;
			__real__ cft = gaussgen_value(st->path[k].gs);
			__imag__ cft = gaussgen_value(st->path[k].gs+1);
			cs = cft * dp[par->path[k].tap];
			st->path[k].dphase = (st->path[k].dphase + par->path[k].doppler) & 0xffff;
			__real__ cft = COS(st->path[k].dphase);
			__imag__ cft = SIN(st->path[k].dphase);
			cs *= cft;
			s += (__real__ cs) * par->path[k].gain;
		}
		s *= 0.1;
		if (fabs(s) > 32767) {
			printf("sample overflow: %10.5f\n", s);
			if (s > 0)
				s = 32767;
			else
				s = -32767;
		}
		*out = s;
		if ((++st->pwrcnt) >= SAMPLE_RATE) {
			float np, sp;
			
			if (st->pwrinp <= 0)
				sp = -1000000;
			else 
				sp = 10*log10(st->pwrinp);
			if (st->pwrnoise <= 0)
				np = -1000000;
			else 
				np = 10*log10(st->pwrnoise);
			printf("channel: signal power %8.3fdB  noise power %8.3fdB  SNR %8.3fdB\n",
			       sp, np, sp-np);
			st->pwrcnt = 0;
			st->pwrnoise = st->pwrinp = 0;
		}
	}
}

/* --------------------------------------------------------------------- */

static int do_one_block(int maxfd, int fd1, int fd2, int audio_fd)
{
	fd_set mask;
	union {
		short s[BLOCKSZ];
		unsigned char c[1];
	} blk1, blk2;
	union {
		short s[2*BLOCKSZ];
		unsigned char c[1];
	} abuf;
	short *sp1, *sp2, *sp3;
	unsigned int ptr1, ptr2, ptr3;
	int i;

	ptr1 = ptr2 = 0;
	while(ptr1 < sizeof(blk1) || ptr2 < sizeof(blk2)) {
		FD_ZERO(&mask);
		if (ptr1 < sizeof(blk1))
			FD_SET(fd1, &mask);
		if (ptr2 < sizeof(blk2))
			FD_SET(fd2, &mask);
		i = select(maxfd, &mask, NULL, NULL, NULL);
		if (i < 0)
			die("select");
		if (FD_ISSET(fd1, &mask)) {
			i = read(fd1, blk1.c+ptr1, sizeof(blk1)-ptr1);
			if (i < 0) {
				if (errno == EPIPE) {
					printf("conn 1 closed\n");
					return -1;
				} else if (errno != EAGAIN)
					die("read");
			} else 
				ptr1 += i;
		}
		if (FD_ISSET(fd2, &mask)) {
			i = read(fd2, blk2.c+ptr2, sizeof(blk2)-ptr2);
			if (i < 0) {
				if (errno == EPIPE) {
					printf("conn 2 closed\n");
					return -1;
				} else if (errno != EAGAIN)
					die("read");
			} else 
				ptr2 += i;
		}
	}
	hfchannel(sim_st, sim_par, blk1.s, blk1.s);
	hfchannel(sim_st+1, sim_par+1, blk2.s, blk2.s);
	/* copy data to audio buffer */
	if (audio_fd >= 0) {
		sp1 = blk1.s;
		sp2 = blk2.s;
		sp3 = abuf.s;
		for (i = 0; i < BLOCKSZ; i++) {
			*sp3++ = *sp1++;
			*sp3++ = *sp2++;
		}
	}
	ptr1 = ptr2 = ptr3 = 0;
	while (ptr1 < sizeof(blk2) || ptr2 < sizeof(blk1) || 
	       (ptr3 < sizeof(abuf) && audio_fd >= 0)) {
		FD_ZERO(&mask);
		if (ptr1 < sizeof(blk2))
			FD_SET(fd1, &mask);
		if (ptr2 < sizeof(blk1))
			FD_SET(fd2, &mask);
		if (ptr3 < sizeof(abuf) && audio_fd >= 0)
			FD_SET(audio_fd, &mask);
		i = select(maxfd, NULL, &mask, NULL, NULL);
		if (i < 0)
			die("select");
		if (FD_ISSET(fd1, &mask)) {
			i = write(fd1, blk2.c+ptr1, sizeof(blk2)-ptr1);
			if (i < 0) {
				if (errno == EPIPE) {
					printf("conn 1 closed\n");
					return -1;
				} else if (errno != EAGAIN)
					die("write");
			} else 
				ptr1 += i;
		}
		if (FD_ISSET(fd2, &mask)) {
			i = write(fd2, blk1.c+ptr2, sizeof(blk1)-ptr2);
			if (i < 0) {
				if (errno == EPIPE) {
					printf("conn 2 closed\n");
					return -1;
				} else if (errno != EAGAIN)
					die("write");
			} else 
				ptr2 += i;
		}
		if (audio_fd >= 0 && FD_ISSET(audio_fd, &mask)) {
			i = write(audio_fd, abuf.c+ptr3, sizeof(abuf)-ptr3);
			if (i < 0) {
				if (errno == EPIPE) {
					printf("audio interface closed\n");
					return -1;
				} else if (errno != EAGAIN)
					die("write");
			} else 
				ptr3 += i;
		}
	}
	return 0;
}

/* --------------------------------------------------------------------- */

static void do_channel(const char *name, unsigned int perm, int audio_fd)
{
	fd_set mask;
	int i, j, fd1 = -1, fd2 = -1, maxfd, sock;
	struct sockaddr_un saddr;
	struct timeval tv1, tv2;
	unsigned int u, exp;

	if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		die("socket");
	saddr.sun_family = AF_UNIX;
	strncpy(saddr.sun_path, name, sizeof(saddr.sun_path));
	unlink(saddr.sun_path);
	if (bind(sock, (struct sockaddr *)&saddr, sizeof(saddr)))
		die("bind");
	if (listen(sock, 2))
		die("listen");
	chmod(saddr.sun_path, perm);
	/*fchmod(sock, comm_perm);*/
	sim_init(sim_st, sim_par);
	sim_init(sim_st+1, sim_par+1);
	/*
	 * first wait until both sockets are open
	 */
	while (fd1 < 0 || fd2 < 0) {
		FD_ZERO(&mask);
		FD_SET(sock, &mask);
		i = select(sock+1, &mask, NULL, NULL, NULL);
		if (i < 0)
			die("select");
		if (FD_ISSET(sock, &mask)) {
			i = sizeof(saddr);
			if ((j = accept(sock, (struct sockaddr *)(&saddr), &i)) < 0)
				die("accept");
			printf("connect from %s fam %d\n", saddr.sun_path, saddr.sun_family);
			if (fd1 < 0) 
				fd1 = j;
			else if (fd2 < 0)
				fd2 = j;
			continue;
		}
	}
	maxfd = fd1;
	if (fd2 > maxfd)
		maxfd = fd2;
	if (audio_fd >= 0 && audio_fd > maxfd)
		maxfd = audio_fd;
	maxfd++;
	printf("start...\n");
	if (gettimeofday(&tv2, NULL))
		die("gettimeofday");
	if (audio_fd >= 0) {
		/* if audio interface is on, audio does slow down the simulator */
		while (!do_one_block(maxfd, fd1, fd2, audio_fd));
		goto out;
	}
	for (;;) {
		/*
		 * process one block
		 */
		tv1 = tv2;
		exp = 0;
		for (i = 0; i < 100; i++, exp += (SIM_SLOWDOWN*1000000/SAMPLE_RATE*BLOCKSZ/2)) {
			if (do_one_block(maxfd, fd1, fd2, -1))
				goto out;
			if (gettimeofday(&tv2, NULL))
				die("gettimeofday");
			u = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
			if (u < exp)
				usleep(exp-u);
		}
	}
out:
	close(fd1);
	close(fd2);
	if (audio_fd >= 0)
		close(audio_fd);
}

/* ---------------------------------------------------------------------- */
#ifdef HAVE_SYS_AUDIOIO_H

static int open_sound(const char *ifname)
{
        audio_info_t audioinfo;
        audio_info_t audioinfo2;
        audio_device_t audiodev;
        int fd;

        if ((fd = open(ifname ? ifname : "/dev/audio", O_WRONLY)) < 0) {
                perror("open");
                return -1;
        }
        if (ioctl(fd, AUDIO_GETDEV, &audiodev) == -1) {
                perror("ioctl: AUDIO_GETDEV");
                return -1;
        }
        AUDIO_INITINFO(&audioinfo);
        audioinfo.record.sample_rate = SAMPLE_RATE;
        audioinfo.record.channels = 2;
        audioinfo.record.precision = 16;
        audioinfo.record.encoding = AUDIO_ENCODING_LINEAR;
        /*audioinfo.record.gain = 0x20;
          audioinfo.record.port = AUDIO_LINE_IN;
          audioinfo.monitor_gain = 0;*/
        if (ioctl(fd, AUDIO_SETINFO, &audioinfo) == -1) {
                perror("ioctl: AUDIO_SETINFO");
                return -1;
        }     
        if (ioctl(fd, I_FLUSH, FLUSHW) == -1) {
                perror("ioctl: I_FLUSH");
                return -1;
        }
        if (ioctl(fd, AUDIO_GETINFO, &audioinfo2) == -1) {
                perror("ioctl: AUDIO_GETINFO");
                return -1;
        }
        fprintf(stdout, "Audio device: name %s, ver %s, config %s, "
                "sampling rate %d\n", audiodev.name, audiodev.version,
                audiodev.config, audioinfo.record.sample_rate);
	return fd;
}

#endif
/* ---------------------------------------------------------------------- */
#ifdef HAVE_SYS_SOUNDCARD_H

static int open_sound(const char *ifname)
{
        int sndparam;
        int fd;
        int fmt;

        if ((fd = open(ifname ? ifname : "/dev/dsp", O_WRONLY)) < 0) {
                perror("open");
                return -1;
        }
#if defined(__BYTE_ORDER) && __BYTE_ORDER == 4321
        sndparam = fmt = AFMT_S16_BE; /* we want 16 bits/sample signed */
#else
        sndparam = fmt = AFMT_S16_LE; /* we want 16 bits/sample signed */
#endif
        if (ioctl(fd, SNDCTL_DSP_SETFMT, &sndparam) == -1) {
                perror("ioctl: SNDCTL_DSP_SETFMT");
                return -1;
        }
        if (sndparam != fmt) {
		perror("ioctl: SNDCTL_DSP_SETFMT");
		return -1;
	}
        sndparam = 1;
        if (ioctl(fd, SNDCTL_DSP_STEREO, &sndparam) == -1) {
                perror("ioctl: SNDCTL_DSP_STEREO");
                return -1;
        }
        if (sndparam != 1) {
                fprintf(stderr, "soundif: Error, cannot set the channel "
                        "number to 1\n");
                return -1;
        }
        sndparam = SAMPLE_RATE; 
        if (ioctl(fd, SNDCTL_DSP_SPEED, &sndparam) == -1) {
                perror("ioctl: SNDCTL_DSP_SPEED");
                return -1;
        }
        if ((10*abs(sndparam-SAMPLE_RATE)) > SAMPLE_RATE) {
                perror("ioctl: SNDCTL_DSP_SPEED");
                return -1;
        }
        if (sndparam != SAMPLE_RATE) {
                fprintf(stderr, "Warning: Sampling rate is %u, "
                        "requested %u\n", sndparam, SAMPLE_RATE);
        }
#if 0
        sndparam = 4;
        if (ioctl(fd, SOUND_PCM_SUBDIVIDE, &sndparam) == -1) {
                perror("ioctl: SOUND_PCM_SUBDIVIDE");
        }
        if (sndparam != 4) {
                perror("ioctl: SOUND_PCM_SUBDIVIDE");
        }
#endif
	return fd;
}
#endif

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
#if defined(HAVE_SCHED_H) && !defined(DISABLE_REALTIME)
	struct sched_param schp;
#endif
        int c;
        int err = 0;
	const char *name_ch = "hfchannel";
	unsigned int perm = 0600;
	unsigned int u;
	const char *audio_if = NULL;

	sim_par[0] = ch_default;
	sim_par[1] = ch_default;
        while ((c = getopt(argc, argv, "a:c:n:p:01234567f:")) != -1) 
                switch (c) {
		case 'a':
			audio_if = optarg;
			break;

                case 'c':
			name_ch = optarg;
			break;

                case 'p':
			perm = strtoul(optarg, NULL, 0);
			break;

		case 'n':
			sim_par[0].gauss_noise_gain = sim_par[1].gauss_noise_gain = 
				pow(10, strtod(optarg, NULL)/20);
			break;

		case 'f':
			sim_par[0].path[0].doppler = sim_par[1].path[0].doppler =
				strtod(optarg, NULL) * (65536.0 / SAMPLE_RATE);
			for (u = 1; u < NUMPATH; u++)
				sim_par[0].path[u].doppler = sim_par[1].path[u].doppler = 
					sim_par[0].path[0].doppler;
			break;

		case '0':
			sim_par[0] = ch_default;
			sim_par[1] = ch_default;
			break;

		case '1':
			sim_par[0] = ch_ccir_good;
			sim_par[1] = ch_ccir_good;
			break;

		case '2':
			sim_par[0] = ch_ccir_moderate;
			sim_par[1] = ch_ccir_moderate;
			break;

		case '3':
			sim_par[0] = ch_ccir_poor;
			sim_par[1] = ch_ccir_poor;
			break;

		case '4':
			sim_par[0] = ch_ccir_flutter;
			sim_par[1] = ch_ccir_flutter;
			break;

		case '5':
			sim_par[0] = ch_singlepath_good;
			sim_par[1] = ch_singlepath_good;
			break;

		case '6':
			sim_par[0] = ch_singlepath_moderate;
			sim_par[1] = ch_singlepath_moderate;
			break;

		case '7':
			sim_par[0] = ch_singlepath_poor;
			sim_par[1] = ch_singlepath_poor;
			break;

                default:
                        err++;
                        break;
                }
	if (err) {
		fprintf(stderr, "usage: channel [-a <audiodev>] [-c <name>] [-p <perm>] [-#] [-n <noise>] [-f <dopplerfreq>]\n");
		exit(1);
	}
#if defined(HAVE_SCHED_H) && !defined(DISABLE_REALTIME)
	memset(&schp, 0, sizeof(schp));
	schp.sched_priority = sched_get_priority_min(SCHED_RR) + 4;
	if (sched_setscheduler(0, SCHED_RR, &schp))
		perror("sched_setscheduler");
#endif
	do_channel(name_ch, perm, audio_if ? open_sound(audio_if) : -1);
	exit(0);
}
