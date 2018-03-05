/*****************************************************************************/

/*
 *      l1.h  --  HF modem upper layer protocol interface.
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

#ifndef _L1_H
#define _L1_H

/* --------------------------------------------------------------------- */

#define SAMPLE_RATE  8000

/* --------------------------------------------------------------------- */

typedef unsigned long l1_time_t;
typedef int l1_soft_t;
typedef unsigned long l1_id_t;

/* --------------------------------------------------------------------- */

/*
 * layer 2 (protocol) interface
 */
extern void l1_init(const char *name_audio, const char *name_ptt, 
		    const char *name_mixer, float sndcorr, float tvuseccorr, 
		    float cpumhz, int dis_rdtsc, int invert_ptt);
extern l1_time_t l1_get_current_time(void);
extern void l1_set_fsk_params(unsigned int freqspace, unsigned int freqmark);
extern void l1_clear_requests(void);
extern void l1_fsk_tx_request(l1_time_t tstart, l1_time_t tinc, int freqdev, 
			      int inv, l1_id_t id, unsigned int nbits, 
			      unsigned char *data);
extern void l1_fsk_rx_request(l1_time_t tstart, l1_time_t tinc, int freqdev, 
			      int baud, l1_id_t id, unsigned int nbits,
			      l1_soft_t *data);
extern l1_id_t l1_wait_request(void);
extern void l1_set_mixer(unsigned int src, int igain, int ogain);
extern void l1_start_sample(short *data, unsigned int len);
extern int l1_sample_finished(void);

/* --------------------------------------------------------------------- */
#endif /* _L1FSK_H */
