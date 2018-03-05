/*****************************************************************************/

/*
 *      mkgtortables.c  --  generate GTOR huffman and golay tables.
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
#include <ctype.h>
#include <stdlib.h>

/* --------------------------------------------------------------------- */

static const char *huff[128] = {
	"1111000011111000",      /* 0x00 */
	"1111000011111001",
	"1111000011111010",
	"1111000011111011",
	"1111000011111100",
	"1111000011111101",
	"1111000011111110",
	"1111000011111111",
	"1111000011110010",      /* 0x08 */
	"1111000011110011",
	"001101",
	"1111000011110100",
	"1111000011110101",
	"001100",
	"1111000011110110",
	"1111000011110111",
	"1111001110000000",      /* 0x10 */
	"1111001110000001",
	"1111001110000010",
	"1111001110000011",
	"1111001110000100",
	"1111001110000101",
	"1111001110000110",
	"1111001110000111",
	"1111001110001000",      /* 0x18 */
	"1111001110001001",
	"1111001110001010",
	"1111001110001011",
	"1111001110001100",
	"1111001110001101",
	"1111001110001110",
	"1111001110001111",
	"10",                    /* 0x20 */
	"11110011101",
	"110001101100",
	"0010100011011",
	"0001010111001",
	"110001101101",
	"111100111001",
	"110001101110",
	"110011011",             /* 0x28 */
	"110011100",
	"001010001100",
	"111100111110",
	"1100101",
	"00010101111",
	"1100100",
	"11110011110",
	"11000111",             /* 0x30 */
	"001010000",
	"0001011010",
	"0001011011",
	"0001011100",
	"0001010101",
	"0001011101",
	"0001011110",
	"0001011111",           /* 0x38 */
	"0001010010",
	"00101000111",
	"11110000110100",
	"0001010111010",
	"1111000010",
	"111100111111",
	"1100110101",
	"0001010111000",        /* 0x40 */
	"00101001",
	"11001111",
	"11110001",
	"11110100",
	"11000000",
	"11001100",
	"00010100111",
	"0010100010",           /* 0x48 */
	"11110010",
	"1100000110",
	"1100110100",
	"110011101",
	"111101010",
	"111100000",
	"000101000",
	"000101100",            /* 0x50 */
	"00101000110101",
	"110000010",
	"1111011",
	"0001101",
	"1100000111",
	"1100011000",
	"0001010100",
	"0001010111011",        /* 0x58 */
	"1111010111",
	"1100011001",
	"00101000110100",
	"11110000110101",
	"11110000110111",
	"11000110111111",
	"111100001100",
	"11110000111000",       /* 0x60 */
	"01000",
	"0000110",
	"010011",
	"00111",
	"011",
	"0000111",
	"000111",
	"000100",               /* 0x68 */
	"1101",
	"00010100110",
	"0010101",
	"000010",
	"001011",
	"0101",
	"010010",
	"11000010",             /* 0x70 */
	"1111010110",
	"1110",
	"00100",
	"00000",
	"11111",
	"11000011",
	"0001100",
	"1100011010",           /* 0x78 */
	"0001010110",
	"1100010",
	"11000110111110",
	"11110000110110",
	"11000110111101",
	"11000110111100",
	"1111000011110001"
};

struct hufftab {
	unsigned int bits;
	unsigned int len;
};

/* --------------------------------------------------------------------- */

static void strtohufftab(struct hufftab *h, const char *str, unsigned int n, unsigned int nbits)
{
	h->bits = h->len = 0;
	while (*str) {
		switch (*str) {
		case '1':
			h->bits |= 1 << (h->len);
			/* fall through */
		case '0':
			h->len++;
			if (h->len >= 20) {
				fprintf(stderr, "huffman string \"%s\" too long\n", str);
				exit(1);
			}
			break;

		default:
			fprintf(stderr, "huffman string \"%s\" invalid\n", str);
			exit(1);
		}
		str++;
	}
	for (; nbits > 0; nbits--) {
		if (n & (1 << (nbits-1)))
			h->bits |= 1 << (h->len);
		h->len++;
		if (h->len >= 20) {
			fprintf(stderr, "huffman string \"%s\" too long\n", str);
			exit(1);
		}
	}
}

/* --------------------------------------------------------------------- */

