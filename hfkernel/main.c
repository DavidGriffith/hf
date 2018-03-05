/*****************************************************************************/

/*
 *      main.c  --  hfkernel main file, parameter parsing and thread 
 *	starting, basic sound functions for hfhernel Linux sound I/O.
 *
 *      Copyright (C) 1996  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *      Swiss Federal Institute of Technology (ETH), Electronics Lab
 *	modified by Gnther Montag
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
#include <syslog.h>
#include <sys/types.h>
#ifdef __linux__
#include <asm/byteorder.h>
#endif
#include <sys/soundcard.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#ifdef __linux__
#include <endian.h>
#endif
#ifdef __FreeBSD__ 
#include <sys/endian.h>
#endif
#include <assert.h>

#ifdef DISABLE_REALTIME
#undef HAVE_SCHED_H
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif
 
#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"
#include "standby.h"
#include "oss.h"
#include "mt63hflink.h"
#include "cw.h"
#ifdef HAVE_ALSA_ASOUNDLIB_H
#include "alsa.h"
#endif /* HAVE_ALSA_ASOUNDLIB_H */

/* --------------------------------------------------------------------- */

/* these variables take hfkernel's options */
static int logging = 0;
const char *name_audio ="/dev/dsp";
const char *name_audio_OSS ="/dev/dsp";
const char *name_ptt = NULL;
const char *name_mixer = NULL;
float snd_corr = 1, tv_corr = 1, cpu_mhz = 0;
int dis_rdtsc = 0, invert_ptt = 0;
const char *name_comm = "/var/run/hfapp";
int OSS = 0, ALSA = 0, fdx = 0;
int force_halfduplex = 0;
int nommap = 0;
int fsk_l1_loop_running = 0;
unsigned int standby_flags = 
  STANDBY_FLAG_FREQ_ESTIMATION|STANDBY_FLAG_MONITOR_200|STANDBY_FLAG_MONITOR_300;
unsigned int comm_perm = S_IRWXU|S_IRWXG|S_IRWXO;
extern int brake, elbug_rts;
pthread_t thr_l2;
int fd_audio = -1, fd_mixer = -1, fd_ptt = -1;
pthread_t thr_l1;
int samples_remain = 0;
int samples_count = 0;
short *samples_ptr = NULL;

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

void output_ptt(int ptt)
{
	int par = TIOCM_RTS;

	if (elbug_rts)	return;
	if (fd_ptt < 0)	return;

	if (ioctl(fd_ptt, 
	    ((ptt && !invert_ptt) || (!ptt && invert_ptt)) ? 
	    TIOCMBIS : TIOCMBIC, &par)) {
	    errstr(SEV_WARNING, "ioctl: TIOCMBI[CS]");
	    errstr(SEV_WARNING, "serial port for ptt can not be opened!");
	    errstr(SEV_FATAL, 
		"maybe you have to modify serial port in /etc/hf.conf !");
	}
}


/* --------------------------------------------------------------------- */

/* to shut down l1 fsk soundcard loop  */
void l1_stop()
{
	if (! fsk_l1_loop_running) return;	

	if (pthread_cancel(thr_l1))
		errstr(SEV_WARNING, "pthread_cancel l1 thread");
	if (pthread_join(thr_l1, NULL))
		errstr(SEV_WARNING, "pthread_join l1 thread");
	fsk_l1_loop_running = 0;	
	printf ("l1 fsk soundcard loop is finished.\n");
	bufprintf(HFAPP_MSG_DATA_STATUS, "l1 fsk soundcard loop is finished.");
//	sleep(1);
	return;
}


/* --------------------------------------------------------------------- */

