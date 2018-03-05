/*****************************************************************************/

/*
 *      fskinlines.h  --  binary FSK modem code, utility inline functions.
 *
 *      Copyright (C) 1997  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *        Swiss Federal Institute of Technology (ETH), Electronics Lab
 *	
 * 	modified by Günther Montag dl4mge
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
#ifndef _FSKINLINES_H
#define _FSKINLINES_H


int brake = 0;
static unsigned short random_seed;

extern void errprintf(int severity, const char *fmt, ...);

extern __inline__ unsigned short random_num(void)
{
        random_seed = 28629 * random_seed + 157;
        return random_seed;
}

/* --------------------------------------------------------------------- */
/*
 * correlator cache routines
 */

extern __inline__ void cc_lock(unsigned int u)
{
	if (u >= CORRELATOR_CACHE)
		return;
	corr_cache[u].refcnt++;
}

extern __inline__ void cc_unlock(unsigned int u)
{
	if (u >= CORRELATOR_CACHE)
		return;
	if ((--corr_cache[u].refcnt) <= 0) {
		unsigned int i;

		for (i = 0; i < CORRELATOR_CACHE; i++) 
			if (corr_cache[i].lru < 32767)
				corr_cache[i].lru++;
		corr_cache[u].lru = 0;
		corr_cache[u].refcnt = 0;
	}
}

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int cc_lookup
    (unsigned short phinc0, unsigned short phinc1)
{
	unsigned int j;

	/* find correlator cache entry */
	for (j = 0; j < CORRELATOR_CACHE; j++) 
		if (corr_cache[j].phase_incs[0] == phinc0 &&
		    corr_cache[j].phase_incs[1] == phinc1)
			return j;
	return ~0;
}

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int cc_replace(void)
{
	unsigned int j, k = CORRELATOR_CACHE;
	int l = -1;

	for (j = 0; j < CORRELATOR_CACHE; j++)
		if (corr_cache[j].refcnt <= 0 && corr_cache[j].lru > l) {
			k = j;
			l = corr_cache[j].lru;
		}
	if (k < CORRELATOR_CACHE)
		return k;
	errprintf(SEV_WARNING, 
	    "modem: out of filter coefficient cache entries\n");
	return random_num() % CORRELATOR_CACHE;
}

/* --------------------------------------------------------------------- */

extern __inline__ int isimplecos(unsigned int arg)
{
	return isintab[((arg+0x4000) >> (16-SINTABBITS)) & (SINTABSIZE-1)];
}

/* --------------------------------------------------------------------- */

extern __inline__ int isimplesin(unsigned int arg)
{
	return isintab[(arg >> (16-SINTABBITS)) & (SINTABSIZE-1)];
}

/* --------------------------------------------------------------------- */

extern __inline__ int itblcos(unsigned int arg)
{
	unsigned int x;
	int dx;
	int s, c;

	x = (arg + (0x8000 >> SINTABBITS)) & 
	    (0xffffu & (0xffffu << (16-SINTABBITS)));
	dx = arg - x;
	x >>= (16-SINTABBITS);
	c = isintab[x+(0x4000 >> (16-SINTABBITS))];
	s = isintab[x];
	return c - ((s * dx * (int)(M_PI*64.0)) >> 21);
}

/* --------------------------------------------------------------------- */

extern __inline__ void itblcossin(unsigned int arg, int *cos, int *sin)
{
	unsigned int x;
	int dx;
	int s, c;

	x = (arg + (0x8000 >> SINTABBITS)) & 
	    (0xffffu & (0xffffu << (16-SINTABBITS)));
	dx = arg - x;
	x >>= (16-SINTABBITS);
	c = isintab[x+(0x4000 >> (16-SINTABBITS))];
	s = isintab[x];
	*cos = c - ((s * dx * (int)(M_PI*64.0)) >> 21);
	*sin = s + ((c * dx * (int)(M_PI*64.0)) >> 21);
}

/* --------------------------------------------------------------------- */

#ifdef MODEM_FLOAT

extern __inline__ float fcorr(int n, const float *coeff, const short *inp)
{
	float sum = 0;
	int i;

	for (i = n; i > 0; i--, coeff++, inp--)
		sum += (*coeff) * (*inp);
	return sum;
}

