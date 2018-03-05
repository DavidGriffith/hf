/*****************************************************************************/

/*
 *      amtorsymsort.c  --  Generate symbol table.
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

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int hweight8(unsigned char w)
{
        unsigned short res = (w & 0x55) + ((w >> 1) & 0x55);
        res = (res & 0x33) + ((res >> 2) & 0x33);
        return (res & 0x0F) + ((res >> 4) & 0x0F);
}

/* --------------------------------------------------------------------- */

static char chsyms[35][8] = {
	"bbbyyyb",
	"ybyybbb",
	"bybbbyy",
	"bbyybyb",
	"ybbybyb",
	"bbybbyy",
	"bybybby",
	"byybybb",
	"bybbyyb",
	"bbbybyy",
	"ybbbbyy",
	"bybyybb",
	"byybbby",
	"byybbyb",
	"byyybbb",
	"bybbyby",
	"ybbbyby",
	"bybybyb",
	"bbybyyb",
	"yybybbb",
	"ybbbyyb",
	"yybbbby",
	"bbbyyby",
	"ybybbby",
	"bbybyby",
	"bbyyybb",
	"yyybbbb",
	"yybbybb",
	"ybybbyb",
	"ybbybby",
	"yybbbyb",
	"ybybybb", 
	"bbyybby", /* beta */
	"bbbbyyy", /* alpha */
	"ybbyybb" /* rq */
};

static char ttysyms[32][6] = {
	"zzaaa",
	"zaazz",
	"azzza",
	"zaaza",
	"zaaaa",
	"zazza",
	"azazz",
	"aazaz",
	"azzaa",
	"zzaza",
	"zzzza",
	"azaaz",
	"aazzz",
	"aazza",
	"aaazz",
	"azzaz",
	"zzzaz",
	"azaza",
	"zazaa",
	"aaaaz",
	"zzzaa",
	"azzzz",
	"zzaaz",
	"zazzz",
	"zazaz",
	"zaaaz",
	"aaaza",
	"azaaa",
	"zzzzz",
	"zzazz",
	"aazaa",
	"aaaaa"
};

struct symtab {
	unsigned int amtorsymb;
	unsigned int ttysymb;
};

static struct symtab symtab[35];

int main(int argc, char *argv[])
{
	int i, j, k;

	for (i = 0; i < 35; i++) {
		for (k = j = 0; j < 7; j++)
			if (chsyms[i][j] == 'b')
				k |= 1<<j;
			else if (chsyms[i][j] != 'y') {
				fprintf(stderr, "Error in Symbol %d, %s\n", i, chsyms[i]);
				exit(1);
			}
		symtab[i].amtorsymb = k;
		if (i >= 32) {
			symtab[i].ttysymb = i;
			continue;
		}
		for (k = j = 0; j < 5; j++)
			if (ttysyms[i][j] == 'z')
				k |= 1<<j;
			else if (ttysyms[i][j] != 'a') {
				fprintf(stderr, "Error in Symbol %d, %s\n", i, chsyms[i]);
				exit(1);
			}
		symtab[i].ttysymb = k;
	}
	for (i = 0; i < 35; i++) {
		printf("Symbol %2d: 0x%02x  0x%02x\n", i, symtab[i].amtorsymb, symtab[i].ttysymb);
		for (j = i+1; j < 35; j++) {
			if (hweight8(symtab[i].amtorsymb ^ symtab[j].amtorsymb) < 2)
				fprintf(stderr, "Symbols %d and %d: Hamming distance less than 2\n", i, j);
			if (i < 32 && j < 32 && symtab[i].ttysymb == symtab[j].ttysymb)
				fprintf(stderr, "Symbols %d and %d: tty symbols the same\n", i, j);
		}
	}
	printf("\n\nstatic const short amtor_check_table[128] = {\n\t");
	for (i = 0; i < 128; i++) {
		for (j = 0; j < 35 && symtab[j].amtorsymb != i; j++);
		printf("%2d", (j >= 35) ? -1 : symtab[j].ttysymb);
		if (i >= 127)
			printf("\n};\n\nstatic const unsigned char amtor_encode_table[35] = {\n\t");
		else if ((i & 15) != 15)
			printf(", ");
		else 
			printf(",\n\t");
	}
	for (i = 0; i < 35; i++) {
		for (j = 0; j < 35 && symtab[j].ttysymb != i; j++);
		if (j >= 35)
			exit(1);
		printf("0x%02x", symtab[j].amtorsymb);
		if (i >= 34)
			printf("\n};\n\n");
		else if ((i & 15) != 15)
			printf(", ");
		else 
			printf(",\n\t");
	}		
	exit(0);
}



