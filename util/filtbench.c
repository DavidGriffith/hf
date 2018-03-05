/*****************************************************************************/

/*
 *      filtbench.c  --  "benchmark" several filter implementations.
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
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>
#ifdef HAVE_SYS_IO_H
#include <sys/io.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/* --------------------------------------------------------------------- */

#if defined(__i386__) && defined(HAVE_SYS_IO_H)

typedef unsigned int bench_state_t;

extern __inline__ void bench_init(void)
{
	if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
		perror("mlockall");
		exit(1);
	}
	if (iopl(3)) {
		perror("iopl");
		exit(1);
	}
}

extern __inline__ void bench_start(bench_state_t *s)
{
	unsigned int t, t2;

	__asm__ __volatile__ ("cli;\n\t"
			      "rdtsc;\n\t" : "=a" (t), "=d" (t2));
	*s = t;
}

extern __inline__ unsigned int bench_end(bench_state_t *s)
{
	unsigned int t, t2;

	__asm__ __volatile__ ("rdtsc;\n\t"
			      "sti;\n\t" : "=a" (t), "=d" (t2));
	return t - *s;	
}

#else

typedef unsigned int bench_state_t;

extern __inline__ void bench_init(void)
{
}

extern __inline__ void bench_start(bench_state_t *s)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	*s = tv.tv_usec;
}

extern __inline__ unsigned int bench_end(bench_state_t *s)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (1000000UL + tv.tv_usec - *s) % 1000000UL;
}

#endif
/* --------------------------------------------------------------------- */

#define FILTLEN     80
#define INBUFSZ     160
#define FILTSPACING 10

/* --------------------------------------------------------------------- */

extern __inline__ float filt1(short *in, float *coeff)
{
	unsigned int u;
	float acc = 0;
	
	for (u = 0; u < FILTLEN; u++, in++, coeff++)
		acc += (*in) * (*coeff);
	return acc * (2.0 / FILTLEN);
}

/* --------------------------------------------------------------------- */

extern __inline__ int filt2(short *in, int *coeff)
{
	unsigned int u;
	long long acc = 0;
	
	for (u = 0; u < FILTLEN; u++, in++, coeff++)
		acc += (*in) * (*coeff);
	acc >>= 15;
	acc *= (0x20000 / FILTLEN);
	return acc >> 16;
}

/* --------------------------------------------------------------------- */

#ifdef __i386__

static int filt3(short *in, int *coeff)
{
	int res = FILTLEN;

/* tnx for correction by Micha!! gave gcc.3.2. error. old: */	
/*	__asm__ __volatile__ ("xorl %%ebx,%%ebx\n\t"
			      "xorl %%ecx,%%ecx\n\t" 
			      "\n1:\n\t"
			      "movswl (%%esi),%%eax\n\t"
			      "imull (%%edi)\n\t"
			      "addl $2,%%esi\n\t"
			      "addl $4,%%edi\n\t"
			      "addl %%eax,%%ebx\n\t"
			      "adcl %%edx,%%ecx\n\t" 
			      "decl %1\n\t"
			      "jnz 1b\n\t" 
			      "shrdl $15,%%ecx,%%ebx\n\t"
			      "# sarl $15,%%ecx ! high part unneccessary\n\t" 
			      "movl %2,%%eax\n\t"
			      "imull %%ebx\n\t" 
			      "movl %%edx,%%eax\n\t"
			      : "=&a" (res) 
			      : "o" (res), "i" (0x200000000ULL / FILTLEN), "S" (in), "D" (coeff)
			      : "ax", "bx", "cx", "dx", "si", "di");
*/

/* new, by Micha Matz, thanks very much !!!!*/
	__asm__ __volatile__ ("xorl %%ebx,%%ebx\n\t"
			      "xorl %%ecx,%%ecx\n\t" 
			      "pushl %%eax\n\t"
			      "\n1:\n\t"
			      "movswl (%%esi),%%eax\n\t"
			      "imull (%%edi)\n\t"
			      "addl $2,%%esi\n\t"
			      "addl $4,%%edi\n\t"
			      "addl %%eax,%%ebx\n\t"
			      "adcl %%edx,%%ecx\n\t" 
			      "decl (%%esp)\n\t"
			      "jnz 1b\n\t" 
			      "shrdl $15,%%ecx,%%ebx\n\t"
			      "# sarl $15,%%ecx ! high part unneccessary\n\t" 
			      "popl %%eax\n\t"
			      "movl %3,%%eax\n\t"
			      "imull %%ebx\n\t" 
			      "movl %%edx,%%eax\n\t"
			      : "+a" (res), "+S"(in), "+D"(coeff) 
			      : "i" (0x200000000ULL / FILTLEN)
			      : "bx", "cx", "dx");

	return res;
}

#else

static int filt3(short *in, int *coeff)
{
	return 0;
}

#endif

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
	short inbuf[INBUFSZ];
	float fcoeff[FILTLEN];
	int icoeff[FILTLEN];
	float res1[(INBUFSZ-FILTLEN)/FILTSPACING];
	int res2[(INBUFSZ-FILTLEN)/FILTSPACING];
	int res3[(INBUFSZ-FILTLEN)/FILTSPACING];
	unsigned int time1, time2, time3;

	bench_state_t bst;
	unsigned int u;
	short *ip;

	fprintf(stderr, "FIR filter implementation benchmark by T. Sailer\n");
	bench_init();
	/*
	 * generate tables
	 */
	for (u = 0; u < FILTLEN; u++)
		icoeff[u] = 32767.0 * (fcoeff[u] = cos(M_PI * 2.0 * 1445.0 / 9600.0 * u));
	for (u = 0; u < INBUFSZ; u++)
		inbuf[u] = random();
	/*
	 * time filter 1
	 */
	bench_start(&bst);
	for (u = 0, ip = inbuf; u < (INBUFSZ-FILTLEN)/FILTSPACING; u++, ip += FILTSPACING)
		res1[u] = filt1(ip, fcoeff);
	time1 = bench_end(&bst);
	/*
	 * time filter 2
	 */
	bench_start(&bst);
	for (u = 0, ip = inbuf; u < (INBUFSZ-FILTLEN)/FILTSPACING; u++, ip += FILTSPACING)
		res2[u] = filt2(ip, icoeff);
	time2 = bench_end(&bst);
	/*
	 * time filter 3
	 */
	bench_start(&bst);
	for (u = 0, ip = inbuf; u < (INBUFSZ-FILTLEN)/FILTSPACING; u++, ip += FILTSPACING)
		res3[u] = filt3(ip, icoeff);
	time3 = bench_end(&bst);
	/*
	 * output results
	 */
	fprintf(stderr, "Results: Times: %6u %6u %6u\n", time1, time2, time3);
	for (u = 0; u < (INBUFSZ-FILTLEN)/FILTSPACING; u++)
		fprintf(stderr, "Res[%02d]: %5d %5d %5d\n", u, (int)res1[u], res2[u], res3[u]);
	exit(0);
}