/* --------------------------------------------------------------------- */

extern __inline__ float fsqr(float x) __attribute__ ((const));

extern __inline__ float fsqr(float x)
{
	return x*x;
}

/* --------------------------------------------------------------------- */

extern __inline__ l1_soft_t do_filter(struct l1rxslots *slot, short *s)
{
	unsigned int cc = slot->corr_cache;

	if (cc >= CORRELATOR_CACHE) {
		printf("do_filter: correlator cache index overrange\n");
		return 0;
	}
	return (fsqr(fcorr(slot->corrlen, corr_cache[cc].correlator[1][0], s)) +
		fsqr(fcorr(slot->corrlen, corr_cache[cc].correlator[1][1], s)) -
		fsqr(fcorr(slot->corrlen, corr_cache[cc].correlator[0][0], s)) -
		fsqr(fcorr(slot->corrlen, corr_cache[cc].correlator[0][1], s))) 
		* slot->scale;
}

/* --------------------------------------------------------------------- */

extern __inline__ float ftblcos(unsigned int arg)
{
	unsigned int x;
	int dx;
	float s, c;

	x = (arg + (0x8000 >> SINTABBITS)) & 
	    (0xffffu & (0xffffu << (16-SINTABBITS)));
	dx = arg - x;
	x >>= (16-SINTABBITS);
	c = fsintab[x+(0x4000 >> (16-SINTABBITS))];
	s = fsintab[x];
	return c - s * (float)(M_PI/32768.0) * dx;
}

/* --------------------------------------------------------------------- */

extern __inline__ void ftblcossin(unsigned int arg, float *cos, float *sin)
{
	unsigned int x;
	int dx;
	float s, c;

	x = (arg + (0x8000 >> SINTABBITS)) & 
	    (0xffffu & (0xffffu << (16-SINTABBITS)));
	dx = arg - x;
	x >>= (16-SINTABBITS);
	c = fsintab[x+(0x4000 >> (16-SINTABBITS))];
	s = fsintab[x];
	*cos = c - s * (float)(M_PI/32768.0) * dx;
	*sin = s + c * (float)(M_PI/32768.0) * dx;
}

/* --------------------------------------------------------------------- */

static void cc_prepare
    (struct l1rxslots *slot, unsigned short phinc0, unsigned short phinc1)
{
	unsigned int j, k, l, ph, phinc;

	slot->scale = 1.0/32768.0/fsqr(slot->corrlen);

	j = cc_lookup(phinc0, phinc1);
	if (j >= CORRELATOR_CACHE) {
	    j = cc_replace();
	    /* calculate the correlator values */
	    printf("corr cache calc: %u  phases: 0x%04x 0x%04x\n", 
		j, phinc0, phinc1);
	    corr_cache[j].phase_incs[0] = phinc0;
	    corr_cache[j].phase_incs[1] = phinc1;
	    for (k = 0; k < 2; k++) {
		phinc = corr_cache[j].phase_incs[k];
		for (ph = l = 0; 
		     l < L1CORR_LEN; 
		     l++, ph = (ph + phinc) & 0xffff)
			ftblcossin
			    (ph, 
			    &corr_cache[j].correlator[k][0][l],
		    	    &corr_cache[j].correlator[k][1][l]);
		}
		corr_cache[j].valid = 1;
		corr_cache[j].refcnt = 0;
	}
	slot->corr_cache = j;
	cc_lock(j);
}

/* --------------------------------------------------------------------- */

#else /* MODEM_FLOAT */

#define SH1  8     /* min. ceil(log2(L1CORR_LEN)) - (31-(2*15-1)) */
#define SH2  (3*15-2*SH1)

#ifdef __i386__

