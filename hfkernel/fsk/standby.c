/*****************************************************************************/

/*
 *      standby.c  --  HF standby mode (search for pactor/amtor calling).
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
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h> 
#include <assert.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"
#include "pactor.h"
#include "amtor.h"
#include "gtor.h"
#include "standby.h"

/* --------------------------------------------------------------------- */

#define NUMBUF  32
#define BUFLEN_100 64          /* 4.32sec / (NUMBUF-2) / 10ms * STANDBY_OVERSAMPLING_100 */
#define BUFLEN_200 64          /* 4.32sec / (NUMBUF-2) / 5ms * STANDBY_OVERSAMPLING_200 */
#define BUFLEN_300 128         /* 4.32sec / (NUMBUF-2) / 3.33ms * STANDBY_OVERSAMPLING_300 */

#define TIME_INC_100 (1000000/100/STANDBY_OVERSAMPLING_100)
#define TIME_INC_200 (1000000/200/STANDBY_OVERSAMPLING_200)
#define TIME_INC_300 (1000000/300/STANDBY_OVERSAMPLING_300)

#define FREQ_TRACKING

#define NUM_FREQ_TRK 4

#ifdef FREQ_TRACKING
static const int freq_trk_dev[NUM_FREQ_TRK] = { -30, 30, -60, 60 };
#endif /* FREQ_TRACKING */

/* --------------------------------------------------------------------- */

//moved to /os/main.c
//unsigned int standby_flags = STANDBY_FLAG_FREQ_ESTIMATION|STANDBY_FLAG_MONITOR_200|STANDBY_FLAG_MONITOR_300;

/* --------------------------------------------------------------------- */
/*
 * 100 baud ring buffers
 */

static struct {
	unsigned int bufidx[2];
	l1_time_t buftimes[2];
	l1_soft_t buf[2*NUMBUF][BUFLEN_100];
#ifdef FREQ_TRACKING
	l1_soft_t trkbuf[NUM_FREQ_TRK][2*NUMBUF][BUFLEN_100];
#endif /* FREQ_TRACKING */
} b1;

static void init_100(l1_time_t tm)
{
	unsigned int k;

	memset(&b1, 0, sizeof(b1));
	b1.buftimes[0] = tm;
	b1.buftimes[1] = tm + BUFLEN_100 * TIME_INC_100;
	b1.bufidx[0] = 0;
	b1.bufidx[1] = 1;
	l1_fsk_rx_request(b1.buftimes[0], TIME_INC_100, 0, 100, 0x1100, BUFLEN_100, b1.buf[0]);
	l1_fsk_rx_request(b1.buftimes[1], TIME_INC_100, 0, 100, 0x1101, BUFLEN_100, b1.buf[1]);
#ifdef FREQ_TRACKING
	if (!(standby_flags & STANDBY_FLAG_FREQ_ESTIMATION))
		return;
	for (k = 0; k < NUM_FREQ_TRK; k++) {
		l1_fsk_rx_request(b1.buftimes[0], TIME_INC_100, freq_trk_dev[k], 100, 0x1110+2*k, BUFLEN_100, b1.trkbuf[k][0]);
		l1_fsk_rx_request(b1.buftimes[1], TIME_INC_100, freq_trk_dev[k], 100, 0x1111+2*k, BUFLEN_100, b1.trkbuf[k][1]);
	}
#endif /* FREQ_TRACKING */
}

