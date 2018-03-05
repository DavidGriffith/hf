 /*****************************************************************************/
/*
 *      fskl1.h  --  binary FSK modem code.
 *     	            --  HF modem low layer protocol interface.
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
      
#ifndef _FSKL1_H
#define _FSKL1_H
      
/* --------------------------------------------------------------------- */
#include <pthread.h>

#define SAMPLE_RATE  8000

#ifndef FREQ_SPACE
#define FREQ_SPACE 1275
#endif

#ifndef FREQ_MARK
#define FREQ_MARK  1475
#endif

#define L1CORR_LEN (SAMPLE_RATE/40)

/* the floating point sine tab (used in the filter coefficient generator) */
#define SINTABBITS  9
#define SINTABSIZE  (1<<SINTABBITS)

#define CORRELATOR_CACHE 8

#ifdef MODEM_FLOAT
typedef float conv_t;
#else /* MODEM_FLOAT */
typedef int conv_t;
#endif /* MODEM_FLOAT */

#undef DEBUG_MSG
//#define DEBUG_MSG

typedef unsigned long l1_time_t;
typedef int l1_soft_t;
typedef unsigned long l1_id_t;
enum l1_slot_st { l1ss_unused, l1ss_ready, l1ss_oper, l1ss_retired };

struct l1rxslots {
 	enum l1_slot_st state;
	l1_time_t tstart, tinc;
	l1_soft_t *data;
	unsigned int nbits;
	unsigned int cntbits;
	unsigned long id;
	unsigned int corrlen;
	conv_t scale;
	unsigned int corr_cache; /* this is where the freqdev param is going */
};

struct l1txslots {
	enum l1_slot_st state;
	l1_time_t tstart, tinc;
	unsigned char *data;
	unsigned int nbits;
	unsigned int cntbits;
	unsigned long id;
	unsigned char inv;
	unsigned int phinc;
	unsigned int phases[2]; /* this is where freqdev goes */
};

struct corr_cache {
	int refcnt;
	int lru;
	unsigned int phase_incs[2];
	conv_t correlator[2][2][L1CORR_LEN];
};
extern struct corr_cache corr_cache[CORRELATOR_CACHE]; 

extern pthread_cond_t l1cond;
extern pthread_mutex_t l1mutex;
#define NUMRXSLOTS 20
#define NUMTXSLOTS 4
#ifdef HFMODEM_FLOAT
extern float fsintab[SINTABSIZE+SINTABSIZE/4];
#endif /* HFMODEM_FLOAT */

/* --------------------------------------------------------------------- */

/*
 * layer 2 (protocol) interface
 */
void l1_init();
/*
void l1_init(const char *name_audio, const char *name_ptt, 
    const char *name_mixer, 
    float sndcorr, float tvuseccorr, float cpumhz, int dis_rdtsc, int invptt);
*/
extern l1_time_t l1_get_current_time(void);
extern void l1_set_fsk_params(unsigned int freqspace, unsigned int freqmark);
extern void l1_fsk_clear_requests(void);
extern void l1_fsk_tx_request(l1_time_t tstart, l1_time_t tinc, int freqdev, 
			      int inv, l1_id_t id, unsigned int nbits, 
			      unsigned char *data);
extern void l1_fsk_rx_request(l1_time_t tstart, l1_time_t tinc, int freqdev, 
			      int baud, l1_id_t id, unsigned int nbits,
			      l1_soft_t *data);
extern l1_id_t l1_fsk_wait_request(void);
extern l1_id_t l1_fsk_wait_tx_request(void);
extern void l1_set_mixer(unsigned int src, int igain, int ogain);
extern void l1_start_sample(short *data, unsigned int len);
extern int l1_sample_finished(void);


/* --------------------------------------------------------------------- */

/*
 * OS sound driver interface
 */
void l1_fsk_input_samples(l1_time_t tstart, l1_time_t tinc, 
	 short *samples, unsigned int nsamples);
int l1_fsk_output_samples(l1_time_t tstart, l1_time_t tinc, 
	 short *samples, unsigned int nsamples);
long l1_fsk_next_tx_event(l1_time_t current);
void l1_fsk_finish_pending_rx_requests(void);
void l1_fsk_modem_init(void);

/* --------------------------------------------------------------------- */

void refclock_probe(float tvuseccorr, float cpumhz, int dis_rdtsc);
void refclock_init(void);
l1_time_t refclock_current(l1_time_t expected, int exp_valid);

/* --------------------------------------------------------------------- */

extern int force_halfduplex;
extern int nommap;
extern int brake;

/* --------------------------------------------------------------------- */

#endif /* _FSKL1_H */
