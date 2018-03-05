/*****************************************************************************/

/*
 *      main.c  --  HF modem kernel driver access.
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
 */

/*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>

#include "l1.h"
#include "hfmodem.h"

/* --------------------------------------------------------------------- */

#ifndef FREQ_SPACE
#define FREQ_SPACE 1275
#endif

#ifndef FREQ_MARK
#define FREQ_MARK  1475
#endif

/* --------------------------------------------------------------------- */

static int fd_krnl = -1;

static unsigned int freq_space = FREQ_SPACE, freq_mark = FREQ_MARK;

/* --------------------------------------------------------------------- */

void l1_init(const char *name_audio, const char *name_ptt, 
	     const char *name_mixer, float sndcorr, float tvuseccorr, 
	     float cpumhz, int dis_rdtsc, int invert_ptt)
{
	if (!name_audio)
		name_audio = "/dev/hfmodem";
	if ((fd_krnl = open(name_audio, O_RDWR)) < 0)
		errstr(SEV_FATAL, "open");
	l1_set_fsk_params(FREQ_SPACE, FREQ_MARK);
}

/* --------------------------------------------------------------------- */

void l1_set_fsk_params(unsigned int freqspace, unsigned int freqmark)
{
	errprintf(SEV_INFO, "FSK Parameters: Space frequency: %d  Mark frequency: %d\n",
		  freqspace, freqmark);
	freq_space = freqspace;
	freq_mark = freqmark;
}

/* --------------------------------------------------------------------- */

l1_time_t l1_get_current_time(void)
{
	hfmodem_time_t tm;

	if (ioctl(fd_krnl, HFMODEM_IOCTL_GETCURTIME, &tm))
		errstr(SEV_FATAL, "ioctl: HFMODEM_IOCTL_GETCURTIME");
	return tm;
}

/* --------------------------------------------------------------------- */

void l1_clear_requests(void)
{
	if (ioctl(fd_krnl, HFMODEM_IOCTL_CLEARRQ, 0))
		errstr(SEV_FATAL, "ioctl: HFMODEM_IOCTL_CLEARRQ");
}

/* --------------------------------------------------------------------- */

void l1_fsk_tx_request(l1_time_t tstart, l1_time_t tinc, int freqdev, 
		       int inv, l1_id_t id, unsigned int nbits, 
		       unsigned char *data)
{
	struct hfmodem_ioctl_fsk_tx_request rq;

	rq.tstart = tstart;
	rq.tinc = tinc;
	rq.freq[0] = freq_space + freqdev;
	rq.freq[1] = freq_mark + freqdev;
	rq.inv = inv;
	rq.id = id;
	rq.nbits = nbits;
	rq.data = data;
	if (ioctl(fd_krnl, HFMODEM_IOCTL_FSKTXREQUEST, &rq))
		errstr(SEV_FATAL, "ioctl: HFMODEM_IOCTL_FSKTXREQUEST");
}

/* --------------------------------------------------------------------- */

void l1_fsk_rx_request(l1_time_t tstart, l1_time_t tinc, int freqdev, 
		       int baud, l1_id_t id, unsigned int nbits,
		       l1_soft_t *data)
{
	struct hfmodem_ioctl_fsk_rx_request rq;

	rq.tstart = tstart;
	rq.tinc = tinc;
	rq.baud = baud;
	rq.id = id;
	rq.nbits = nbits;
	rq.data = data;
	rq.freq[0] = freq_space + freqdev;
	rq.freq[1] = freq_mark + freqdev;
#if 1
	if (ioctl(fd_krnl, HFMODEM_IOCTL_FSKRXREQUEST, &rq))
		errstr(SEV_FATAL, "ioctl: HFMODEM_IOCTL_FSKRXREQUEST");
#else
	for (;;) {
		if (!ioctl(fd_krnl, HFMODEM_IOCTL_FSKRXREQUEST, &rq))
			return;
		if (errno != EAGAIN)
			errstr(SEV_FATAL, "ioctl: HFMODEM_IOCTL_FSKRXREQUEST");
		errstr(SEV_WARNING, "ioctl: HFMODEM_IOCTL_FSKRXREQUEST");
	}
#endif
}

/* --------------------------------------------------------------------- */

l1_id_t l1_wait_request(void)
{
	fd_set mask;
	hfmodem_id_t id;
	struct timeval tv;

	for(;;) {
		/* check for cancel */
		if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL))
			errstr(SEV_FATAL, "pthread_setcancelstate");
		pthread_testcancel();
		if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL))
			errstr(SEV_FATAL, "pthread_setcancelstate");
		/* wait for request */
		FD_ZERO(&mask);
		FD_SET(fd_krnl, &mask);
		tv.tv_sec = 0;
		tv.tv_usec = 250000;
		if (select(fd_krnl+1, &mask, NULL, NULL, &tv) < 0) {
			if (errno == EAGAIN || errno == EINTR) {
				errstr(SEV_WARNING, "select hmmmm?");
				continue;
			}
			errstr(SEV_FATAL, "select");
		}
		if (!ioctl(fd_krnl, HFMODEM_IOCTL_WAITRQ, &id))
			return id;
		if (errno != EAGAIN)
			errstr(SEV_FATAL, "ioctl: HFMODEM_IOCTL_WAITRQ");
	}	
}

/* --------------------------------------------------------------------- */

void l1_set_mixer(unsigned int src, int igain, int ogain)
{
	struct hfmodem_ioctl_mixer_params mix;

	mix.src = src;
	mix.igain = igain;
	mix.ogain = ogain;
	if (ioctl(fd_krnl, HFMODEM_IOCTL_MIXERPARAMS, &mix))
		errstr(SEV_FATAL, "ioctl: HFMODEM_IOCTL_MIXERPARAMS");
}

/* --------------------------------------------------------------------- */

void l1_start_sample(short *data, unsigned int len)
{
	struct hfmodem_ioctl_sample_params spar;

	spar.data = data;
	spar.len = len;
	if (ioctl(fd_krnl, HFMODEM_IOCTL_SAMPLESTART, &spar))
		errstr(SEV_FATAL, "ioctl: HFMODEM_IOCTL_SAMPLESTART");
}

/* --------------------------------------------------------------------- */

int l1_sample_finished(void)
{
	if (!ioctl(fd_krnl, HFMODEM_IOCTL_SAMPLEFINISHED, 0))
		return 1;
	if (errno == EAGAIN || errno == EPIPE)
		return 0;
	errstr(SEV_FATAL, "ioctl: HFMODEM_IOCTL_SAMPLEFINISHED");
	return 0; /* never reached */
}

/* --------------------------------------------------------------------- */