static int handle_100(l1_id_t id)
{
	l1_time_t tm, tms;
	l1_soft_t *s;
	unsigned int i = id & 1;
	unsigned char pkt[12];
	unsigned int j, k;
#ifdef FREQ_TRACKING
	l1_soft_t *trks[NUM_FREQ_TRK];
	int freqdev;
#endif /* FREQ_TRACKING */

	if (id < 0x1100 || id > 0x1101)
		return 0;
	tms = tm = b1.buftimes[i];
	memcpy(s = b1.buf[b1.bufidx[i]+NUMBUF], b1.buf[b1.bufidx[i]], sizeof(b1.buf[0]));
#ifdef FREQ_TRACKING
	for (k = 0; k < NUM_FREQ_TRK; k++)
		memcpy(trks[k] = b1.trkbuf[k][b1.bufidx[i]+NUMBUF], b1.trkbuf[k][b1.bufidx[i]], sizeof(b1.trkbuf[0][0]));
#endif /* FREQ_TRACKING */
	b1.bufidx[i] = (b1.bufidx[i] + 2) % NUMBUF;
	b1.buftimes[i] += 2 * BUFLEN_100 * TIME_INC_100;
	l1_fsk_rx_request(b1.buftimes[i], TIME_INC_100, 0, 100, id, BUFLEN_100, b1.buf[b1.bufidx[i]]);
#ifdef FREQ_TRACKING
	for (k = 0; k < NUM_FREQ_TRK; k++)
		l1_fsk_rx_request(b1.buftimes[i], TIME_INC_100, freq_trk_dev[k], 100, id+0x10+2*k, 
				  BUFLEN_100, b1.trkbuf[k][b1.bufidx[i]]);
#endif /* FREQ_TRACKING */
	/* process samples */
	for (j = 0; j < BUFLEN_100; j++, s++, tm += TIME_INC_100) {
		soft_to_hard(s-STANDBY_OVERSAMPLING_100*(12*8-1), pkt, 12*8, STANDBY_OVERSAMPLING_100, 0);
		pactor_monitor_100(pkt, tm-1000000/100);
		if (pactor_check_call(pkt, s-STANDBY_OVERSAMPLING_100*(12*8-1), tm-(1000000/100)*(12*8-1), 0))
			return 1;
		if (amtor_monitor_arq(tm, pkt[4] << 2 | pkt[3] >> 6, 
				      pkt[5] << 3 | pkt[4] >> 5, 
				      pkt[6] << 4 | pkt[5] >> 4, 
				      pkt[10] << 5 | pkt[9] >> 3, 
				      pkt[11] << 6 | pkt[10] >> 2,
				      pkt[11] >> 1, s, 0))
			return 1;
		if (amtor_monitor_fec(tm, pkt[11] >> 1, pkt[11] << 6 | pkt[10] >> 2,
				      pkt[10] << 5 | pkt[9] >> 3, pkt[9] << 4 | pkt[8] >> 4,
				      pkt[8] << 3 | pkt[7] >> 5, pkt[7] << 2 | pkt[6] >> 6,
				      pkt[6] << 1 | pkt[5] >> 7, pkt[5],
				      pkt[4] >> 1, pkt[4] << 6 | pkt[3] >> 2,
				      pkt[3] << 5 | pkt[2] >> 3, pkt[2] << 4 | pkt[1] >> 4, s, 0))
			return 1;
		if (gtor_monitor(1, s-STANDBY_OVERSAMPLING_100*(24*8-1), 
				 s-STANDBY_OVERSAMPLING_100*(24*8-1)-2400000/TIME_INC_100, 
				 tm-(1000000/100)*(24*8-1), STANDBY_OVERSAMPLING_100, 0))
			return 1;
	}
#ifdef FREQ_TRACKING
	if (!(standby_flags & STANDBY_FLAG_FREQ_ESTIMATION))
		return 0;
	for (k = 0; k < NUM_FREQ_TRK; k++) {
		freqdev = freq_trk_dev[k];
		for (s = trks[k], tm = tms, j = 0; j < BUFLEN_100; j++, s++, tm += TIME_INC_100) {
			soft_to_hard(s-STANDBY_OVERSAMPLING_100*(12*8-1), pkt, 12*8, STANDBY_OVERSAMPLING_100, 0);
			if (pactor_check_call(pkt, s-STANDBY_OVERSAMPLING_100*(12*8-1), tm-(1000000/100)*(12*8-1), freqdev))
				return 1;
			if (amtor_monitor_arq(tm, pkt[4] << 2 | pkt[3] >> 6, 
					      pkt[5] << 3 | pkt[4] >> 5, 
					      pkt[6] << 4 | pkt[5] >> 4, 
					      pkt[10] << 5 | pkt[9] >> 3, 
					      pkt[11] << 6 | pkt[10] >> 2,
					      pkt[11] >> 1, s, freqdev))
				return 1;
			if (amtor_monitor_fec(tm, pkt[11] >> 1, pkt[11] << 6 | pkt[10] >> 2,
					      pkt[10] << 5 | pkt[9] >> 3, pkt[9] << 4 | pkt[8] >> 4,
					      pkt[8] << 3 | pkt[7] >> 5, pkt[7] << 2 | pkt[6] >> 6,
					      pkt[6] << 1 | pkt[5] >> 7, pkt[5],
					      pkt[4] >> 1, pkt[4] << 6 | pkt[3] >> 2,
					      pkt[3] << 5 | pkt[2] >> 3, pkt[2] << 4 | pkt[1] >> 4, s, freqdev))
				return 1;
			if (gtor_monitor(1, s-STANDBY_OVERSAMPLING_100*(24*8-1), 
					 s-STANDBY_OVERSAMPLING_100*(24*8-1)-2400000/TIME_INC_100, 
					 tm-(1000000/100)*(24*8-1), STANDBY_OVERSAMPLING_100, freqdev))
				return 1;
		}
	}
#endif /* FREQ_TRACKING */
	return 0;
}

