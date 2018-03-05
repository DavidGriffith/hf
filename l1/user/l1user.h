/*****************************************************************************/

/*
 *      l1user.c  --  binary FSK modem code.
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
      
#ifndef _L1USER_H
#define _L1USER_H

/* --------------------------------------------------------------------- */

/*
 * OS sound driver interface
 */
extern void l1_input_samples(l1_time_t tstart, l1_time_t tinc, 
			     short *samples, unsigned int nsamples);
extern int l1_output_samples(l1_time_t tstart, l1_time_t tinc, 
			     short *samples, unsigned int nsamples);
extern long l1_next_tx_event(l1_time_t current);
extern void l1_finish_pending_rx_requests(void);
extern void l1_modem_init(void);

/* --------------------------------------------------------------------- */

extern void refclock_probe(float tvuseccorr, float cpumhz, int dis_rdtsc);
extern void refclock_init(void);
extern l1_time_t refclock_current(l1_time_t expected, int exp_valid);

/* --------------------------------------------------------------------- */
#endif /* _L1USER_H */
