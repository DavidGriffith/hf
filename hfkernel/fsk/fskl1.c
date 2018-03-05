/*****************************************************************************/

/*
 *      fsk.c  --  binary FSK modem code.
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

#include <stdlib.h>
#include <syslog.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"

/* --------------------------------------------------------------------- */

static l1_time_t last_time = 0; 
/*static  */
pthread_cond_t l1cond = PTHREAD_COND_INITIALIZER;
/*static */
pthread_mutex_t l1mutex = PTHREAD_MUTEX_INITIALIZER;

static struct l1rxslots l1rxslots[NUMRXSLOTS];
static struct l1txslots l1txslots[NUMTXSLOTS];
static int isintab[SINTABSIZE+SINTABSIZE/4];
#ifdef HFMODEM_FLOAT
static float fsintab[SINTABSIZE+SINTABSIZE/4];
#endif /* HFMODEM_FLOAT */

struct corr_cache corr_cache[CORRELATOR_CACHE];
static unsigned int tx_phase = 0;
static unsigned int fsk_freq[2];

/* --------------------------------------------------------------------- */

#include "fskinlines.h"

/* --------------------------------------------------------------------- */

l1_time_t l1_get_current_time(void)
{
	return last_time + 20000L; /* heuristic */
}

/* --------------------------------------------------------------------- */