/* --------------------------------------------------------------------- */

static int handle_100_pactoronly(l1_id_t id)
{
	l1_time_t tm, tms;
	l1_soft_t *s;
	unsigned int i = id & 1;
	unsigned char pkt[12];
	unsigned int j, k;
#ifdef FREQ_TRACKING
	l1_soft_t *trks[NUM_FREQ_TRK];
	int freqdev;
#endif /* FREQ_TRACKING */

	if (id < 0x1100 || id > 0x1101)
		return 0;
	tms = tm = b1.buftimes[i];
	memcpy(s = b1.buf[b1.bufidx[i]+NUMBUF], b1.buf[b1.bufidx[i]], sizeof(b1.buf[0]));
#ifdef FREQ_TRACKING
	for (k = 0; k < NUM_FREQ_TRK; k++)
		memcpy(trks[k] = b1.trkbuf[k][b1.bufidx[i]+NUMBUF], b1.trkbuf[k][b1.bufidx[i]], sizeof(b1.trkbuf[0][0]));
#endif /* FREQ_TRACKING */
	b1.bufidx[i] = (b1.bufidx[i] + 2) % NUMBUF;
	b1.buftimes[i] += 2 * BUFLEN_100 * TIME_INC_100;
	l1_fsk_rx_request(b1.buftimes[i], TIME_INC_100, 0, 100, id, BUFLEN_100, b1.buf[b1.bufidx[i]]);
#ifdef FREQ_TRACKING
	for (k = 0; k < NUM_FREQ_TRK; k++)
		l1_fsk_rx_request(b1.buftimes[i], TIME_INC_100, freq_trk_dev[k], 100, id+0x10+2*k, 
				  BUFLEN_100, b1.trkbuf[k][b1.bufidx[i]]);
#endif /* FREQ_TRACKING */
	/* process samples */
	for (j = 0; j < BUFLEN_100; j++, s++, tm += TIME_INC_100) {
		soft_to_hard(s-STANDBY_OVERSAMPLING_100*(12*8-1), pkt, 12*8, STANDBY_OVERSAMPLING_100, 0);
		pactor_monitor_100(pkt, tm-1000000/100);
		if (pactor_check_call(pkt, s-STANDBY_OVERSAMPLING_100*(12*8-1), tm-(1000000/100)*(12*8-1), 0))
			return 1;
	}
#ifdef FREQ_TRACKING
	if (!(standby_flags & STANDBY_FLAG_FREQ_ESTIMATION))
		return 0;
	for (k = 0; k < NUM_FREQ_TRK; k++) {
		freqdev = freq_trk_dev[k];
		for (s = trks[k], tm = tms, j = 0; j < BUFLEN_100; j++, s++, tm += TIME_INC_100) {
			soft_to_hard(s-STANDBY_OVERSAMPLING_100*(12*8-1), pkt, 12*8, STANDBY_OVERSAMPLING_100, 0);
			if (pactor_check_call(pkt, s-STANDBY_OVERSAMPLING_100*(12*8-1), tm-(1000000/100)*(12*8-1), freqdev))
				return 1;
		}
	}
#endif /* FREQ_TRACKING */
	return 0;
}



/* --------------------------------------------------------------------- */
/*
 * 200 baud ring buffers
 */

static struct {
	unsigned int bufidx[2];
	l1_time_t buftimes[2];
	l1_soft_t buf[2*NUMBUF][BUFLEN_200];
} b2;

static void init_200(l1_time_t tm)
{
	memset(&b2, 0, sizeof(b2));
	b2.buftimes[0] = tm;
	b2.buftimes[1] = tm + BUFLEN_200 * TIME_INC_200;
	b2.bufidx[0] = 0;
	b2.bufidx[1] = 1;
	l1_fsk_rx_request(b2.buftimes[0], TIME_INC_200, 0, 200, 0x1200, BUFLEN_200, b2.buf[0]);
	l1_fsk_rx_request(b2.buftimes[1], TIME_INC_200, 0, 200, 0x1201, BUFLEN_200, b2.buf[1]);
}

