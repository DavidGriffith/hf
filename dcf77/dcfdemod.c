/*****************************************************************************/

/*
 *      dcfdemod.c  --  Linux soundcard DCF77 receiver, demodulator code.
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

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "dcf77.h"
#include "xgintf.h"

/* --------------------------------------------------------------------- */

#define INF_N 12

static struct dcf_state {
	unsigned int sample_rate;

	struct {
		int inf_i[INF_N];
		int inf_q[INF_N];
		unsigned int inf_ptr;

		unsigned int ph_car;
		int car_fcorr;
	} f;

	struct {
		unsigned int car_inc, sec_inc;

		union {
			struct {
			} pn;
			
			struct {
				unsigned int ampl;
				unsigned int amphist;
				unsigned int state;
				unsigned int decay_cnt;
			} a;
		} d;

		int sec_ph;
		int sec_fcorr;
	} d;

	struct {
		unsigned long long dbits;
		unsigned int dcnt;
		unsigned int dhist;
	} t;	

} d;

/* --------------------------------------------------------------------- */

static void dem_init(unsigned int sample_rate)
{
	memset(&d, 0, sizeof(d));
	d.sample_rate = sample_rate;
	d.d.car_inc = 0x1000000ULL * CARRIER_FREQ / sample_rate;
	d.d.sec_inc = 0x40000000UL / sample_rate;
}

/* --------------------------------------------------------------------- */

extern __inline__ void decode_ampl_bit(unsigned int bit, unsigned int samples)
{
	if (bit == 0) {
		if (d.t.dcnt >= 59)
			time_decode(d.t.dbits, samples);
		d.t.dbits = 0;
		d.t.dcnt = 0;
		return;
	}
	if (d.t.dcnt >= 60)
		return;
	if (bit == 2)
		d.t.dbits |= 1ULL << d.t.dcnt;
	d.t.dcnt++;
}

/* --------------------------------------------------------------------- */

extern __inline__ void dcf77_process_ampl(int si, unsigned int samples)
{
	static int rodcnt = 0;
	char* rod = "|/-\\ ";
	if ((++d.d.d.a.decay_cnt) >= 400) {
		d.d.d.a.decay_cnt = 0;
		d.d.d.a.ampl = (d.d.d.a.ampl * 4055) >> 12;
	}
	if (d.d.sec_ph >= 0x40000000) {
		d.d.sec_ph &= 0x3fffffff;
		d.d.sec_ph += d.d.sec_fcorr;
		d.d.d.a.state = 0;
	}
	if (si > 0 && si > d.d.d.a.ampl)
		d.d.d.a.ampl += (si - d.d.d.a.ampl) >> 4;
	d.d.d.a.amphist = (d.d.d.a.amphist << 1) | (d.d.d.a.amphist & 1);
	if (si > (3*d.d.d.a.ampl/4))
		d.d.d.a.amphist |= 1;
	else if (si < (d.d.d.a.ampl/2))
		d.d.d.a.amphist &= ~1;
	d.d.d.a.amphist &= 0xffffffffu;
	if (d.d.d.a.amphist == 0xffff0000u) {
		vlprintf(2, "second tick at: %08x\n", d.d.sec_ph);
		if (verboselevel < 2) {
		    printf("%c\r", rod[rodcnt]);
		    fflush(stdout); // without this, printf does not printf!!
		    rodcnt++;
		    if (rodcnt > 3) rodcnt = 0;
		}
		if (((d.d.sec_ph - (0x40000000 / SAMPLE_RATE * 16)) & 0x3fffffff) < 0x20000000) {
			d.d.sec_ph -= 0x400000;
			d.d.sec_fcorr -= 0x10;
		} else {
			d.d.sec_ph += 0x400000;
			d.d.sec_fcorr += 0x10;
		}
	}
	if (!d.d.d.a.state && (d.d.d.a.amphist & 3) == 1) {
		if (d.d.sec_ph >= 0x40000000/100*5 && d.d.sec_ph <= 0x40000000/100*15)
			d.d.d.a.state = 1;
		else if (d.d.sec_ph >= 0x40000000/100*15 && d.d.sec_ph <= 0x40000000/100*25)
			d.d.d.a.state = 2;
	}
	if (d.d.d.a.state < 4 && d.d.sec_ph >= 0x40000000/100*30) {
		decode_ampl_bit(d.d.d.a.state, samples);
		d.d.d.a.state = 4;
	}
}

/* --------------------------------------------------------------------- */

static void demod_ampl(const short *s, unsigned int n)
{
	unsigned int samples;
	int si, sq;
	unsigned int u;
	int i;

	for (samples = 0; samples < n; samples++, s++) {
		d.f.inf_i[d.f.inf_ptr] = (COS(d.f.ph_car) * (*s)) >> 15;
		d.f.inf_q[d.f.inf_ptr] = (SIN(d.f.ph_car) * (*s)) >> 15;
		d.f.inf_ptr = (d.f.inf_ptr + 1) % INF_N;
		for (si = sq = u = 0; u < INF_N; u++) {
			si += d.f.inf_i[u];
			sq += d.f.inf_q[u];
		}
		if (abs(sq) > (abs(si) >> 8)) { 
			if (sq > 0) {
				d.f.ph_car -= 32768;
				d.f.car_fcorr -= 1;
			} else {
				d.f.ph_car += 32768;
				d.f.car_fcorr += 1;
			}
		}
		d.f.ph_car = (d.f.ph_car + d.d.car_inc + d.f.car_fcorr) & 0xffffffu;
#if 0
		xg_val(0, si, "I");
		xg_val(1, sq, "Q");
		xg_val(2, d.d.d.a.ampl, "Ampl");
		xg_next(32768, "DCF77 phase recovery");
#endif
		dcf77_process_ampl(si, samples);	
		d.d.sec_ph += d.d.sec_inc;
		if (d.d.sec_ph >= 0x40000000) {
			vlprintf(2, "Carrier frequency offset: %6.3fHz   Sec freq offset: %6.1fms/s\n", 
				d.f.car_fcorr * ((float)d.sample_rate / (float)0x1000000),
				d.d.sec_fcorr * (1000.0 / 0x40000000));
		}
	}
	i = d.d.car_inc >> 8;
	if (abs(d.f.car_fcorr) >= i) {
		if (d.f.car_fcorr > 0)
			d.f.car_fcorr = i;
		else
			d.f.car_fcorr = -i;
	}
	if (abs(d.d.sec_fcorr) > 0x100000) {
		if (d.d.sec_fcorr > 0)
			d.d.sec_fcorr = 0x100000;
		else
			d.d.sec_fcorr = -0x100000;
	}
}

/* --------------------------------------------------------------------- */

const struct demodulator dcf77_demodulator_ampl = { "DCF77 demodulator ampl",
						    dem_init, demod_ampl };

/* --------------------------------------------------------------------- */