extern __inline__ int icorr(int n, const int *coeff, const short *inp)
{
	int ret, rethi, tmp1 = 0, tmp2 = 0;

	__asm__("\n1:\n\t"
		"movswl (%0),%%eax\n\t"
		"imull (%1)\n\t"
		"subl $2,%0\n\t"
		"addl $4,%1\n\t"
		"addl %%eax,%3\n\t"
		"adcl %%edx,%4\n\t"
		"decl %2\n\t"
		"jne 1b\n\t"
		: "=&S" (inp), "=&D" (coeff), "=&c" (n), "=m" (tmp1), "=m" (tmp2)
		: "0" (inp), "1" (coeff), "2" (n)
		: "ax", "dx");
	__asm__("shrdl %2,%1,%0\n\t"
		"# sarl %2,%1\n\t"
		: "=&r" (ret), "=&r" (rethi)
		: "i" (SH1), "0" (tmp1), "1" (tmp2));


	return ret;
}

#else /* __i386__ */

extern __inline__ int icorr(int n, const int *coeff, const short *inp)
{
	long long sum = 0;
	int i;

	for (i = n; i > 0; i--, coeff++, inp--)
		sum += (*coeff) * (*inp);
	sum >>= SH1;
	return sum;
}

#endif /* __i386__ */

/* --------------------------------------------------------------------- */

extern __inline__ long long isqr(int x) __attribute__ ((const));

extern __inline__ long long isqr(int x)
{
	return ((long long)x) * ((long long)x);
}

/* --------------------------------------------------------------------- */

extern __inline__ l1_soft_t do_filter(struct l1rxslots *slot, short *s)
{
	
	unsigned int cc = slot->corr_cache;
        long long ll;
        int ret;
	
    static unsigned cnt = 0;

	if (cc >= CORRELATOR_CACHE) {
		printf("do_filter: correlator cache index overrange\n");
		return 0;
	}
	ll = isqr(icorr(slot->corrlen, corr_cache[cc].correlator[1][0], s)) +
		isqr(icorr(slot->corrlen, corr_cache[cc].correlator[1][1], s)) -
		isqr(icorr(slot->corrlen, corr_cache[cc].correlator[0][0], s)) -
		isqr(icorr(slot->corrlen, corr_cache[cc].correlator[0][1], s));
	ll >>= SH2;
	ret = (ll * slot->scale) >> 23;

	cnt++;
	cnt %= 512;
	if (!cnt){
		printf("hfkernel running: corrout: %6d  intermediate: %Ld\n", ret, ll);
		if (brake > 0) {
		    printf("xrun count reset: %d -> 0", brake);	
		    brake = 0;
		}
	}
       	return ret;
}

/* --------------------------------------------------------------------- */

static void cc_prepare
    (struct l1rxslots *slot, unsigned short phinc0, unsigned short phinc1)
{
	unsigned int j, k, l, ph, phinc;

	slot->scale = (1<<23) / (slot->corrlen*slot->corrlen);

	j = cc_lookup(phinc0, phinc1);
	if (j >= CORRELATOR_CACHE) {
	    j = cc_replace();
	    /* calculate the correlator values */
	    printf("corr cache calc: %u  phases: 0x%04x 0x%04x\n", 
		j, phinc0, phinc1);
	    corr_cache[j].phase_incs[0] = phinc0;
	    corr_cache[j].phase_incs[1] = phinc1;
	    for (k = 0; k < 2; k++) {
		phinc = corr_cache[j].phase_incs[k];
		for (ph = l = 0; l < L1CORR_LEN; l++, ph = (ph + phinc) & 0xffff)
		    itblcossin(
			ph, 
			&corr_cache[j].correlator[k][0][l],
			&corr_cache[j].correlator[k][1][l]);
		}
		corr_cache[j].refcnt = 0;
#if 0
		printk("corr: %u ph: 0x%04x 0x%04x\n", 
		    j, corr_cache[j].phase_incs[0], 
		    corr_cache[j].phase_incs[1]);
		for (l = 0; l < MAXCORRLEN; l++)
		    printf("corr: %6d %6d %6d %6d\n", 
			corr_cache[j].correlator[0][0][l],
			corr_cache[j].correlator[0][1][l], 
			corr_cache[j].correlator[1][0][l],
			corr_cache[j].correlator[1][1][l]);
#endif
	}
	slot->corr_cache = j;
	cc_lock(j);
}

#endif /* MODEM_FLOAT */
/* --------------------------------------------------------------------- */
#endif  /* _FSKINLINES_H */
