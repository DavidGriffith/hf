/*****************************************************************************/

/*
 *      testgaussgen.c  --  calculate tables for the channel simulator.
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
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "chan_tables.h"

/* --------------------------------------------------------------------- */

struct gaussgen_state {
	float rndv[GAUSSLPFLEN/GAUSSLPFOVER];
	float gval, gvalold;
	unsigned int rndcnt;
	unsigned int boxcnt, boxmax;
	float boxgcorr;
};

/* --------------------------------------------------------------------- */

static unsigned int myrand_seed = 1;

extern __inline__ unsigned int myrandom(void)
{
	unsigned int tmp2;
	unsigned long long tmp;

	if (!myrand_seed)
		return (myrand_seed = 1);
	tmp = myrand_seed * 16807ULL;
	tmp2 = (tmp & 0x7FFFFFFFU) + (tmp >> 31);
	if (tmp2 >= 0x7FFFFFFFU)
		tmp2 -= 0x7FFFFFFFU;
	return (myrand_seed = tmp2);
}

/* --------------------------------------------------------------------- */

/*
 * Approximation of a normally distributed random number generator
 * with unit variance
 *
 * division factor: randommax * sqrt(nrand / 12)
 * with nrand = 16 and randommax = 0x1000
 */

extern __inline__ float randn(void)
{
        int sum = 0, i;

        for (i = 0; i < 16; i++)
                sum += random() & 0xfff;
        return (sum - 0x8000) / 4729.7;
}

/* --------------------------------------------------------------------- */

static void gaussgen_init(struct gaussgen_state *gs, unsigned int fsdiv)
{
	memset(gs, 0, sizeof(*gs));
	gs->boxcnt = gs->boxmax = fsdiv / GAUSSLPF3DBFDIV;
	gs->boxgcorr = 1.0 / gs->boxmax;
}

/* --------------------------------------------------------------------- */

static float gaussgen_value(struct gaussgen_state *gs)
{
	const float *f1, *f2;
	float f;
	unsigned int u;

	if ((++gs->boxcnt) < gs->boxmax)
		return (gs->gval * gs->boxcnt + gs->gvalold * (gs->boxmax - gs->boxcnt)) * gs->boxgcorr;
	gs->boxcnt = 0;
	if (!gs->rndcnt) {
		memmove(gs->rndv+1, gs->rndv, sizeof(gs->rndv)-sizeof(gs->rndv[0]));
		gs->rndv[0] = randn();
	}
	f1 = gaussfilttbl[gs->rndcnt];
	gs->rndcnt = (gs->rndcnt + 1) % GAUSSLPFOVER;
	f2 = gs->rndv;
	for (f = u = 0; u < GAUSSLPFLEN/GAUSSLPFOVER; u++)
		f += (*f1++) * (*f2++);
	gs->gvalold = gs->gval;
	gs->gval = f;
	return gs->gvalold;
}

/* --------------------------------------------------------------------- */

struct gaussgen_state gs;

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
	float f;
	unsigned int i;

#if 0
	i = 0;
	while (myrandom() != 1) {
		i++;
		if (i & 0xfffff)
			continue;
		printf("Count: %10u\n", i);
	}
	printf("Count: %10u\n", i);
	exit(1);
#endif
	gaussgen_init(&gs, 800);
	for (i = 0; i < 1048576; i++) {
		f = gaussgen_value(&gs);
		if (fwrite(&f, sizeof(float), 1, stdout) != 1) {
			perror("fwrite");
			exit(1);
		}
	}
	exit(0);
}
