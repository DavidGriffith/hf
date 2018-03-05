/*****************************************************************************/

/*
 *      main.c  --  Parameter parsing and thread starting.
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

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "os_linux.h"
#include "l1.h"
#include "standby.h"

#ifdef DISABLE_REALTIME
#undef HAVE_SCHED_H
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

/* --------------------------------------------------------------------- */

pthread_t thr_l2;

static int logging = 0;

const char *name_comm = "hfapp";
static const char *name_audio = NULL;
static const char *name_ptt = NULL;
static const char *name_mixer = NULL;

unsigned int comm_perm = S_IRWXU|S_IRWXG|S_IRWXO;

/* --------------------------------------------------------------------- */
/*
 * Logging functions
 */

void errprintf(int severity, const char *fmt, ...)
{
        va_list args;

        va_start(args, fmt);
        if (logging) {
                char tmp[512];
                vsnprintf(tmp, sizeof(tmp), fmt, args);
                syslog(severity, tmp);
        } else {
                fprintf(stderr, "hfkernel[%u]: ", getpid());
                vfprintf(stderr, fmt, args);
        }
        va_end(args);
        if (severity <= SEV_FATAL)
                exit(1);
}

/* --------------------------------------------------------------------- */

void errstr(int severity, const char *st)
{
        errprintf(severity, "error: %s: %s\n", st, strerror(errno));
}

/* --------------------------------------------------------------------- */

static void start_threads(void)
{
#ifdef HAVE_SCHED_H
	struct sched_param schp;
#endif
	pthread_attr_t thr_attr;
	pthread_t thr_io;
     
	signal(SIGPIPE, SIG_IGN);
	if (pthread_attr_init(&thr_attr))
		errstr(SEV_FATAL, "pthread_attr_init");
#ifdef HAVE_SCHED_H
	memset(&schp, 0, sizeof(schp));
	if (pthread_attr_setschedpolicy(&thr_attr, SCHED_RR))
		errstr(SEV_WARNING, "pthread_attr_setschedpolicy");
	schp.sched_priority = sched_get_priority_min(SCHED_RR)+1;
	if (pthread_attr_setschedparam(&thr_attr, &schp))
		errstr(SEV_WARNING, "pthread_attr_setschedparam");
#endif
	if (pthread_create(&thr_l2, &thr_attr, mode_standby, NULL))
		errstr(SEV_FATAL, "pthread_create");
#ifdef HAVE_SCHED_H
	schp.sched_priority = sched_get_priority_min(SCHED_RR);
	if (pthread_attr_setschedparam(&thr_attr, &schp))
		errstr(SEV_WARNING, "pthread_attr_setschedparam");
#endif
	if (pthread_create(&thr_io, &thr_attr, io_process, NULL))
		errstr(SEV_FATAL, "pthread_create");
	if (pthread_attr_destroy(&thr_attr))
		errstr(SEV_WARNING, "pthread_attr_destroy");
	if (pthread_join(thr_io, NULL))
		errstr(SEV_WARNING, "pthread_join");
	if (munlockall())
		errstr(SEV_WARNING, "mlockall");
}

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
        int c;
        int err = 0;
	float cpumhz = 99.46;
	float sndcorr = 1.000;
	float tvcorr = 1.000;
	int dis_rdtsc = 0;
	int invptt = 0;

        while ((c = getopt(argc, argv, "a:M:c:lip:m:t:s:r:Rf23")) != -1) 
                switch (c) {
		case 'a':
			name_audio = optarg;
			break;

		case 'M':
			name_mixer = optarg;
			break;

		case 'c':
			name_comm = optarg;
			break;

		case 'p':
			name_ptt = optarg;
			break;

                case 'i':
                        invptt = !invptt;
                        break;

                case 'l':
                        logging = 1;
                        break;

                case 'm':
			cpumhz = strtod(optarg, NULL);
			if (cpumhz < 10 || cpumhz > 1000) {
				fprintf(stderr, "CPU clock out of range 10MHz..1000MHz\n");
				err++;
			}
			break;

                case 's':
			sndcorr = strtod(optarg, NULL);
			if (sndcorr < 0.9 || sndcorr > 1.1) {
				fprintf(stderr, "Soundcard clock adjust factor outside of range 0.9 .. 1.1\n");
				err++;
			}
			break;

                case 't':
			tvcorr = strtod(optarg, NULL);
			if (tvcorr < 0.9 || tvcorr > 1.1) {
				fprintf(stderr, "gettimeofday clock adjust factor outside of range 0.9 .. 1.1\n");
				err++;
			}
			break;
			
		case 'r':
			comm_perm = strtoul(optarg, NULL, 0);
			break;

		case 'R':
			dis_rdtsc = 1;
			break;

		case 'f':
			standby_flags &= ~STANDBY_FLAG_FREQ_ESTIMATION;
			break;

		case '2':
			standby_flags &= ~STANDBY_FLAG_MONITOR_200;
			break;

		case '3':
			standby_flags &= ~STANDBY_FLAG_MONITOR_300;
			break;

		default:
                        err++;
                        break;
                }
        if (err) {
                fprintf(stderr, "usage: hfkernel [-a <audio device>] [-M <mixer device>] [-c <comm socket>] [-l]\n"
			"                [-r <socket perms>] [-i] [-p <ptt comm port>] [-m <cpu clock MHz>] [-R]\n"
			"                [-t <gettimeofday correction>] [-s <soundcard clock correction>]"
			"                [-f] [-2] [-3]\n\n"
			"  -a: audio device path (default: /dev/dsp)\n"
			"  -M: mixer device path (default: none)\n"
			"  -c: path of the communication socket (default: hfapp)\n"
			"  -r: access permissions of the communication socket (default: 0777 = rwxrwxrwx)\n"
			"  -p: path of the serial port to output PTT (default: none)\n"
			"  -m: CPU clock in MHz (exact at the kHz level)\n"
			"  -t: gettimeofday correction factor\n"
			"  -s: soundcard sampling rate correction\n"
			"  -R: disable the use of the rdtsc instruction (Intel systems only)\n"
			"  -f: standby: disable frequency estimation\n"
			"  -2: standby: disable monitoring of 200baud signals\n"
			"  -3: standby: disable monitoring of 300baud signals\n");
                exit(1);
        }
        if (logging)
                openlog("hfkernel", LOG_PID, LOG_DAEMON);
	errprintf(SEV_INFO, "hfkernel v0.2 starting...\n");
#ifndef DISABLE_REALTIME
	if (mlockall(MCL_CURRENT))
		errstr(SEV_WARNING, "mlockall");
#endif
	l1_init(name_audio, name_ptt, name_mixer, sndcorr, tvcorr, cpumhz, dis_rdtsc, invptt);
 	errno = EAGAIN;
	start_threads();
	exit(0);
}

/* --------------------------------------------------------------------- */
