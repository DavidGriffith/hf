/*
 *    mt63hflink.h 
 *    
 *    by Günther Montag 2004
 *
 *    functions to link the MT63 transmitter and receiver in C++ for LINUX
 *    by Pawel Jalocha, SP9VRC
 *    to 
 *    hf by Tom Sailer 
 *    This file is c code and _not_ in the libmt63.a
 *
 *    hf and MT63 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    hf and MT63 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with MT63; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef MT63HFLINK_H
#define MT63HFLINK_H

#include "mt63hf.h"
#include "fskl1.h"

void mt63_set_params(unsigned int bandwidth, unsigned int integration, 
    const unsigned char* cwcall, unsigned int doubleinterleave);
void l1_mt63_input_samples(short *samples, unsigned int nsamples);
int l1_mt63_output_samples(l1_time_t tstart, l1_time_t tinc,
	short *samples, unsigned int nsamples);

void *mode_mt63_rx(void *dummy);
void *mode_mt63_tx(void *dummy);

void mt63_rx_test();
void mt63_tx_test();

struct mt63p {
    unsigned int bandwidth;
    unsigned int integration;
    unsigned char cwcall[10];
    unsigned int doubleinterleave;
};
extern struct mt63p mt63p;
#define MT63RXBUFLEN 4096
#define MT63TXBUFLEN 4096
extern short mt63rxbuf[MT63RXBUFLEN];
extern short mt63txbuf[MT63TXBUFLEN];
extern int mt63rxbuf_written, mt63rxbuf_read;
extern int mt63txbuf_written, mt63txbuf_read;
extern pthread_mutex_t mt63_mut;
extern pthread_cond_t  mt63_cond;

#endif /* MT63HFLINK_H */
