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
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "dcf77.h"

char gmt[32];

/* --------------------------------------------------------------------- */

void timequery()
{
	time_t now;
	time(&now);
	//would also be possible like ...
	//sprintf(gmt, "%s", asctime(gmtime(&now)));	
	// strftime(gmt, 31, "%a %d.%m.%y, %H:%M", gmtime(&now));
	strftime(gmt, 31, "%Y-%m-%d %H%M", gmtime(&now));
} 

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

float soundcorr, mhzcorr, timecorr;
struct tm *tm;
unsigned int timediff;

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
	tc.tm2inc = (1000000 + (sample_rate / 2)) / sample_rate;
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
//	printf("dcf77 time decoding program");
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
			vlprintf(1, "First CPU clock estimate: %9dHz", 
				 tc.tm3inc * tc.sr);
	}
}

/* --------------------------------------------------------------------- */

void output()
{
        static char head[256],  foot[256];
	char factors[512];
#ifdef __linux__
	const char *configfilename =  "/etc/calibrations";
#endif
#ifdef __FreeBSD__
	const char *configfilename =  "/usr/local/etc/calibrations";
#endif
	FILE *configfile = NULL;
	static int prepared = 0;

/*
 *  output of the 3 correction factors to stdout
 */
	printf(	"Corrections: current time %02d:%02d:%02d, time diff %us\n" // was: ld
	    "\t***  soundcorr:   (hfkernel option -s):\t%10.8f  ***\n"
	    "\t***  mhzcorr:     (hfkernel option -m):\t%10.6f  ***\n"
	    "\t***  timecorr:    (hfkernel option -t):\t%10.8f  ***\n",
	    tm->tm_hour, tm->tm_min, tm->tm_sec, timediff,
	    soundcorr, mhzcorr, timecorr);

/*
 *  preparation of head, foot, and opening config file
 */
	configfile = fopen(configfilename, "a");
	if(configfile == NULL) {
	    fprintf(stderr, "sorry, configuration file %s can not be opened\n"
	        "for appending correction factors\n", configfilename);
	    return;
	} 
	vlprintf(2, "configuration file %s opened\n", configfilename);

	if (! prepared) {
    	    timequery();
	    sprintf(head, 
"##############################################################################\n"
	    "## * * Correction factors by dcf77rx at %s  * *\n"
	    "## * * Edit if not credible. Last values will be valid.  * *\n",
	    gmt);
//	    printf( "head string prepared.\n"); 

	    sprintf(foot, 
	    "## * * * * End of one cycle of dcf77rx calibration.  * * * *\n");
	    vlprintf(2, "foot string prepared.\n"); 

	    if ( (fwrite(head, strlen(head), 1, configfile)) != 1) {
    		fprintf (stderr, 
		    "Error in writing head lines to config file %s\n", 
		    configfilename);
        	return;
	    }
	    vlprintf(2, "head string written.\n"); 	    
	    prepared = 1;
	}

/*	
 *  output of the 3 correction factors to the config file
 */
	    
	sprintf(factors, "## dcf77rx calibration at time %02d:%02d:%02d:\n" 
	    "soundcorr=%10.8f\n"
	    "mhzcorr=%10.6f\n"
	    "timecorr=%10.8f\n",
	    tm->tm_hour, tm->tm_min, tm->tm_sec, 
	    soundcorr, mhzcorr, timecorr);
	    vlprintf(2, "factors string prepared\n"); 
	    
	if ( (fwrite(factors, strlen(factors), 1, configfile)) != 1) {
    	    fprintf (stderr, 
		"Error in writing correction factors to config file %s\n", 
		configfilename);
    	    return;
	}
	vlprintf(2, "correction factors written to config file.\n"); 	

	if ( (fwrite(foot, strlen(foot), 1, configfile)) != 1) {
    	    fprintf (stderr, 
		"Error in writing foot lines to config file %s\n", 
		configfilename);
    	    return;
	}
	vlprintf(2, "foot string written.\n"); 
	fclose(configfile);
	vlprintf(2, "configuration file closed.\n"); 
	return;
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



	vlprintf(1, "tc_minute: curtime: %u\n", curtime);

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
/*
 *  calculation of the 3 correction factors
 */
	soundcorr = (float)tc.sr/((float)(tm1c - tc.tm1start)/(float)timediff);
	mhzcorr = (float)(tm3c - tc.tm3start)/(float)timediff / 1000000;
	timecorr = 2.0 * 1000000.0/((float)(tm2c - tc.tm2start)/(float)timediff),

	output();

/* old fragments by Tom */
 //(int)((1<<24)*1000000.0/((float)(tm2c - tc.tm2start)/(float)timediff)), 
 //(int)((float)(1ULL<<32)*1000000.0/((float)(tm3c - tc.tm3start)/(float)timediff)));

}

/* --------------------------------------------------------------------- */
