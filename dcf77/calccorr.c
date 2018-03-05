/*****************************************************************************/

/*
 *      calccorr.c  --  Linux soundcard DCF77 receiver, deviation calculation.
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

#include <string.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include "dcf77.h"

/* --------------------------------------------------------------------- */

struct time_corr_state {
	unsigned int sr;
	unsigned long long tm1, tm2, tm3, tm3st;
	unsigned int tm2inc, tm3inc;
	unsigned int last_usec;

	time_t tstart;
	unsigned long long tm1start, tm2start, tm3start;
	time_t tcur;
} tc;

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
			vlprintf(1, "Your CPU supports the RDTSC instruction.\n");
			return;

		case 2:
			vlprintf(1, "RDTSC present, but does not change!\n");
			break;
			
		default:
			vlprintf(1, "Illegal instruction exception caught at trying RDTSC\n");
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

void tc_init(unsigned int sample_rate)
{
	rdtsc_probe();
	memset(&tc, 0, sizeof(tc));
	tc.tm2inc = (1000000 + sample_rate / 2) / sample_rate;
	tc.sr = sample_rate;
}

/* --------------------------------------------------------------------- */

void tc_start(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL))
		die("gettimeofday");
	tc.last_usec = tv.tv_usec;
	tc.tm3st = tc.tm3 = rdtsc_get();
}

/* --------------------------------------------------------------------- */

void tc_now(unsigned int minus_samples)
{
	struct timeval tv;

	tc.tm3 = rdtsc_get();
	if (gettimeofday(&tv, NULL))
		die("gettimeofday");
	tv.tv_usec -= minus_samples * tc.tm2inc;
	tc.tm2 += (2000000 + tv.tv_usec - tc.last_usec) % 1000000;
	tc.last_usec = tv.tv_usec;
	tc.tm3 -= tc.tm3inc * minus_samples;
	/* make an estimate for the CPU clock */
	if (tc.tm1 < 8000 && tc.tm3 > tc.tm3st && tc.tm1 > 0) {
		tc.tm3inc = (tc.tm3st - tc.tm3) / (tc.tm1+minus_samples);
		if (tc.tm3inc == 0)
			vlprintf(2, "First CPU clock estimate: %9dHz", 
				 tc.tm3inc * tc.sr);
	}
}

/* --------------------------------------------------------------------- */

void tc_advance_samples(unsigned int s)
{
	tc.tm1 += s;
	tc.tm2 += tc.tm2inc * s;
	tc.tm3 += tc.tm3inc * s;
}

/* --------------------------------------------------------------------- */

void tc_minute(time_t curtime, unsigned int samples)
{
	unsigned long long tm1c = tc.tm1 + samples;
	unsigned long long tm2c = tc.tm2 + samples * tc.tm2inc; 
	unsigned long long tm3c = tc.tm3 + samples * tc.tm3inc;
	unsigned int timediff;
	struct tm *tm;

	vlprintf(2, "tc_minute: curtime: %u\n", curtime);

	if (!tc.tm1start) {
		if (curtime == INVALID_TIME)
			return;
		tc.tm1start = tm1c;
		tc.tm2start = tm2c;
		tc.tm3start = tm3c;
		tc.tstart = tc.tcur = curtime;
		tm = localtime(&tc.tstart);
		vlprintf(1, "Corrections: start time %02ld:%02ld:%02ld\n", 
			 tm->tm_hour, tm->tm_min, tm->tm_sec);
		return;
	}
	if (curtime != INVALID_TIME)
		tc.tcur = curtime;
	else
		tc.tcur += 60;
	timediff = (tc.tcur - tc.tstart) % 86400;
	tm = localtime(&tc.tcur);
	vlprintf(1, "Corrections: current time %02ld:%02ld:%02ld, time diff %us, "
		 "sample corr: %10.8f, gettimeofday corr: %10.8f, CPU clock: %10.0f\n"
		 "(kernel driver command line params: scale_tvusec=%u scale_rdtsc=%u)\n",
		 tm->tm_hour, tm->tm_min, tm->tm_sec, timediff,
		 (float)tc.sr/((float)(tm1c - tc.tm1start)/(float)timediff),
		 1000000.0/((float)(tm2c - tc.tm2start)/(float)timediff),
		 (float)(tm3c - tc.tm3start)/(float)timediff, 
		 (int)((1<<24)*1000000.0/((float)(tm2c - tc.tm2start)/(float)timediff)), 
		 (int)((float)(1ULL<<32)*1000000.0/((float)(tm3c - tc.tm3start)/(float)timediff)));
}

/* --------------------------------------------------------------------- */
