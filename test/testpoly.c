#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h> 
#include <stdio.h>

#define POLY_HI (1<<9)
#define POLY_LO ((1<<4)|(1<<0))
#define POLY (POLY_HI|POLY_LO)
#define POLY_REV ((1<<0)|(1<<5)|(1<<9))

#define POLY_MASK 0x1ff

extern __inline__ unsigned int hweight32(unsigned int w) 
{
        unsigned int res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
        res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
        res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
        res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
        return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
}

int main(int argc, char *argv[])
{
	unsigned int divst, mulst = 0, cnt = 0;

	divst = 1;
	do {
		divst <<= 1;
		mulst <<= 1;
		if (divst & POLY_HI) {
			divst ^= POLY_LO;
			mulst |= 1;
		}
		divst &= POLY_MASK;
		cnt++;
		printf("Count: %d State: %4d  Out: %c Mul: %c  Weight: %d\n", cnt, divst, '0'+(mulst & 1), 
		       '0'+(hweight32(mulst & POLY_REV) & 1), hweight32(mulst & 0xfff));
	} while (cnt < 10000 && divst != 1);
	exit(0);
}