void l1_fsk_modem_init(void)
{
	unsigned int i;

	if (pthread_mutex_lock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	last_time = 0;
	for (i = 0; i < NUMRXSLOTS; i++)
		l1rxslots[i].state = l1ss_unused;
	for (i = 0; i < NUMTXSLOTS; i++)
		l1txslots[i].state = l1ss_unused;
	memset(corr_cache, 0, sizeof(corr_cache));
	for (i = 0; i < (SINTABSIZE+SINTABSIZE/4); i++)
		isintab[i] = 32767.0 * sin(2.0 * M_PI / SINTABSIZE * i);
#ifdef MODEM_FLOAT
	for (i = 0; i < (SINTABSIZE+SINTABSIZE/4); i++)
		fsintab[i] = sin(2.0 * M_PI / SINTABSIZE * i);
#endif /* MODEM_FLOAT */
       	if (pthread_mutex_unlock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
	l1_set_fsk_params(FREQ_SPACE, FREQ_MARK);
}

/* --------------------------------------------------------------------- */

void l1_set_fsk_params(unsigned int freqspace, unsigned int freqmark)
{
	errprintf(SEV_INFO, "FSK Parameters: Space frequency: %d  Mark frequency: %d\n",
		  freqspace, freqmark);
	fsk_freq[0] = freqspace;
	fsk_freq[1] = freqmark;
}

/* --------------------------------------------------------------------- */

void l1_fsk_clear_requests(void)
{
	int i;

	if (pthread_mutex_lock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	for (i = 0; i < NUMRXSLOTS; i++) 
		l1rxslots[i].state = l1ss_unused;
	for (i = 0; i < NUMTXSLOTS; i++)
		l1txslots[i].state = l1ss_unused;
	for (i = 0; i < CORRELATOR_CACHE; i++)
		corr_cache[i].refcnt = 0;
	if (pthread_mutex_unlock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
}

/* --------------------------------------------------------------------- */

static void dbg(void)
{
	errprintf(SEV_INFO, "txrqs: %1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld\n",
		  l1txslots[0].state, l1txslots[0].tstart, l1txslots[0].tstart+
		  (l1txslots[0].nbits-l1txslots[0].cntbits)*l1txslots[0].tinc, 
		  l1txslots[1].state, l1txslots[1].tstart, l1txslots[1].tstart+
		  (l1txslots[1].nbits-l1txslots[1].cntbits)*l1txslots[1].tinc, 
		  l1txslots[2].state, l1txslots[2].tstart, l1txslots[2].tstart+
		  (l1txslots[2].nbits-l1txslots[2].cntbits)*l1txslots[2].tinc, 
		  l1txslots[3].state, l1txslots[3].tstart, l1txslots[3].tstart+
		  (l1txslots[3].nbits-l1txslots[3].cntbits)*l1txslots[3].tinc); 
	errprintf(SEV_INFO, "rxrqs: %1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld / "
		  "%1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld / "
		  "%1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld / "
		  "%1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld / %1d %8ld %8ld\n",
		  l1rxslots[0].state, l1rxslots[0].tstart, l1rxslots[0].tstart+
		  (l1rxslots[0].nbits-l1rxslots[0].cntbits)*l1rxslots[0].tinc, 
		  l1rxslots[1].state, l1rxslots[1].tstart, l1rxslots[1].tstart+
		  (l1rxslots[1].nbits-l1rxslots[1].cntbits)*l1rxslots[1].tinc, 
		  l1rxslots[2].state, l1rxslots[2].tstart, l1rxslots[2].tstart+
		  (l1rxslots[2].nbits-l1rxslots[2].cntbits)*l1rxslots[2].tinc, 
		  l1rxslots[3].state, l1rxslots[3].tstart, l1rxslots[3].tstart+
		  (l1rxslots[3].nbits-l1rxslots[3].cntbits)*l1rxslots[3].tinc,
		  l1rxslots[4].state, l1rxslots[4].tstart, l1rxslots[4].tstart+
		  (l1rxslots[4].nbits-l1rxslots[4].cntbits)*l1rxslots[4].tinc, 
		  l1rxslots[5].state, l1rxslots[5].tstart, l1rxslots[5].tstart+
		  (l1rxslots[5].nbits-l1rxslots[5].cntbits)*l1rxslots[5].tinc, 
		  l1rxslots[6].state, l1rxslots[6].tstart, l1rxslots[6].tstart+
		  (l1rxslots[6].nbits-l1rxslots[6].cntbits)*l1rxslots[6].tinc, 
		  l1rxslots[7].state, l1rxslots[7].tstart, l1rxslots[7].tstart+
		  (l1rxslots[7].nbits-l1rxslots[7].cntbits)*l1rxslots[7].tinc,
		  l1rxslots[8].state, l1rxslots[8].tstart, l1rxslots[8].tstart+
		  (l1rxslots[8].nbits-l1rxslots[8].cntbits)*l1rxslots[8].tinc, 
		  l1rxslots[9].state, l1rxslots[9].tstart, l1rxslots[9].tstart+
		  (l1rxslots[9].nbits-l1rxslots[9].cntbits)*l1rxslots[9].tinc, 
		  l1rxslots[10].state, l1rxslots[10].tstart, l1rxslots[10].tstart+
		  (l1rxslots[10].nbits-l1rxslots[10].cntbits)*l1rxslots[10].tinc, 
		  l1rxslots[11].state, l1rxslots[11].tstart, l1rxslots[11].tstart+
		  (l1rxslots[11].nbits-l1rxslots[11].cntbits)*l1rxslots[11].tinc,
		  l1rxslots[12].state, l1rxslots[12].tstart, l1rxslots[12].tstart+
		  (l1rxslots[12].nbits-l1rxslots[12].cntbits)*l1rxslots[12].tinc, 
		  l1rxslots[13].state, l1rxslots[13].tstart, l1rxslots[13].tstart+
		  (l1rxslots[13].nbits-l1rxslots[13].cntbits)*l1rxslots[13].tinc, 
		  l1rxslots[14].state, l1rxslots[14].tstart, l1rxslots[14].tstart+
		  (l1rxslots[14].nbits-l1rxslots[14].cntbits)*l1rxslots[14].tinc, 
		  l1rxslots[15].state, l1rxslots[15].tstart, l1rxslots[15].tstart+
		  (l1rxslots[15].nbits-l1rxslots[15].cntbits)*l1rxslots[15].tinc);
}

/* --------------------------------------------------------------------- */

void l1_fsk_tx_request(l1_time_t tstart, l1_time_t tinc, int freqdev, int inv,
		   unsigned long id, unsigned int nbits, unsigned char *data)
{
	int i, j;

#ifdef DEBUG_MSG
	errprintf(SEV_INFO, "l1_fsk_tx_request: cur time %8ld  req time %8ld  id %08lx\n", last_time, tstart, id);
#endif /* DEBUG_MSG */
	if (pthread_mutex_lock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	for (i = 0; i < NUMTXSLOTS && l1txslots[i].state != l1ss_unused; i++);
	if (i >= NUMTXSLOTS) {
		dbg();
		errprintf(SEV_FATAL, "l1_fsk_tx_request: out of tx requests, current id %lu\n", id);
	}
	if (!data || nbits <= 0) {
		l1txslots[i].state = l1ss_retired;
		l1txslots[i].id = id;
	} else {
		l1txslots[i].state = l1ss_ready;
		l1txslots[i].tstart = tstart;
		l1txslots[i].tinc = tinc;
		l1txslots[i].data = data;
		l1txslots[i].nbits = nbits;
		l1txslots[i].cntbits = 0;
		l1txslots[i].inv = inv ? 0xff : 0;
		l1txslots[i].id = id;
		for (j = 0; j < 2; j++)
			l1txslots[i].phases[j] = ((fsk_freq[j]+freqdev)*0x10000+(SAMPLE_RATE/2))/SAMPLE_RATE;
	}
	if (pthread_mutex_unlock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
//	dbg();
}

/* --------------------------------------------------------------------- */

void l1_fsk_rx_request(l1_time_t tstart, l1_time_t tinc, int freqdev, int baud, 
		   unsigned long id, unsigned int nbits, l1_soft_t *data)
{
	int i;

#ifdef DEBUG_MSG
	errprintf(SEV_INFO, "l1_fsk_rx_request: cur time %8ld  req time %8ld  id %08lx\n", last_time, tstart, id);
#endif /* DEBUG_MSG */
	if (pthread_mutex_lock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	for (i = 0; i < NUMRXSLOTS && l1rxslots[i].state != l1ss_unused; i++);
	if (i >= NUMRXSLOTS) {
		dbg();
		errprintf(SEV_FATAL, "l1_fsk_rx_request: out of rx requests, current id %lu\n", id);
	}
	l1rxslots[i].id = id;
	l1rxslots[i].corrlen = SAMPLE_RATE/baud;
	cc_prepare(l1rxslots+i, ((fsk_freq[0]+freqdev)*0x10000+(SAMPLE_RATE/2))/SAMPLE_RATE, 
		   ((fsk_freq[1]+freqdev)*0x10000+(SAMPLE_RATE/2))/SAMPLE_RATE);
	if (!data || nbits <= 0) {
		if (nbits > 0 && data)
			memset(data, 0, nbits * sizeof(int));
		l1rxslots[i].state = l1ss_retired;
	} else {
		l1rxslots[i].state = l1ss_ready;
		l1rxslots[i].tstart = tstart;
		l1rxslots[i].tinc = tinc;
		l1rxslots[i].data = data;
		l1rxslots[i].nbits = nbits;
		l1rxslots[i].cntbits = 0;
	}
	if (pthread_mutex_unlock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
}

/* --------------------------------------------------------------------- */

static void fsk_wait_cleanup(void *dummy)
{
	if (pthread_mutex_unlock(&l1mutex))
	    errstr(SEV_FATAL, "pthread_mutex_unlock");
}

/* --------------------------------------------------------------------- */

unsigned long l1_fsk_wait_request(void)
{
	//is called by mode_(...) from l2 thread
	int i, idx, cnt;
	unsigned long id;
	l1_time_t tm = 0;

	if (pthread_mutex_lock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	pthread_cleanup_push(fsk_wait_cleanup, NULL);
	if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL))
		errstr(SEV_FATAL, "pthread_setcancelstate");
	for (;;) {
		for (idx = -1, cnt = i = 0; i < NUMRXSLOTS; i++) {
			if (l1rxslots[i].state == l1ss_retired) 
				if (idx < 0 || (signed)(tm - l1rxslots[i].tstart) > 0) {
					tm = l1rxslots[i].tstart;
					idx = i;
				}
			cnt += (l1rxslots[i].state != l1ss_unused);
		}
		if (idx >= 0) {
			cc_unlock(l1rxslots[idx].corr_cache);
			id = l1rxslots[idx].id;
			l1rxslots[idx].state = l1ss_unused;
			i = 1;
			break;
		}
		for (idx = -1, i = 0; i < NUMTXSLOTS; i++) {
			if (l1txslots[i].state == l1ss_retired)
				if (idx < 0 || (signed)(tm - l1txslots[i].tstart) > 0) {
					tm = l1txslots[i].tstart;
					idx = i;
				}
			cnt += (l1txslots[i].state != l1ss_unused);
		}
		if (idx >= 0) {
			id = l1txslots[idx].id;
			l1txslots[idx].state = l1ss_unused;
			i = 0;
			break;
		}
		if (cnt <= 0)
		// was: fatal
			errprintf(SEV_WARNING, 
			    "l1_fsk_wait_request: no requests pending");
		if (pthread_cond_wait(&l1cond, &l1mutex))
			errstr(SEV_FATAL, "pthread_cond_wait");
	}
	pthread_cleanup_pop(0);
	// //=6.12.06 
	//if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL))
	//	errstr(SEV_FATAL, "pthread_setcancelstate");
	if (pthread_mutex_unlock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
#ifdef DEBUG_MSG
	errprintf(SEV_INFO, "retiring id %lx  rx %d\n", id, i);
#endif /* DEBUG_MSG */
	return id;
}

/* --------------------------------------------------------------------- */

void l1_fsk_input_samples(l1_time_t tstart, l1_time_t tinc, 
		      short *samples, unsigned int nsamples)
{
	l1_time_t tst, tend;
	short *s;
	int i, j;
	l1_soft_t sample;

	if (pthread_mutex_lock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	last_time = tstart + (nsamples - 1) * tinc;
	for (i = 0; i < NUMRXSLOTS; i++) {
	    if (l1rxslots[i].state == 
		    l1ss_unused || l1rxslots[i].state == l1ss_retired)
		continue;
	    tst = tstart - (l1rxslots[i].corrlen-1) * tinc;
	    tend = tst + (nsamples-1) * tinc;
	    if (l1rxslots[i].state == l1ss_ready) {
		if ((signed)(l1rxslots[i].tstart - tend) > 0) 
		    continue;
		    l1rxslots[i].state = l1ss_oper;
	    }
	    for (s = samples, j = 0; j < nsamples; j++, s++, tst += tinc)
		if ((signed)(l1rxslots[i].tstart - tst) <= 0) {
		    sample = do_filter(l1rxslots+i, s);
		    while ((signed)(l1rxslots[i].tstart - tst) <= 0 && 
		           l1rxslots[i].cntbits < l1rxslots[i].nbits) {
			l1rxslots[i].data[l1rxslots[i].cntbits] = sample;
			l1rxslots[i].cntbits++;
			l1rxslots[i].tstart += l1rxslots[i].tinc;
		    }
		    if (l1rxslots[i].cntbits >= l1rxslots[i].nbits) {
			l1rxslots[i].state = l1ss_retired;
			if (pthread_cond_signal(&l1cond))
		    	    errstr(SEV_FATAL, "pthread_cond_signal");
			break;
		    }
		}
	    }
	if (pthread_mutex_unlock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
}

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int l1fsk_phinc(struct l1txslots *s, unsigned int nbit)
{
	return s->phases[!!((s->data[nbit >> 3] ^ s->inv) & (1 << (nbit & 7)))];
}

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int output_one_sample(l1_time_t tm)
{
	int i, j, k;

	/*
	 * first activate new output slots
	 */
	for (j = -1, i = 0; i < NUMTXSLOTS; i++) {
		if (l1txslots[i].state == l1ss_ready && (signed)(l1txslots[i].tstart - tm) <= 0) {
			for (k = 0; k < NUMTXSLOTS; k++) {
				if (l1txslots[k].state != l1ss_oper)
					continue;
				l1txslots[k].state = l1ss_retired;
				if (pthread_cond_signal(&l1cond))
					errstr(SEV_FATAL, "pthread_cond_signal");
			}
			l1txslots[i].state = l1ss_oper;
			l1txslots[i].tstart += l1txslots[i].tinc;
			l1txslots[i].phinc = l1fsk_phinc(l1txslots+i, 0);
			l1txslots[i].cntbits = 1;
		};
		if (l1txslots[i].state != l1ss_oper)
			continue;
		j = i;
	}
	if (j < 0 || j >= NUMTXSLOTS)
		return 0;
	/*
	 * calculate the current slot
	 */
	while ((signed)(l1txslots[j].tstart - tm) <= 0) {
		if (l1txslots[j].cntbits >= l1txslots[j].nbits) {
			l1txslots[j].state = l1ss_retired;
			if (pthread_cond_signal(&l1cond))
				errstr(SEV_FATAL, "pthread_cond_signal");
			return 0;
		}
		l1txslots[j].tstart += l1txslots[j].tinc;
		l1txslots[j].phinc = l1fsk_phinc(l1txslots+j, l1txslots[j].cntbits);
		l1txslots[j].cntbits++;
	}
	return l1txslots[j].phinc;
}

/* --------------------------------------------------------------------- */

int l1_fsk_output_samples(l1_time_t tstart, l1_time_t tinc, 
		      short *samples, unsigned int nsamples)
{
	int i, j;
	l1_time_t tend = tstart + (nsamples-1) * tinc;

// errprintf(SEV_INFO, "output samples: %8ld %8ld\n", tstart, tstart+tinc*nsamples);
	if (pthread_mutex_lock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	for (i = 0; i < NUMTXSLOTS; i++) {
		if (l1txslots[i].state == l1ss_oper)
			break;
		if (l1txslots[i].state == l1ss_ready && (signed)(l1txslots[i].tstart - tend) <= 0)
			break;
	}
	if (i >= NUMTXSLOTS) {
		if (pthread_mutex_unlock(&l1mutex))
			errstr(SEV_FATAL, "pthread_mutex_unlock");
		return 0;
	}
	for (j = 0; j < nsamples; j++, tstart += tinc, samples++) {
		*samples = isimplesin(tx_phase);
		tx_phase += output_one_sample(tstart);
	}
	if (pthread_mutex_unlock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
	return 1;
}

/* --------------------------------------------------------------------- */

long l1_fsk_next_tx_event(l1_time_t current)
{
	long diff = LONG_MAX, t;
	int i;

	if (pthread_mutex_lock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	for (i = 0; i < NUMTXSLOTS; i++) {
		if (l1txslots[i].state == l1ss_oper)
			if (diff > 0)
				diff = 0;
		if (l1txslots[i].state == l1ss_ready) {
			t = l1txslots[i].tstart - current;
			if (t < diff)
				diff = t;
		}
	}
	if (pthread_mutex_unlock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
	return diff;
}

/* --------------------------------------------------------------------- */

void l1_fsk_finish_pending_rx_requests(void)
{
	int i;

	if (pthread_mutex_lock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	for (i = 0; i < NUMRXSLOTS; i++) {
		if (l1rxslots[i].state != l1ss_oper)
			continue;
		while (l1rxslots[i].cntbits < l1rxslots[i].nbits) {
			l1rxslots[i].data[l1rxslots[i].cntbits] = 0;
			l1rxslots[i].cntbits++;
		}
		l1rxslots[i].state = l1ss_retired;
		if (pthread_cond_signal(&l1cond))
			errstr(SEV_FATAL, "pthread_cond_signal");
	}
	if (pthread_mutex_unlock(&l1mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
}

/* --------------------------------------------------------------------- */
