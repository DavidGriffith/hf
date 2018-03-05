/*****************************************************************************/

/*
 *      pactorcrc.c  --  calculate pactor CRC preset from CRC result.
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
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "fskl1.h"
#include "fskutil.h"

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
	unsigned char buf[23];
	int i, j, k, l;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <crcresult> <length>\n", argv[0]);
		return 1;
	}
	j = strtoul(argv[1], NULL, 0) & 0xffff;
	k = strtoul(argv[2], NULL, 0);
	if (k <= 2 || k > sizeof(buf)) {
		fprintf(stderr, "length out of range\n");
		return 1;
	}
	for (i = 0; i < 0x10000; i++) {
		memset(buf, 0, k-2);
		l = calc_crc_ccitt(buf, k-2, i);
		buf[k-2] = l & 0xff;
		buf[k-1] = l >> 8;
		l = calc_crc_ccitt(buf, k, 0xffff);
		if (l == j)
			printf("CRC preset: 0x%04x\n", i);
		else if ((l ^ 0xffff) == j)
			printf("CRC inverted preset: 0x%04x\n", i);
	}
	return 0;
}