static void gen_huff(void)
{
	struct hufftab hufftab[290];
	int i, j;
#ifdef PARANOID
	int k;
#endif

	for (i = 0; i < 128; i++)
		strtohufftab(hufftab+i, huff[i], 0, 0);
	for (i = 128; i < 256; i++)
		strtohufftab(hufftab+i, "111100110", (i & 127), 7);  /* upper ascii */
	for (i = 256; i < 288; i++)
		strtohufftab(hufftab+i, "11110000111001", (i & 31), 5);  /* rle 0-31 */
	strtohufftab(hufftab+288, "1111000011110000", 0, 0);  /* idle */
	strtohufftab(hufftab+289, "1111000011101", 0, 0);  /* unused */

#ifdef PARANOID
	for (i = 0; i < (1<<19); i++) {
		for (k = j = 0; j < 290; j++)
			if ((i & ((1 << hufftab[j].len)-1)) == hufftab[j].bits)
				k++;
		if (k != 1)
			fprintf(stderr, "Warning: bit pattern 0x%x  %d entries matching\n", i, k);
	}
#endif

	printf("static const struct gtor_huffman_table huff_table_1[290] = {\n\t");
	for (i = 0; i < 290; i++) {
		printf("{ 0x%05x, %2d }", hufftab[i].bits, hufftab[i].len);
		if (i >= 289)
			break;
		printf((i & 3) != 3 ? ", " : ",\n\t");
	}
	printf("\n};\n\nstatic const struct gtor_huffman_table huff_table_2[290] = {\n\t");
	for (i = 0; i < 290; i++) {
		j = i;
		if (i < 255 && isupper(i))
			j += 32;
		else if (i < 255 && islower(i))
			j -= 32;
		printf("{ 0x%05x, %2d }", hufftab[j].bits, hufftab[j].len);
		if (i >= 289)
			break;
		printf((i & 3) != 3 ? ", " : ",\n\t");
	}
	printf("\n};\n\n");
}

/* --------------------------------------------------------------------- */

static unsigned int golay_gen_matrix[12] = {
	0xdc5, 0xb8b, 0x717, 0xe2d, 0xc5b, 0x8b7,
	0x16f, 0x2dd, 0x5b9, 0xb71, 0x6e3, 0xffe
};

extern __inline__ unsigned int golay_encode(unsigned int w)
{
	unsigned int ret = 0;
	unsigned int i;
	unsigned int mask;

	for (mask = 0x800, i = 0; i < 12; i++, mask >>= 1)
		if (w & mask)
			ret ^= golay_gen_matrix[i];
	return ret;
}

extern __inline__ unsigned int golay_syndrome(unsigned int inp)
{
	unsigned int syn;

	/* first calculate the syndrome vector (we use the fact that golay_encode(golay_encode(p)) == p) */
	syn = golay_encode(inp & 0xfff) ^ (inp >> 12);
	syn &= 0xfff;
	return syn;
}

/* --------------------------------------------------------------------- */

static void gen_golay(void)
{
	unsigned short golay_encode_tab[0x1000];
	unsigned short golay_error_tab[0x1000];
	unsigned int corr[5];
	unsigned int u, a, b, c;

	for (u = 0; u < 0x1000; u++) {
		golay_encode_tab[u] = golay_encode(u);
		golay_error_tab[u] = 0xf000;
	}
	golay_error_tab[0] = 0x0000;
	for (a = 0; a < 24; a++) {
		u = golay_syndrome(1 << a);
		if (golay_error_tab[u] < 0xf000)
			fprintf(stderr, "golay: syndrome location 0x%03x already used, value 0x%04x!!\n",
				u, golay_error_tab[u]);
		golay_error_tab[u] = 0x1000 | ((1 << a) & 0xfff);
		for (b = a+1; b < 24; b++) {
			u = golay_syndrome((1 << a) | (1 << b));
			if (golay_error_tab[u] < 0xf000)
				fprintf(stderr, "golay: syndrome location 0x%03x already used, value 0x%04x!!\n",
					u, golay_error_tab[u]);
			golay_error_tab[u] = 0x2000 | (((1 << a) | (1 << b)) & 0xfff);
			for (c = b+1; c < 24; c++) {
				u = golay_syndrome((1 << a) | (1 << b) | (1 << c));
				if (golay_error_tab[u] < 0xf000)
					fprintf(stderr, "golay: syndrome location 0x%03x already used, value 0x%04x!!\n",
						u, golay_error_tab[u]);
				golay_error_tab[u] = 0x3000 | (((1 << a) | (1 << b) | (1 << c)) & 0xfff);
			}
		}
	}
	printf("unsigned short golay_encode_tab[0x1000] = {\n\t");
	for (u = 0; u < 0x1000; u++) {
		printf("0x%03x", golay_encode_tab[u]);
		if (u >= 0xfff)
			break;
		printf((u & 7) == 7 ? ",\n\t" : ", ");
	}
	printf("\n};\n\nunsigned short golay_error_tab[0x1000] = {\n\t");
	corr[0] = corr[1] = corr[2] = corr[3] = corr[4] = 0;
	for (u = 0; u < 0x1000; u++) {
		corr[(golay_error_tab[u] >= 0x4000) ? 4 : ((golay_error_tab[u] >> 12) & 0xf)]++;
		printf("0x%04x", golay_error_tab[u]);
		if (u >= 0xfff)
			break;
		printf((u & 7) == 7 ? ",\n\t" : ", ");
	}
	printf("\n};\n\n");
	fprintf(stderr, "golay: correcting errors: %u %u %u %u %u\n", corr[0], corr[1], corr[2], corr[3], corr[4]);
}

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
	printf("/*\n * automatically generated by %s, DO NOT EDIT\n */\n\n",
	       argv[0]);
	gen_huff();
	gen_golay();
	exit(0);
}

