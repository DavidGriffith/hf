/*****************************************************************************/

/*
 *      dcf77.h  --  DCF77 system parameters.
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

#ifndef _DCF77_H
#define _DCF77_H

/* --------------------------------------------------------------------- */

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

/* --------------------------------------------------------------------- */

#define SAMPLE_RATE  8000
/*#define CARRIER_FREQ 1000*/
#define CARRIER_FREQ 1000
#define PN_FREQ      77500/120
#define PN_PERIOD    120/77500
#define PN_MOD_IDX   10/360

/* --------------------------------------------------------------------- */

struct modulator {
	const char *name;
	void (*init)(unsigned int sample_rate);
	void (*modulator)(short *s, unsigned int n);
};

struct demodulator {
	const char *name;
	void (*init)(unsigned int sample_rate);
	void (*demodulator)(const short *s, unsigned int n);
};

/* --------------------------------------------------------------------- */

/* util.c */
extern void die(const char *func) __attribute__ ((noreturn));
extern void vlprintf(int vl, const char *fmt, ...);
extern unsigned int verboselevel;

/* audioout.c */
extern void audio_out(const char *name_audio, const struct modulator *mod);
extern void oss_nommap_audio_out(const char *name_audio, const struct modulator *mod);
extern void audio_stdout(const char *name_audio, const struct modulator *mod);

/* dcfmod.c */
extern const struct modulator dcf77_modulator;

/* hbgmod.c */
extern const struct modulator hbg_modulator;

/* timeenc.c */
extern int timeenc_getbit(void);
extern void timeenc_init(time_t curtime, unsigned int tz_mesz, unsigned int ant,
			 unsigned int ann_tz, unsigned int ann_leapsec);

/* calccorr.c */
#define INVALID_TIME ((time_t)-1)
extern void tc_init(unsigned int sample_rate);
extern void tc_start(void);
extern void tc_now(unsigned int minus_samples);
extern void tc_advance_samples(unsigned int s);
extern void tc_minute(time_t curtime, unsigned int samples);
void output();

/* audioin.c */
extern void audio_in(const char *name_audio, const struct demodulator *dem);
extern void oss_nommap_audio_in(const char *name_audio, const struct demodulator *dem);
extern void audio_stdin(const char *name_audio, const struct demodulator *dem);

/* timedec.c */
extern void time_decode(unsigned long long bits, unsigned int samples);

/* dcfdemod.c */
extern const struct demodulator dcf77_demodulator_ampl;

/* dcfdemodpn.c */
extern const struct demodulator dcf77_demodulator_pn;

/* hbgdemod.c */
extern const struct demodulator hbg_demodulator;

/* --------------------------------------------------------------------- */

/* dcf77gen.c and dcf77rx.c */
extern int lsb_mode;
extern int use_mmap;
extern int set_time;

/* --------------------------------------------------------------------- */

#define COSTAB_BITS  10
#define COSTAB_SIZE  (1<<COSTAB_BITS)
#define COSTAB_MASK  (COSTAB_SIZE-1)

#define COS(x)       (costab[((x) >> (24-COSTAB_BITS)) & COSTAB_MASK])
#define SIN(x)       (COS((x)+0xc00000))

extern const short costab[COSTAB_SIZE];

/* --------------------------------------------------------------------- */

/*
 * why store the PN sequence as table?
 * because:
 * a) the table is small, only 16 words = 64 bytes = 512 bits
 * b) the table is much easier to advance by more than one chip
 */

#define PN_LENGTH       512
#define PN_LOGWORDSIZE  5

#define PN(x)           ((pn_tab[((x)&(PN_LENGTH-1))>>PN_LOGWORDSIZE]>>((x)&((1<<PN_LOGWORDSIZE)-1)))&1)

extern const unsigned int pn_tab[PN_LENGTH>>PN_LOGWORDSIZE];

/* --------------------------------------------------------------------- */
/*
 * misc utility functions
 */

extern __inline__ unsigned int hweight32(unsigned int w);
extern __inline__ unsigned int hweight16(unsigned short w);
extern __inline__ unsigned int hweight8(unsigned char w);

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int zero_run_length(unsigned int bits);

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int dcf_pn_advance(unsigned int *st);

/* --------------------------------------------------------------------- */

extern __inline__ int ld2(unsigned int x);

/* --------------------------------------------------------------------- */
#endif /* _DCF77_H */



