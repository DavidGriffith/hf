/*****************************************************************************/

/*
 *      dcfmod.c  --  Linux soundcard DCF77 signal generator (modulator).
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

#include <string.h>

#include "dcf77.h"

/* --------------------------------------------------------------------- */

#define FILTER

/* --------------------------------------------------------------------- */

static struct modulator_state {
	unsigned int car_inc;
	unsigned int pn_inc;
	unsigned int sec_inc;

	int cur_bit;
	int cur_pn;

	unsigned int ph_sec;
	unsigned int ph_car;
	unsigned int ph_pn;
	unsigned int pn_cnt;
	unsigned int pn_st;

#ifdef FILTER

#define FILTER_N 32
#define FILTER_MASK (FILTER_N-1)

	struct filter_state {
		int state[FILTER_N];
		unsigned int ptr;
	} f;
#endif /* FILTER */
} m;

/* --------------------------------------------------------------------- */

static void dcf_init(unsigned int sample_rate)
{
	memset(&m, 0, sizeof(m));
	m.car_inc = 0x1000000ULL * CARRIER_FREQ / sample_rate;
	m.pn_inc = 0x10000ULL * PN_FREQ / sample_rate;
	m.sec_inc = 0x1000000 / sample_rate;
}

/* --------------------------------------------------------------------- */

#ifdef FILTER

static const int filt_coeff[2*FILTER_N] = {
	465, 48, -33, -1295, -736, 221, -503, -1930,
       -2095, -684, 101, -1922, -4702, -3117, 4111, 11188,
       11188, 4111, -3117, -4702, -1922, 101, -684, -2095,
       -1930, -503, 221, -736, -1295, -33, 48, 465,
	465, 48, -33, -1295, -736, 221, -503, -1930,
       -2095, -684, 101, -1922, -4702, -3117, 4111, 11188,
       11188, 4111, -3117, -4702, -1922, 101, -684, -2095,
       -1930, -503, 221, -736, -1295, -33, 48, 465
};

extern __inline__ short calc_filter(short new)
{
	const int *c = filt_coeff + FILTER_N;
	int *s = m.f.state;
	int sum = 0;
	unsigned int u;

	m.f.state[m.f.ptr++] = new;
	m.f.ptr &= FILTER_MASK;
	c -= m.f.ptr;
	for (u = 0; u < FILTER_N; u++, c++, s++)
		sum += (*c) * (*s);
	sum >>= 16;
	return sum;
}

#endif /* FILTER */

/* --------------------------------------------------------------------- */

static void dcf_mod(short *s, unsigned int n)
{
	short sp;
	
	for (; n > 0; n--, s++) {
		if ((m.ph_sec < 0x1000000*1/10 && m.cur_bit == 0) || 
		    (m.ph_sec < 0x1000000*2/10 && m.cur_bit > 0))
			sp = COS(m.ph_car) >> 2;
		else if (m.ph_sec >= 0x1000000*2/10 && m.pn_cnt < PN_LENGTH) {
			sp = COS(m.ph_car + m.cur_pn);
			m.ph_pn += m.pn_inc;
			if (m.ph_pn >= 0x10000) {
				m.ph_pn &= 0xffff;
				m.pn_cnt++;
				m.cur_pn = ((m.cur_bit ^ PN(m.pn_cnt) ^ !lsb_mode) & 1) ? 
					0x1000000*PN_MOD_IDX : -0x1000000*PN_MOD_IDX;
			}
		} else
			sp = COS(m.ph_car);

#ifdef FILTER
		*s = calc_filter(sp);
#else /* FILTER */
		*s = sp;
#endif /* FILTER */
		m.ph_car = (m.ph_car + m.car_inc) & 0xffffffu;
		m.ph_sec += m.sec_inc;
		if (m.ph_sec >= 0x1000000) {
			m.ph_sec &= 0xffffffu;
			m.pn_st = 0;
			m.pn_cnt = 0;
			m.ph_pn = 0;
			m.cur_bit = timeenc_getbit();
			m.cur_pn = ((m.cur_bit ^ PN(0) ^ !lsb_mode) & 1) ? 
				0x1000000*PN_MOD_IDX : -0x1000000*PN_MOD_IDX;
		}
	}
}

/* --------------------------------------------------------------------- */

const struct modulator dcf77_modulator = { "DCF77 modulator", dcf_init, dcf_mod };

/* --------------------------------------------------------------------- */