static void handle_200(l1_id_t id)
{
	l1_time_t tm;
	l1_soft_t *s;
	unsigned int i = id & 1;
	unsigned char pkt[24];
	unsigned int j;

	if (id < 0x1200 || id > 0x1201)
		return;
	tm = b2.buftimes[i];
	memcpy(s = b2.buf[b2.bufidx[i]+NUMBUF], b2.buf[b2.bufidx[i]], sizeof(b2.buf[0]));
	b2.bufidx[i] = (b2.bufidx[i] + 2) % NUMBUF;
	b2.buftimes[i] += 2 * BUFLEN_200 * TIME_INC_200;
	l1_fsk_rx_request(b2.buftimes[i], TIME_INC_200, 0, 200, id, BUFLEN_200, b2.buf[b2.bufidx[i]]);
	/* process samples */
	for (j = 0; j < BUFLEN_200; j++, s++, tm += TIME_INC_200) {
		soft_to_hard(s-STANDBY_OVERSAMPLING_200*(24*8-1), pkt, 24*8, STANDBY_OVERSAMPLING_200, 0);
		pactor_monitor_200(pkt, tm-1000000/200);
		gtor_monitor(2, s-STANDBY_OVERSAMPLING_200*(48*8-1), 
			     s-STANDBY_OVERSAMPLING_200*(48*8-1)-2400000/TIME_INC_200, 
			     tm-(1000000/200)*(48*8-1), STANDBY_OVERSAMPLING_200, 0);
	}
}


static void handle_200_pactoronly(l1_id_t id)
{
	l1_time_t tm;
	l1_soft_t *s;
	unsigned int i = id & 1;
	unsigned char pkt[24];
	unsigned int j;

	if (id < 0x1200 || id > 0x1201)
		return;
	tm = b2.buftimes[i];
	memcpy(s = b2.buf[b2.bufidx[i]+NUMBUF], b2.buf[b2.bufidx[i]], sizeof(b2.buf[0]));
	b2.bufidx[i] = (b2.bufidx[i] + 2) % NUMBUF;
	b2.buftimes[i] += 2 * BUFLEN_200 * TIME_INC_200;
	l1_fsk_rx_request(b2.buftimes[i], TIME_INC_200, 0, 200, id, BUFLEN_200, b2.buf[b2.bufidx[i]]);
	/* process samples */
	for (j = 0; j < BUFLEN_200; j++, s++, tm += TIME_INC_200) {
		soft_to_hard(s-STANDBY_OVERSAMPLING_200*(24*8-1), pkt, 24*8, STANDBY_OVERSAMPLING_200, 0);
		pactor_monitor_200(pkt, tm-1000000/200);
	}
}

/* --------------------------------------------------------------------- */
/*
 * 300 baud ring buffers
 */

static struct {
	unsigned int bufidx[2];
	l1_time_t buftimes[2];
	l1_soft_t buf[2*NUMBUF][BUFLEN_300];
} b3;

static void init_300(l1_time_t tm)
{
	memset(&b3, 0, sizeof(b3));
	b3.buftimes[0] = tm;
	b3.buftimes[1] = tm + BUFLEN_300 * TIME_INC_300;
	b3.bufidx[0] = 0;
	b3.bufidx[1] = 1;
	l1_fsk_rx_request(b3.buftimes[0], TIME_INC_300, 0, 300, 0x1300, BUFLEN_300, b3.buf[0]);
	l1_fsk_rx_request(b3.buftimes[1], TIME_INC_300, 0, 300, 0x1301, BUFLEN_300, b3.buf[1]);
}

static void handle_300(l1_id_t id)
{
	l1_time_t tm;
	l1_soft_t *s;
	unsigned int i = id & 1;
	unsigned int j;

	if (id < 0x1300 || id > 0x1301)
		return;
	tm = b3.buftimes[i];
	memcpy(s = b3.buf[b3.bufidx[i]+NUMBUF], b3.buf[b3.bufidx[i]], sizeof(b3.buf[0]));
	b3.bufidx[i] = (b3.bufidx[i] + 2) % NUMBUF;
	b3.buftimes[i] += 2 * BUFLEN_300 * TIME_INC_300;
	l1_fsk_rx_request(b3.buftimes[i], TIME_INC_300, 0, 300, id, BUFLEN_300, b3.buf[b3.bufidx[i]]);
	/* process samples */
	for (j = 0; j < BUFLEN_300; j++, s++, tm += TIME_INC_300) {
		gtor_monitor(3, s-STANDBY_OVERSAMPLING_300*(72*8-1), 
			     s-STANDBY_OVERSAMPLING_300*(72*8-1)-2400000/TIME_INC_300, 
			     tm-(1000000/300)*(72*8-1), STANDBY_OVERSAMPLING_300, 0);
	}
}

