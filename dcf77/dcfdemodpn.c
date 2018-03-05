/*****************************************************************************/

/*
 *      dcfdemodpn.c  --  Linux soundcard DCF77 receiver, PN demodulator code.
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

#include "dcf77.h"
#include "xgintf.h"

/* --------------------------------------------------------------------- */

#define INF_N 12
#define SRCH_N 32  /* should divide the PN sequence length */

static struct dcf_state {
	unsigned int sample_rate;

	struct {
		int inf_i[INF_N];
		int inf_q[INF_N];
		unsigned int inf_ptr;

		unsigned int ph_car;
		int car_fcorr;
	} f;

	/*
	 * PN sequence searcher
	 */
	struct {
		unsigned int scnt;
		unsigned int sphase;
		int aacc[SRCH_N];
		int sacc[SRCH_N];		
	} s;

	/* early/late gate */
	struct {
		unsigned int aacc;
		int sacc[3];
		unsigned int sphase[3];
		unsigned int scnt[3];
	} o;

	struct {
		unsigned int car_inc, pn_inc, sec_inc;

		unsigned int lock_cnt;

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

extern __inline__ void decode_pn_bit(unsigned int bit, unsigned int samples)
{
	d.t.dhist <<= 1;
	bit &= 1;
	d.t.dhist |= bit;
	vlprintf(1, "Time: Bit %c  cnt: %2d\n", '0'+bit, d.t.dcnt);
	if ((d.t.dhist & 0x3ff) == 0x3ff) {
		d.t.dbits = 0x3ff;
		d.t.dcnt = 10;
		return;
	}
	if (bit)
		d.t.dbits |= 1ULL << d.t.dcnt;
	d.t.dcnt++;
	if (d.t.dcnt == 60)
		time_decode(d.t.dbits, samples);
}

/* --------------------------------------------------------------------- */

static void trk_init(void)
{
	memset(&d.o, 0, sizeof(d.o));
}

/* --------------------------------------------------------------------- */

static void srch_init(void)
{
	unsigned int u;

	memset(&d.s, 0, sizeof(d.s));
	for (u = 0; u < SRCH_N; u++)
		d.s.aacc[u] = 0x7fffff;
}

/* --------------------------------------------------------------------- */

#define PN_STARTSEQ        (0x40000000ULL*2/10)
#define PN_SEQINC          (0x40000000ULL*PN_PERIOD)
#define PN_TRKTAU          (PN_SEQINC/3)
#define PN_TRKADJ          (PN_SEQINC/8)
	
extern __inline__  void trk_sample(int sq, unsigned int samples)
{
	if (d.d.sec_ph > 0x3fffffff) {
		trk_init();
		d.d.sec_ph &= 0x3fffffff;
	}

	if (d.d.sec_ph >= PN_STARTSEQ-PN_TRKTAU && d.o.scnt[0] < PN_LENGTH) {
		d.o.sphase[0] += d.d.pn_inc;
		if (d.o.sphase[0] >= 0x10000) {
			d.o.sphase[0] &= 0xffff;
			if (PN(d.o.scnt[0]))
				d.o.sacc[0] += sq;
			else
				d.o.sacc[0] -= sq;
			d.o.scnt[0]++;
		}
	}
	if (d.d.sec_ph >= PN_STARTSEQ && d.o.scnt[1] < PN_LENGTH) {
		d.o.sphase[1] += d.d.pn_inc;
		if (d.o.sphase[1] >= 0x10000) {
			d.o.sphase[1] &= 0xffff;
			if (PN(d.o.scnt[1]))
				d.o.sacc[1] += sq;
			else
				d.o.sacc[1] -= sq;
			d.o.scnt[1]++;
			d.o.aacc += abs(sq);
		}
	}
	if (d.d.sec_ph >= PN_STARTSEQ+PN_TRKTAU && d.o.scnt[2] < PN_LENGTH) {
		d.o.sphase[2] += d.d.pn_inc;
		if (d.o.sphase[2] >= 0x10000) {
			d.o.sphase[2] &= 0xffff;
			if (PN(d.o.scnt[2]))
				d.o.sacc[2] += sq;
			else
				d.o.sacc[2] -= sq;
			d.o.scnt[2]++;
			if (d.o.scnt[2] == PN_LENGTH) {
				vlprintf(2, "PN Tracking: %10d %10d %10d %10d\n",
					 d.o.sacc[0], d.o.sacc[1], d.o.sacc[2], d.o.aacc);
				decode_pn_bit((d.o.sacc[1] < 0) ^ (!lsb_mode), samples);
				if (abs(d.o.sacc[1]) > d.o.aacc>>1) {
					d.d.lock_cnt++;
					if (d.d.lock_cnt >= 8)
						d.d.lock_cnt = 8;
				} else {
					d.d.lock_cnt--;
					if (d.d.lock_cnt <= 0)
						srch_init();
				}
					if (abs(d.o.sacc[0]) > abs(d.o.sacc[2]))
						d.d.sec_ph += PN_TRKADJ;
					else
						d.d.sec_ph -= PN_TRKADJ;
#if 0
				if (abs(d.o.sacc[0]) > abs(d.o.sacc[1]) ||
				    abs(d.o.sacc[2]) > abs(d.o.sacc[1])) {
				}
#endif
				trk_init();
			}
		}
	}
}

/* --------------------------------------------------------------------- */

extern __inline__  void srch_sample(int sq)
{
	unsigned int u, k;
	int asq = abs(sq);

	d.d.sec_ph &= 0x3fffffff;

	d.s.sphase += d.d.pn_inc;
	if (d.s.sphase < 0x10000)
		return;
	d.s.sphase &= 0xffffu;
	asq = abs(sq);
	for (u = 0, k = d.s.scnt; u < SRCH_N; u++, k = (k + PN_LENGTH/SRCH_N) % PN_LENGTH) {
		if (k == 0) {
			vlprintf(2, "Searcher %3u: accumulated: %10d  pn: %10d\n",
				 u, d.s.aacc[u], d.s.sacc[u]);
			if (abs(d.s.sacc[u]) >= d.s.aacc[u] >> 1) {
				vlprintf(2, "PN sequence lock at secph 0x%08x\n", d.d.sec_ph);
				d.d.lock_cnt = 4;
				d.d.sec_ph = PN_STARTSEQ+PN_SEQINC*(PN_LENGTH+1);
				trk_init();
			}
			d.s.sacc[u] = d.s.aacc[u] = 0;
		}
		d.s.aacc[u] += asq;
		if (PN(k))
			d.s.sacc[u] += sq;
		else
			d.s.sacc[u] -= sq;
	}
	d.s.scnt = (d.s.scnt + 1) % PN_LENGTH;
	if (!d.s.scnt)
		d.s.sphase += d.d.pn_inc/4;  /* advance phase slightly */
}


/* --------------------------------------------------------------------- */

static void demod_pn(const short *s, unsigned int n)
{
	unsigned int samples;
	int si, sq;
	unsigned int u;
	int i;
	static int rodcnt = 0;
	char* rod = "|/-\\ ";

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
				d.f.ph_car -= 256;
				d.f.car_fcorr -= 1;
			} else {
				d.f.ph_car += 256;
				d.f.car_fcorr += 1;
			}
		}
		d.f.ph_car = (d.f.ph_car + d.d.car_inc + d.f.car_fcorr) & 0xffffffu;
#if 0
		xg_val(0, *s, "in");
		xg_next(32768, "DCF77 input signal");
#endif
#if 0
		xg_val(0, si, "I");
		xg_val(1, sq, "Q");
		xg_next(32768, "DCF77 phase recovery");
#endif
		if (d.d.lock_cnt > 0)
			trk_sample(sq, samples);
		else
			srch_sample(sq);
		d.d.sec_ph += d.d.sec_inc;
		if (d.d.sec_ph >= 0x40000000) {
			vlprintf(2, "Carrier frequency offset: %6.3fHz   Sec freq offset: %6.1fms/s\n", 
				d.f.car_fcorr * ((float)d.sample_rate / (float)0x1000000),
				d.d.sec_fcorr * (1000.0 / 0x40000000));
			if (verboselevel < 2) {
			    printf("\t\t\t\t\t\t\t\t%c\r", rod[rodcnt]);
			    fflush(stdout); // without this, printf does not printf!!
			    rodcnt++;
			    if (rodcnt > 3) rodcnt = 0;
			}
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

static void dem_init(unsigned int sample_rate)
{
	memset(&d, 0, sizeof(d));
	d.sample_rate = sample_rate;
	d.d.car_inc = 0x1000000ULL * CARRIER_FREQ / sample_rate;
	d.d.pn_inc = 0x10000ULL * PN_FREQ / sample_rate;
	d.d.sec_inc = 0x40000000UL / sample_rate;
	srch_init();
}

/* --------------------------------------------------------------------- */

const struct demodulator dcf77_demodulator_pn = { 
	"DCF77 demodulator pn",	dem_init, demod_pn 
};

/* --------------------------------------------------------------------- */
