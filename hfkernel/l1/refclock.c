 /*****************************************************************************/
/*
 *      refclock.c  --  Accurate reference clock for half duplex soundcards.

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

#include <stdlib.h> 
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <syslog.h>

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/* --------------------------------------------------------------------- */

/*
 * NOTE: These methods aren't perfect. The gettimeofday method
 * currently gives a discontinuity after about 12 days after start,
 * and the RDTSC method gives the discontinuity after about 584 years
 * after start for a 1GHz CPU.
 */

/* --------------------------------------------------------------------- */

static unsigned int scale_tvusec;
static unsigned int last_tvusec;
static unsigned long long time_cnt;

static l1_time_t lasttime;

#ifdef __i386__

static unsigned int starttime_lo, starttime_hi;
static unsigned int scale_rdtsc;

static int rdtsc_ok = 0;

/* --------------------------------------------------------------------- */

static void sig_ill(int sig)
{
	exit(1);
}

#endif /* __i386__ */

/* --------------------------------------------------------------------- */

void refclock_probe(float tvuseccorr, float cpumhz, int dis_rdtsc)
{
	struct timeval t1, t2;
	int i; 
	
	scale_tvusec = tvuseccorr * (1<<24);
#ifdef __i386__
	//errprintf(SEV_INFO, "RDTSC test: dis_rdtsc = %d\n", dis_rdtsc);
	if ( ! dis_rdtsc) {
		unsigned int tmp0 = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0;
		pid_t pid;

		scale_rdtsc = ((float)0x100000000LL)/cpumhz;
		switch (pid = fork()) {
		case -1:
			errstr(SEV_FATAL, "fork");
			
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
				errstr(SEV_FATAL, "wait4");
		}
		rdtsc_ok = !i;
		errprintf(SEV_INFO, "RDTSC test: i = %d\n",i);
		switch (i) {
		case 0:
		    errprintf(SEV_INFO, 
		    "\nYour CPU seems to support the RDTSC instruction at first test. Good.\n"
		    "Let's hope you entered the CPU clock correctly with %10.6f,\n"
		    "not just \"100\" for a 100MHz CPU, it might actually be 99.46.\n"
		    "Remember, AMTOR specifies +-20ppm (parts per million), \n"
		    "that is 0.000001) clock accuracy. \n"
		    "So you need to specify the CPU clock with kHz accuracy!\n",
		    cpumhz);
		    return;

		case 2:
		    errprintf(SEV_WARNING, 
			"RDTSC present, but does not change!\n");
		    break;
			
		default:
		    errprintf(SEV_INFO, 
			"Illegal instruction exception caught at trying RDTSC\n");
			break;
		}
	}
#endif /* __i386__ */
	for (i = 0; i < 10; i++) {
		if (gettimeofday(&t1, NULL))
			errstr(SEV_FATAL, "gettimeofday");
		if (gettimeofday(&t2, NULL))
			errstr(SEV_FATAL, "gettimeofday");
		if (((unsigned)(1000000+t2.tv_usec-t1.tv_usec) % 1000000) > 1)
			break;
	}
	if (i >= 10)
	    errprintf(SEV_FATAL, 
		"Your gettimeofday resolution seems to be inappropriate\n");
	
	errprintf(SEV_INFO, 
	    "\nRDTSC call is not working and/or you chose the option -R.\n" 
	    "So I use gettimeofday() as timing source, which has lowest accuracy.\n"
	    "Let's hope you entered a good gettimeofday correction with -t %10.9f.\n", 
	    tvuseccorr);
	return;
}

/* --------------------------------------------------------------------- */

void refclock_init(void)
{
	struct timeval tv;

	lasttime = 0;
#ifdef __i386__
	if (rdtsc_ok) {
		__asm__("rdtsc;" : "=d" (starttime_hi), "=a" (starttime_lo));
		return;
	}
#endif /* __i386__ */
	if (gettimeofday(&tv, NULL))
		errstr(SEV_FATAL, "gettimeofday");
	last_tvusec = tv.tv_usec;
	time_cnt = 0;
}

/* --------------------------------------------------------------------- */

l1_time_t refclock_current(l1_time_t expected, int exp_valid)
{
	struct timeval tv;
	l1_time_t curtime;
	long diff;
	static int bad_refclock_count = 0;

#ifdef __i386__
// tnx to Michael Matz (SuSE programming mailing list) 
// for asm repair because of gcc 3.2 compiler error !
	if (rdtsc_ok) {
		unsigned int tmp0 = 0, tmp1 = 0;
		unsigned int tmp2 = 0, tmp3 = 0;
		unsigned int ignore;
		__asm__("mull %2"
			: "=d" (tmp2), "=a" (ignore)
			: "m" (scale_rdtsc), "1" (tmp0));
		__asm__("mull %2" 
			: "=d" (ignore), "=a" (tmp3) 
			: "m" (scale_rdtsc), "1" (tmp1)); 
		curtime = tmp2 + tmp3;
		goto time_known;
	}
#endif /* __i386__ */
	if (gettimeofday(&tv, NULL))
		errstr(SEV_FATAL, "gettimeofday");
	time_cnt += (unsigned)(1000000 + tv.tv_usec - last_tvusec) % 1000000;
	last_tvusec = tv.tv_usec;
	curtime = (time_cnt * scale_tvusec) >> 24;
  time_known:
	if (exp_valid && abs(diff = (curtime - lasttime - expected)) >= 1000) {
	    errprintf(SEV_WARNING, "refclock adjustment %ld more than 1ms\n", diff);
	    bad_refclock_count++;
	    if ( bad_refclock_count > 10) {
		errprintf(SEV_INFO, 
		    "\nRDTSC call gives wrong results at runtime test.\n" 
		    "So I will do as if you had given the option -R to hfkernel.\n" 
		    "So I use gettimeofday() as timing source, which has lowest accuracy.\n"
		    "Let's hope you entered a good gettimeofday correction with -t %10.9f.\n", 
		    tv_corr);
		rdtsc_ok=0;
	    }
	}
	return (lasttime = curtime);
}

/* --------------------------------------------------------------------- */