/* thought i need it for mt63, but no need now, maybe later */
void l1_switch_to_nommap()
{
#ifndef NOREALTIME
	static struct sched_param schp;
#endif /* NOREALTIME */
	pthread_attr_t thr_attr;

	if (pthread_cancel(thr_l1))
		errstr(SEV_FATAL, "pthread_cancel");
	if (pthread_join(thr_l1, NULL))
		errstr(SEV_FATAL, "pthread_join");
	if (pthread_attr_init(&thr_attr))
		errstr(SEV_FATAL, "pthread_attr_init");
	//printf("l1_swtommap: fd_audio is %d\n", fd_audio);
#ifndef NOREALTIME
 	memset(&schp, 0, sizeof(schp));
	if (pthread_attr_setschedpolicy(&thr_attr, SCHED_RR))
		errstr(SEV_FATAL, "pthread_attr_setschedpolicy");
	schp.sched_priority = sched_get_priority_min(SCHED_RR)+2;
	if (pthread_attr_setschedparam(&thr_attr, &schp))
		errstr(SEV_FATAL, "pthread_attr_setschedparam");
#endif /* NOREALTIME */
	if (pthread_create(&thr_l1, &thr_attr, 
		oss_nommap_fdx_driver, (char *)name_audio))
	    errstr(SEV_FATAL, "pthread_create nommap-driver");
	if (pthread_attr_destroy(&thr_attr))
		errstr(SEV_WARNING, "pthread_attr_destroy");
	printf("switched to nommap sound driver.\n");
	//bufprintf(HFAPP_MSG_DATA_STATUS, "switched to nommap sound driver");
	return;
}

/* --------------------------------------------------------------------- */

/* thought i need it for mt63, but no need now, maybe later */
void l1_switch_to_mmap()
{
#ifndef NOREALTIME
	static struct sched_param schp;
#endif /* NOREALTIME */
	pthread_attr_t thr_attr;

	//printf("switch back mmap sound driver: before cancel\n");
	if (pthread_cancel(thr_l1))
		errstr(SEV_FATAL, "pthread_cancel");
	//printf("switch back mmap sound driver: before join\n");
	if (pthread_join(thr_l1, NULL))
		errstr(SEV_FATAL, "pthread_join");
	//printf("switch back mmap sound driver: before init\n");
	if (pthread_attr_init(&thr_attr))
		errstr(SEV_FATAL, "pthread_attr_init");
	//printf("l1_switch_to_mmap: fd_audio is %d\n", fd_audio);
#ifndef NOREALTIME
 	memset(&schp, 0, sizeof(schp));
	if (pthread_attr_setschedpolicy(&thr_attr, SCHED_RR))
		errstr(SEV_FATAL, "pthread_attr_setschedpolicy");
	schp.sched_priority = sched_get_priority_min(SCHED_RR)+2;
	if (pthread_attr_setschedparam(&thr_attr, &schp))
		errstr(SEV_FATAL, "pthread_attr_setschedparam");
#endif /* NOREALTIME */
	//printf("switch back mmap sound driver: before create\n");
	if (pthread_create(&thr_l1, &thr_attr, 
		oss_fdx_driver, (char *)name_audio))
	    errstr(SEV_FATAL, "pthread_create mmap-driver");
	if (pthread_attr_destroy(&thr_attr))
		errstr(SEV_WARNING, "pthread_attr_destroy");
	printf("switched back to Tom's good old mmap driver.\n");
	//bufprintf(HFAPP_MSG_DATA_STATUS, 
	//    "switched back to Tom's good old mmap driver");
	return;
}

/* --------------------------------------------------------------------- */

