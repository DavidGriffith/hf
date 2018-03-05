/*****************************************************************************/

/*
 *      fskutil.h  --  hfkernel fsk codec functions
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
      
#ifndef _FSKUTIL_H
#define _FSKUTIL_H

/* --------------------------------------------------------------------- */

#include <string.h>

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int hweight32(unsigned int w) 
{
        unsigned int res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
        res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
        res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
        res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
        return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
}

extern __inline__ unsigned int hweight16(unsigned short w)
{
        unsigned short res = (w & 0x5555) + ((w >> 1) & 0x5555);
        res = (res & 0x3333) + ((res >> 2) & 0x3333);
        res = (res & 0x0F0F) + ((res >> 4) & 0x0F0F);
        return (res & 0x00FF) + ((res >> 8) & 0x00FF);
}

extern __inline__ unsigned int hweight8(unsigned char w)
{
        unsigned short res = (w & 0x55) + ((w >> 1) & 0x55);
        res = (res & 0x33) + ((res >> 2) & 0x33);
        return (res & 0x0F) + ((res >> 4) & 0x0F);
}

/* --------------------------------------------------------------------- */

extern __inline__ void soft_to_hard(const int *inp, unsigned char *out, 
				    int nsamp, int spacing, int inv)
{
	unsigned char mask = 1;
	unsigned char invm = inv ? 0 : 0xff;

	memset(out, 0, (nsamp + 7) >> 3);
	while (nsamp > 0) {
		*out |= ((*inp >> 31) ^ invm) & mask;
		mask <<= 1;
		if (mask == 0) {
			mask = 1;
			out++;
		}
		inp += spacing;
		nsamp--;
	}
}

/* --------------------------------------------------------------------- */

extern __inline__ void invert(unsigned char *c, int len)
{
	for (; len > 0; len--, c++)
		*c ^= 0xff;
}

/* --------------------------------------------------------------------- */

extern long soft_time_dev(l1_soft_t *inp, int nbits, 
			int spacing, l1_time_t tinc);

/* --------------------------------------------------------------------- */

extern __inline__ int freq_tracking(l1_soft_t *inmid, l1_soft_t *inlo, 
				    l1_soft_t *inhi, int nbits,
				    int spacing, l1_soft_t *valmid, 
				    l1_soft_t *vallo, l1_soft_t *valhi)
{
	l1_soft_t vm = 0, vl = 0, vh = 0;

	for (; nbits > 0; nbits--, inmid += spacing, inlo++, inhi++) {
		if (*inmid > 0) {
			vm += *inmid;
			vl += *inlo;
			vh += *inhi;
		} else {
			vm -= *inmid;
			vl -= *inlo;
			vh -= *inhi;
		}
	}
	*valmid = vm;
	*vallo = vl;
	*valhi = vh;
	if (vm > vl && vm > vh)
		return 0;
	if (vl > vh)
		return -1;
	return 1;
}

/* --------------------------------------------------------------------- */

extern const unsigned short crc_ccitt_table[256];

extern __inline__ unsigned int calc_crc_ccitt(unsigned char *data, int len,
					      unsigned int preset)
{
        unsigned int crc = preset;
        
        for(; len > 0; len--, data++)
                crc = ((crc >> 8) & 0xff) ^
                        crc_ccitt_table[(crc ^ *data) & 0xff];
        return (~crc) & 0xffff;
}


extern __inline__ unsigned int gtor_calc_crc_ccitt(unsigned char *data, 
						    int len, 
						   unsigned int preset)
{
        unsigned int crc = preset;
        
        for(; len > 2; len--, data++)
                crc = ((crc >> 8) & 0xff) ^
                        crc_ccitt_table[(crc ^ *data) & 0xff];
	crc = ((crc >> 8) & 0xff) ^ crc_ccitt_table[(crc ^ data[1]) & 0xff];
	crc = ((crc >> 8) & 0xff) ^ crc_ccitt_table[(crc ^ data[0]) & 0xff];
	return (~crc) & 0xffff;
}


/* --------------------------------------------------------------------- */
#endif /* _FSKUTIL_H */
