/*****************************************************************************/

/*
 *      testcos.c  --  test cosine implementation.
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

#include <stdlib.h> 
#include <stdio.h>
#include <math.h>

/* --------------------------------------------------------------------- */

#define SINTABBITS  9
#define SINTABSIZE  (1<<SINTABBITS)

static float sintab[SINTABSIZE+SINTABSIZE/4];
static short isintab[SINTABSIZE+SINTABSIZE/4];

/* --------------------------------------------------------------------- */

static /*__inline__*/ float ftblcos(unsigned int arg)
{
	unsigned int x;
	int dx;
	float s, c;

	x = (arg + (0x8000 >> SINTABBITS)) & (0xffffu & (0xffffu << (16-SINTABBITS)));
	dx = arg - x;
	x >>= (16-SINTABBITS);
	c = sintab[x+(0x4000 >> (16-SINTABBITS))];
	s = sintab[x];
	return c - s * (float)(M_PI/32768.0) * dx;
}

/* --------------------------------------------------------------------- */

static /*__inline__*/ short itblcos(unsigned int arg)
{
	unsigned int x;
	int dx;
	int s, c;

	x = (arg + (0x8000 >> SINTABBITS)) & (0xffffu & (0xffffu << (16-SINTABBITS)));
	dx = arg - x;
	x >>= (16-SINTABBITS);
	c = isintab[x+(0x4000 >> (16-SINTABBITS))];
	s = isintab[x];
	return c - ((s * dx * (int)(M_PI*64.0)) >> 21);
}

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
	float relerr = 0, abserr = 0, irelerr = 0, iabserr = 0, f1, f2, f3, f4;
	int i;

	for (i = 0; i < (SINTABSIZE+SINTABSIZE/4); i++)
		isintab[i] = 32767.0 * (sintab[i] = sin(2.0 * M_PI / SINTABSIZE * i));
	for (i = 0; i < 65536; i++) {
		f1 = cos(M_PI / 32768.0 * i);
		f2 = ftblcos(i);
		f3 = fabs(f1-f2);
		if (f1 != 0)
			f4 = f3 / f1;
		else
			f4 = 0;
		if (f3 > abserr)
			abserr = f3;
		if (f4 > relerr)
			relerr = f4;
		f2 = itblcos(i) / 32767.0;
		f3 = fabs(f1-f2);
		if (f1 != 0)
			f4 = f3 / f1;
		else
			f4 = 0;
		if (f3 > iabserr)
			iabserr = f3;
		if (f4 > irelerr)
			irelerr = f4;
	}
	printf("maximum relative error: %10.5f  maximum absolute error: %10.8f\n", relerr, abserr);
	printf("integer version: maximum relative error: %10.5f  maximum absolute error: %10.8f\n", 
	       irelerr, iabserr);
	exit(0);
}