void l1_init()
{
	
//	test & start low-level soundcard loop, for fsk modes only
	static struct sched_param schp;
	pthread_attr_t thr_attr;
	
	if (fsk_l1_loop_running) return;
	
	//fsk_l1_loop_running = 1;	
	// again for sure
	//if (close(fd_audio)) errstr(SEV_FATAL, "l1_init: audio device could not be closed.");

	l1_fsk_modem_init();
	if (OSS) {
	    errprintf(SEV_INFO, "preparing mixer ...\n");
	    oss_prepare_mixer(name_mixer);
	}
	/*
	 * start the l1 thread
	 */
	memset(&schp, 0, sizeof(schp));
	if (pthread_attr_init(&thr_attr))
		errstr(SEV_FATAL, "pthread_attr_init");
	if (pthread_attr_setschedpolicy(&thr_attr, SCHED_RR))
		errstr(SEV_FATAL, "pthread_attr_setschedpolicy");
	schp.sched_priority = sched_get_priority_min(SCHED_RR)+2;
	if (pthread_attr_setschedparam(&thr_attr, &schp))
		errstr(SEV_FATAL, "pthread_attr_setschedparam");

	if (OSS) {
	    if (fdx) {
		errprintf(SEV_INFO, 
		    "creating thread for oss full duplex driver ...\n");
	    	if (nommap) {
		    if (pthread_create(&thr_l1, &thr_attr, 
			oss_nommap_fdx_driver, (char *)name_audio))
			errstr(SEV_FATAL, "pthread_create");
		    if (pthread_attr_destroy(&thr_attr))
			errstr(SEV_WARNING, "pthread_attr_destroy");
		    fsk_l1_loop_running = 1;	
		    return;
		}  else {
		    if (pthread_create(&thr_l1, &thr_attr, 
			oss_fdx_driver, (char *)name_audio))
			errstr(SEV_FATAL, "pthread_create");
		    if (pthread_attr_destroy(&thr_attr))
			errstr(SEV_WARNING, "pthread_attr_destroy");
		    fsk_l1_loop_running = 1;	
		    return;
		}
	    }
	}

#ifdef HAVE_ALSA_ASOUNDLIB_H
	if (ALSA) {
	    errprintf(SEV_INFO, "creating thread for ALSA driver ...\n");
	    if (pthread_create(&thr_l1, &thr_attr, alsa_fdx_driver, 
			(char *)name_audio))
		errstr(SEV_FATAL, "pthread_create");
	    if (pthread_attr_destroy(&thr_attr))
		errstr(SEV_WARNING, "pthread_attr_destroy");
	    fsk_l1_loop_running = 1;	
	    return;
	}
#endif /* HAVE_ALSA_ASOUNDLIB_H */
	
	refclock_probe(tv_corr, cpu_mhz, dis_rdtsc);
	
	if (OSS) {
	    errprintf(SEV_INFO, 
		"creating thread for oss half duplex driver ...\n");
	    if (nommap) {
		if (pthread_create(&thr_l1, &thr_attr, oss_nommap_hdx_driver, 
		    (char *)name_audio))
	    	    errstr(SEV_FATAL, "pthread_create");	
		if (pthread_attr_destroy(&thr_attr))
		    errstr(SEV_WARNING, "pthread_attr_destroy");
		fsk_l1_loop_running = 1;	
		return;
	    } else {
		if (pthread_create(&thr_l1, &thr_attr, oss_hdx_driver, 
			(char *)name_audio))
	    	    errstr(SEV_FATAL, "pthread_create");	
		if (pthread_attr_destroy(&thr_attr))
		    errstr(SEV_WARNING, "pthread_attr_destroy");
		fsk_l1_loop_running = 1;	
		return;
	    }
	}
}

/* --------------------------------------------------------------------- */

void l1_input_samples(l1_time_t tstart, l1_time_t tinc, 
		      short *samples, unsigned int nsamples)
{
	if (modefamily == MT63) {
	    //l1_mt63_input_samples(samples, nsamples);
	    errprintf(SEV_WARNING, "l1_input_samples should not have been called.\n");
	    return;
	}
	else if (modefamily == FSK) {
	    l1_fsk_input_samples(tstart, tinc, samples, nsamples);
	    return;
	}
	else	
	errprintf(SEV_WARNING, "hfkernel: no rx mode-family specified.\n");
        return;
	//exit (1);
}

/* --------------------------------------------------------------------- */

int l1_output_samples(l1_time_t tstart, l1_time_t tinc, 
		      short *samples, unsigned int nsamples)
{
	int retval; 
	if (modefamily == FSK) {
	    retval = l1_fsk_output_samples(tstart, tinc, samples, nsamples);
	    return retval;
	}
	else if (modefamily == MT63) 
	{
	    //retval = l1_mt63_output_samples(tstart, tinc, samples, nsamples);
	    errprintf(SEV_WARNING, "l1_output_samples should not have been called.\n");
	    return retval;
	}
	else 
	errprintf(SEV_WARNING, "hfkernel: no tx mode-family specified.\n");
	return 0;
	//exit (1);
}

/* --------------------------------------------------------------------- */

void l1_start_sample(short *data, unsigned int len)
{
	if (!data || !len)
		return;
	samples_ptr = data;
	samples_count = len;
	samples_remain = len;
}

/* --------------------------------------------------------------------- */