/* --------------------------------------------------------------------- */

void *mode_pactor_standby(void *dummy)
{
// test for only pactor sby
	l1_time_t tm;
	l1_id_t id;

	if (standby_flags & STANDBY_FLAG_MONITOR_200)  //by dl4mge
	    bufprintf(HFAPP_MSG_DATA_MONITOR, "mode_pactor_standby: will monitor 100 and 200 baud.\n");
	else bufprintf(HFAPP_MSG_DATA_MONITOR, "mode_pactor_standby: will monitor only 100 baud.\n");

//	if (standby_flags & STANDBY_FLAG_MONITOR_300)
//	    bufprintf(HFAPP_MSG_DATA_MONITOR, "mode_pactor_standby: will monitor 300 baud.");
	
	for (;;) {
		/* initialize everything */
		modefamily = FSK;
		l1_fsk_clear_requests();
		errprintf(SEV_INFO, "mode: pactor only standby\n");
		bufprintf(HFAPP_MSG_DATA_STATUS, "PACTOR ONLY STANDBY");
		send_short_msg(HFAPP_MSG_STATE_PACTOR_STANDBY, ERR_NOERR);
		tm = l1_get_current_time() + 100000;
		/* do some sort of staggered initialize 
		   to prevent accumulation of retired rx slots */
		pactor_monitor_init(tm);
//		amtor_monitor_init(tm);
		init_100(tm);
		if (standby_flags & STANDBY_FLAG_MONITOR_200)
			init_200(tm+20000);
//		if (standby_flags & STANDBY_FLAG_MONITOR_300)
//			init_300(tm+40000);
		for (;;) {
		    id = l1_fsk_wait_request();
		    if ((id & ~0xff) == 0x1100) {
			if (handle_100_pactoronly(id))
		    	break;
		    } else if ((id & ~0xff) == 0x1200) {
		        if (standby_flags & STANDBY_FLAG_MONITOR_200) {//by dl4mge
			//    bufprintf(HFAPP_MSG_DATA_MONITOR, 
			//	"will handle 100 and 200 baud pactor only.\n");
			    handle_200_pactoronly(id);
			}
			//else bufprintf(HFAPP_MSG_DATA_MONITOR, 
			//    "will not handle 200 baud pactor only.\n");			
		    }
//		    else if ((id & ~0xff) == 0x1300)
//				handle_300(id);
		    else
			errprintf(SEV_WARNING, "standby: invalid request id 0x%lx\n", id);
		}
	}
}


void *mode_standby(void *dummy)
{
	l1_time_t tm;
	l1_id_t id;

	for (;;) {
		/* initialize everything */
		modefamily = FSK;
		l1_fsk_clear_requests();
		errprintf(SEV_INFO, "mode: standby\n");
		bufprintf(HFAPP_MSG_DATA_STATUS, "STANDBY");
		send_short_msg(HFAPP_MSG_STATE_STANDBY, ERR_NOERR);
		tm = l1_get_current_time() + 100000;
		/* do some sort of staggered initialize to prevent accumulation of retired rx slots */
		pactor_monitor_init(tm);
		amtor_monitor_init(tm);
		init_100(tm);
		if (standby_flags & STANDBY_FLAG_MONITOR_200)
			init_200(tm+20000);
		if (standby_flags & STANDBY_FLAG_MONITOR_300)
			init_300(tm+40000);
		for (;;) {
			id = l1_fsk_wait_request();
			if ((id & ~0xff) == 0x1100) {
				if (handle_100(id))
					break;
			} else if ((id & ~0xff) == 0x1200)
				handle_200(id);
			else if ((id & ~0xff) == 0x1300)
				handle_300(id);
			else
				errprintf(SEV_WARNING, "standby: invalid request id 0x%lx\n", id);
		}
	}
}

/* --------------------------------------------------------------------- */

