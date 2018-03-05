/*****************************************************************************/

/*
 *      link.c  --  Connects two "l1links".
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
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include "os.h"
#include "l1.h"
#include "../user/l1user.h"

#ifdef DISABLE_REALTIME
#undef HAVE_SCHED_H
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

/* --------------------------------------------------------------------- */
/*
 * Sample output
 */
static int samples_remain = 0;
static int samples_count = 0;
static short *samples_ptr = NULL;

static pthread_t thr_l1;

/* --------------------------------------------------------------------- */

#define BUFSZ 64
#define EXCESSSZ 384
#define INC_SAMPLE (1000000/SAMPLE_RATE)

static void *link_driver(void *name)
{
        int fd_link, i, j;
	fd_set rmask, wmask;
	l1_time_t itime = 0, otime = 0;
	char ibuf[EXCESSSZ+BUFSZ];
	char obuf[BUFSZ];
	unsigned int ibuf_wr = 0;
	unsigned int obuf_rd = 0, obuf_wr = 0;
	struct sockaddr_un saddr;

	if ((fd_link = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		errstr(SEV_FATAL, "socket");
	saddr.sun_family = AF_UNIX;
	strncpy(saddr.sun_path, name, sizeof(saddr.sun_path));
	if (connect(fd_link, (struct sockaddr *)&saddr, sizeof(saddr)))
		errstr(SEV_FATAL, "connect");
	for (;;) {
#if 0
		printf("ibuf: rd: %2u  wr: %2u  tm: %lu   obuf: rd: %2u  wr: %2u  tm: %lu\n",
		       ibuf_rd, ibuf_wr, itime, obuf_rd, obuf_wr, otime);
#endif
		/* generate output data */
		if (obuf_rd >= obuf_wr && (signed)(otime - itime) < 1000) {
			obuf_rd = 0;
			obuf_wr = BUFSZ;
			if (!l1_output_samples(otime, INC_SAMPLE, (short *)obuf, BUFSZ/2))
				memset(obuf, 0, BUFSZ);
			otime += INC_SAMPLE * BUFSZ/2;
		}
		/* wait output/input ready */
		FD_ZERO(&rmask);
		FD_ZERO(&wmask);
		FD_SET(fd_link, &rmask);
		if (obuf_rd < obuf_wr)
			FD_SET(fd_link, &wmask);
		i = select(fd_link+1, &rmask, &wmask, NULL, NULL);
		if (i < 0)
			errstr(SEV_FATAL, "select");
		if (FD_ISSET(fd_link, &rmask)) {
			i = read(fd_link, ibuf+EXCESSSZ+ibuf_wr, BUFSZ-ibuf_wr);
			if (i < 0) {
				if (errno != EAGAIN)
					errstr(SEV_FATAL, "read");
			} else {
				ibuf_wr += i;
				if (ibuf_wr >= BUFSZ) {
				/* process input data */
					i = BUFSZ / 2;
					if (i > 0) {
						if (samples_remain > 0) {
							if ((j = i) > samples_remain)
								j = samples_remain;
							memcpy(samples_ptr, ibuf+EXCESSSZ, 2*j);
							samples_ptr += j;
							samples_remain -= j;
						}
						l1_input_samples(itime, INC_SAMPLE, (short *)(ibuf+EXCESSSZ), i);
						itime += INC_SAMPLE * i;
					}
					ibuf_wr = 0;
					memmove(ibuf, ibuf+BUFSZ, EXCESSSZ);
				}
			}
		}
		if (FD_ISSET(fd_link, &wmask)) {
			i = write(fd_link, obuf+obuf_rd, obuf_wr - obuf_rd);
			if (i < 0) {
				if (errno != EAGAIN)
					errstr(SEV_FATAL, "write");
			} else 
				obuf_rd += i;
		}
	}
}

/* --------------------------------------------------------------------- */

void l1_init(const char *name_audio, const char *name_ptt, const char *name_mixer, 
	     float sndcorr, float tvuseccorr, float cpumhz, int dis_rdtsc, int invptt)
{
#if defined(HAVE_SCHED_H) && !defined(DISABLE_REALTIME)
	static struct sched_param schp;
#endif
	pthread_attr_t thr_attr;

	if (!name_audio)
		name_audio = "hfchannel";
	l1_modem_init();
	/*
	 * start the l1 thread
	 */
	if (pthread_attr_init(&thr_attr))
		errstr(SEV_FATAL, "pthread_attr_init");
#if defined(HAVE_SCHED_H) && !defined(DISABLE_REALTIME)
	memset(&schp, 0, sizeof(schp));
	if (pthread_attr_setschedpolicy(&thr_attr, SCHED_RR))
		errstr(SEV_WARNING, "pthread_attr_setschedpolicy");
	schp.sched_priority = sched_get_priority_min(SCHED_RR)+2;
	if (pthread_attr_setschedparam(&thr_attr, &schp))
		errstr(SEV_WARNING, "pthread_attr_setschedparam");
#endif
	if (pthread_create(&thr_l1, &thr_attr, link_driver, (void *)name_audio))
		errstr(SEV_FATAL, "pthread_create");
	if (pthread_attr_destroy(&thr_attr))
		errstr(SEV_WARNING, "pthread_attr_destroy");
}

/* --------------------------------------------------------------------- */

void l1_set_mixer(unsigned int src, int igain, int ogain)
{
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
