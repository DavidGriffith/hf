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

extern unsigned int hweight32(unsigned int w);
extern unsigned int hweight16(unsigned short w);
extern unsigned int hweight8(unsigned char w);

/* --------------------------------------------------------------------- */

extern void soft_to_hard(const int *inp, unsigned char *out, 
				    int nsamp, int spacing, int inv);

/* --------------------------------------------------------------------- */

extern void invert(unsigned char *c, int len);

/* --------------------------------------------------------------------- */

extern long soft_time_dev(l1_soft_t *inp, int nbits, 
			int spacing, l1_time_t tinc);

/* --------------------------------------------------------------------- */

extern int freq_tracking(l1_soft_t *inmid, l1_soft_t *inlo, 
				    l1_soft_t *inhi, int nbits,
				    int spacing, l1_soft_t *valmid, 
				    l1_soft_t *vallo, l1_soft_t *valhi);

/* --------------------------------------------------------------------- */

extern const unsigned short crc_ccitt_table[256];

extern unsigned int calc_crc_ccitt(unsigned char *data, int len,
					      unsigned int preset);

extern unsigned int gtor_calc_crc_ccitt(unsigned char *data, 
						    int len, 
						   unsigned int preset);


/* --------------------------------------------------------------------- */
#endif /* _FSKUTIL_H */