int l1_sample_finished(void)
{
	if (samples_remain == 0 && samples_count > 0) {
		samples_count = 0;
		return 1;
	}
	return 0;
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

/* here a primitive test for implementing new modes... 
 * for this hfkernel needs to be started only in console mode 
 * We must declare the modefamily first: MT63 od CW od FSK.
 * rtty, standby, amtor, pactor, gtor are the fsk modes.
 * If you test a fsk, l1_init() first.
 * 
 */
	
//	modefamily = MT63;
	modefamily = FSK;
//	modefamily = 0;
	l1_init();
//	if (pthread_create(&thr_l2, &thr_attr, mode_cw_tx, NULL))
	if (pthread_create(&thr_l2, &thr_attr, mode_standby, NULL))
//	if (pthread_create(&thr_l2, &thr_attr, mode_mt63_tx, NULL))
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

static void start_io_thread(void)
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
	float pre_cpumhz = 0;
	float pre_snd_corr = 1.000;
	float pre_tvcorr = 1.000;

        while ((c = getopt(argc, argv, "a:M:c:klhip:m:nt:s:r:Rf23")) != -1) 
            switch (c) {
		case 'a':
		    name_audio = optarg;
		    break;

		case 'M':
		    name_mixer = optarg;
		    break;

		case 'c':
		    if (optarg != NULL) name_comm = optarg;
		    break;

		case 'p':
		    name_ptt = optarg;
		    break;

                case 'h':
		    force_halfduplex = 1;
            	    break;

                case 'i':
            	    invert_ptt = 1;
                    break;

                case 'k':
                    system ("killall hfkernel");

                case 'l':
                    logging = 1;
                    break;

                case 'm':
		    pre_cpumhz = strtod(optarg, NULL);
		    
/*
		    if (pre_cpumhz < 10 || pre_cpumhz > 16000) {
			fprintf(stderr, 
			    "CPU clock out of range 10MHz..16000MHz\n");
			err++;
		    }
*/
		    cpu_mhz = pre_cpumhz; 

		    break;

                case 'n':
		    nommap = 1;
		    break;

                case 's':
		    pre_snd_corr = strtod(optarg, NULL);
		    if (pre_snd_corr < 0.9 || pre_snd_corr > 1.1) {
		        fprintf (stderr, 
			    "Soundcard clock adjust factor outside "
			    "of range 0.9 .. 1.1\n");
			err++;
		    }
		    snd_corr = pre_snd_corr; 
//		    errprintf(SEV_INFO, 
//			"sound clock correction:  %f\n", pre_snd_corr);
		    break;

                case 't':
		    pre_tvcorr = strtod(optarg, NULL);
		    if (pre_tvcorr < 0.9 || pre_tvcorr > 1.1) {
		        fprintf (stderr, 
			    "gettimeofday clock adjust factor %f"
			    "outside of range 0.9 .. 1.1\n", pre_tvcorr);
			err++;
		    }
//		    errprintf(SEV_INFO, 
//			"gettimeofday correction: %f\n", tvcorr);
		    tv_corr = pre_tvcorr;
		    break;
			
		case 'r':
		    comm_perm = strtoul(optarg, NULL, 0);
		    break;

		case 'R':
		    dis_rdtsc = 1;
		    break;

		case 'f':
		    standby_flags &= ~STANDBY_FLAG_FREQ_ESTIMATION;
		    errprintf(SEV_INFO, 
			"standby: will not use frequency estimation.\n");
		    break;

		case '2':
		    standby_flags &= ~STANDBY_FLAG_MONITOR_200;
		    errprintf(SEV_INFO, 
			"standby: will not monitor 200 baud.\n");
		    break;

		case '3':
		    standby_flags &= ~STANDBY_FLAG_MONITOR_300;
		    errprintf(SEV_INFO, 
			"standby: will not monitor 300 baud.\n");
		    break;

		default:
                        err++;
                        break;
                }
        if (err) {
                fprintf(stderr, "usage: hfkernel [-2] [-3] [-a <audio device>] [-c <comm socket>] [-f] [-h]\n"
			"                [-i] [-k] [-l] [-M <mixer device>] [-m <cpu clock MHz>] \n"
			"		[-n] [-p <ptt comm port>] [-R] [-r <socket perms] \n"
			"                [-s <soundcard clock correction>] [-t <gettimeofday correction>]\n\n"
			"  -2: standby: disable monitoring of 200baud signals\n"
			"  -3: standby: disable monitoring of 300baud signals\n"
			"  -a: audio device path (default for OSS: /dev/dsp)\n"
			"      (for ALSA driver use -a plughw:0,0)\n"
			"  -c: path of the communication socket (default: /var/run/hfapp)\n"
			"  -f: standby: disable frequency estimation\n"			
			"  -h: force half duplex mode (for OSS only!)\n"			
			"  -i: invert PTT (default: PTT = positive signal)\n"
			"  -k: stop hfkernel (this is also used by the start script hf)\n"
			"  -l: logging (default: off)\n"			
			"  -M: mixer device path (default: none)\n"
			"  -m: CPU clock in MHz (exactly at kHz level)\n"
			"  -n: no mmap() (which some cards/drivers can't) (for OSS only!)\n"			
			"  -p: path of the serial port to output PTT (default: none)\n"
			"  -R: disable the use of the rdtsc instruction (Intel systems only)\n"
			"  -r: access permissions of the communication socket (default: 0777 = rwxrwxrwx)\n"
			"  -s: soundcard sampling rate correction\n"
			"  -t: gettimeofday correction factor\n\n");

                exit(1);
        }
        if (logging)
                openlog("hfkernel", LOG_PID, LOG_DAEMON);
	printf("hfkernel %s starting...\n", PACKAGE_VERSION);
#ifndef DISABLE_REALTIME
	if (mlockall(MCL_CURRENT))
		errstr(SEV_WARNING, "mlockall");
#endif
	errno = EAGAIN;

	if (!name_audio) {
	    name_audio = "/dev/dsp";
	    printf("hfkernel: I will try OSS driver for %s\n", 
		name_audio);
	    OSS = 1;
	} else if (strstr (name_audio, "dev/")) {
	    printf("hfkernel: I will try OSS driver for %s\n", 
		name_audio);
	    OSS = 1;	
	} else if (strstr (name_audio, "hw:")) {
#ifndef HAVE_ALSA_ASOUNDLIB_H
	    errprintf(SEV_FATAL, "hfkernel: Soundcard name contains 'hw:...', "
		"which is ALSA convention, but I am not compiled with ALSA driver here.\n");
	    exit (1);
#endif /* HAVE_ALSA_ASOUNDLIB_H */ 
	    ALSA = 1;	
	    if (strstr (name_audio, "hw:0")) 
		name_audio_OSS="/dev/dsp0";
	    if (strstr (name_audio, "hw:1")) 
		name_audio_OSS="/dev/dsp1";
	    if (strstr (name_audio, "hw:2")) 
		name_audio_OSS="/dev/dsp2";
	    if (strstr (name_audio, "hw:3")) 
		name_audio_OSS="/dev/dsp3";
            printf("For FSK modes, I will try ALSA code for device %s\n"
		"but for MT63, I will use OSS code for device %s\n", name_audio, name_audio_OSS);
	} else {
	    errprintf(SEV_FATAL, 
		"Soundcard name does not contain 'dev/...' (OSS) and not 'hw:...'(ALSA) -> \n"
		"please check soundcard option, because I want to know what driver to use....\n");
	    exit (1);
 	}

	if (OSS) {
	    fdx = probe_oss(name_audio);
	}

	if (name_ptt) {
	    if ((fd_ptt = open(name_ptt, O_RDWR, 0)) < 0)
		errstr(SEV_WARNING, "error at opening ptt device\n");
	    else {
	        errprintf(SEV_INFO, "opened ptt device with fd %d\n", fd_ptt);
		output_ptt(0);
	    } 
	 } else 
	    fd_ptt = -1;
	
	/* this is for testing new modes only.#
	 * From 0.7.4. on the mode-threads are controlled from hfterm 
	 */
	
//	start_threads();

	printf("Note: hfkernel is only part of the hf package.\n"); 
	printf("It is controlled by the graphic terminal hfterm. To start them both, use the start script hf. In newer linuxes (kernel 2.6...) we need the syntax\n ´LD_ASDSUME_KERNEL=2.2.5 hfterm´, this is already prepared in the hf script. \n");
	start_io_thread();
	exit(0); }

