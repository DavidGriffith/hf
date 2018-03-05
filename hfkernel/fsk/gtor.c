/*****************************************************************************/

/*
 *      gtor.c  --  GTOR protocol.
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


#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h> 
#include <syslog.h>

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"
#include "gtor.h"
#include "standby.h"

/* --------------------------------------------------------------------- */
/*
 * GTOR defines
 */

#define GTOR_CYCLE_ARQ   2400000UL

#define GTOR_CS1         0xf11a
#define GTOR_CS2         0x6b62
#define GTOR_CS3         0x5e13
#define GTOR_CS4         0x4d3c
#define GTOR_CS5         0x8957

#define GTOR_CORRECT_FCS        0xf47
#define GTOR_INVERTED_FCS_22    0x6873
#define GTOR_INVERTED_FCS_24    0x3984
#define GTOR_INVERTED_FCS_48    0x346e
#define GTOR_INVERTED_FCS_72    0x0886

/* --------------------------------------------------------------------- */

#define GTOR_RETRY_QRT          4
#define GTOR_RETRY_CALL         20
#define GTOR_RETRY_QSO          80
#define GTOR_RETRY_HISPEED      4
#define GTOR_RETRY_FEC          2

/* --------------------------------------------------------------------- */

#define RXOVERSAMPLING 8

#define AUTOMATIC_SPEEDUP

#define FREQ_TRACKING_DIST 10 /* Hz */
#define FREQ_TRACKING

/* --------------------------------------------------------------------- */

static struct {
	unsigned char destcall[10];
	unsigned char mycall[10];
	l1_time_t txdelay;
	unsigned int retry;
} gp = { 
	"\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f", "\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f", 
	30000, 30,
};

static struct {
	/*
	 * written by decode_packet
	 */
	unsigned char pkt_mon[128];
	unsigned char pkt_data[512];
	unsigned int pkt_mon_len;
	unsigned int pkt_data_len;
	unsigned char pkt_mycall[10];
	unsigned char pkt_destcall[10];

	unsigned char pkt_counter;
	/*
	 * misc state
	 */
	int is_master;
	int retry;
	int rxinv, txinv;
	int golay_flag;
	l1_time_t rxtime, txtime;
	int rxfreqdev, txfreqdev;
	unsigned char cur_hdr;
	unsigned int last_pkt_cnt;
	unsigned int qrt_cs;

	unsigned int flags;
	/*
	 * monitor stuff
	 */
	l1_time_t mon_mute;
	l1_time_t mon_callmute;

	struct {
		long devflt[32];
		unsigned int ptr;
	} tm;

	/*
	 * tx/rx storage
	 */
	unsigned int rx_validmask;
	unsigned short rx_dat[48];
	unsigned short rx_par[48];

	unsigned char txb_dat[72];
	unsigned char txb_par[72];
	l1_soft_t rxbuf[72*8*RXOVERSAMPLING];
#ifdef AUTOMATIC_SPEEDUP
	union {
		struct {
			l1_soft_t rxb1[24*8];
			l1_soft_t rxb2[24*8];
			l1_soft_t rxb3[24*8];
			l1_soft_t rxb4[24*8];
		} b100;

		struct {
			l1_soft_t rxb1[48*8];
			l1_soft_t rxb2[48*8];
		} b200;
	} spdup;
#endif /* AUTOMATIC_SPEEDUP */
#ifdef FREQ_TRACKING
	struct {
		l1_soft_t rxb1[72*8];
		l1_soft_t rxb2[72*8];
	} trk;
#endif /* FREQ_TRACKING */
} gs;

/* --------------------------------------------------------------------- */
/* 
 * gtor specific utility functions
 */

struct gtor_huffman_table {
	unsigned int bits;
	unsigned int len;
};

#include "gtor_tables.h"

/* --------------------------------------------------------------------- */

#define FLG_QRT         (1<<0)
#define FLG_BECOMEISS   (1<<1)
#define FLG_BECOMEIRS   (1<<2)
#define FLG_SPEEDUP     (1<<3)
#define FLG_SPEEDUP2    (1<<4)

/*
 * this flags purpose is to delay the output of 200 baud receive data
 * to prevent double data in case of switch back to 100 baud... hairy...
 */
#define FLG_VALIDDATA   (1<<31)

void gtor_mode_qrt(void)
{
	gs.flags |= FLG_QRT;
}

void gtor_mode_irs(void)
{
	gs.flags |= FLG_BECOMEIRS;
}

void gtor_mode_iss(void)
{
	gs.flags |= FLG_BECOMEISS;
}

void gtor_mode_speedup(void)
{
	if (gs.flags & FLG_SPEEDUP)
		gs.flags |= FLG_SPEEDUP2;
	else
		gs.flags |= FLG_SPEEDUP;
}

/* --------------------------------------------------------------------- */

static void strnupr(unsigned char *c, size_t len)
{
	for (; len > 0 && *c; len--, c++)
		if (islower(*c))
			*c -= 'a'-'A';
}

/* --------------------------------------------------------------------- */

void gtor_set_params(const char *destcall, const char *mycall, int txd, int retry)
{
	unsigned char *bp;
	
	errprintf(SEV_INFO, "gtor params: destcall %-10s mycall %-10s txdelay %dms retries %d\n", 
		  destcall, mycall, txd, retry);
	strncpy(gp.destcall, destcall, sizeof(gp.destcall));
	if ((bp = memchr(gp.destcall, 0, sizeof(gp.destcall))))
		memset(bp, 0x0f, gp.destcall+sizeof(gp.destcall)-bp);
	strnupr(gp.destcall, sizeof(gp.destcall));
	strncpy(gp.mycall, mycall, sizeof(gp.mycall));
	if ((bp = memchr(gp.mycall, 0, sizeof(gp.mycall))))
		memset(bp, 0x0f, gp.mycall+sizeof(gp.mycall)-bp);
	strnupr(gp.mycall, sizeof(gp.mycall));
	gp.txdelay = txd * 1000;
	gp.retry = retry;
}

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int get_crc_preset(int pktlen)
{
	return 0xffff;
}

/* --------------------------------------------------------------------- */

static const unsigned char rle_offset[17] = {
	10, 10, 10, 7, 5, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2
};

static unsigned int encode_plain(unsigned char *bp, unsigned int len)
{
	unsigned int chr_cnt = 0;
	unsigned short ch;

	kbd_negack();
	while (len > 0) {
		if ((ch = kbd_get()) == KBD_EOF) {
			for (; len > 0; len--) 
				*bp++ = 0x1e; /* idle code */
			return chr_cnt;
		}
		switch (ch) {
		case 0x1c:
			if (len < 2) {
				kbd_back();
				*bp = 0x1e;
				return chr_cnt;
			}
			*bp++ = 0x1c;
			*bp++ = 0x7c;
			len -= 2;
			break;

		case 0x1e:
			if (len < 2) {
				kbd_back();
				*bp = 0x1e;
				return chr_cnt;
			}
			*bp++ = 0x1c;
			*bp++ = 0x7e;
			len -= 2;
			break;

		default:
			*bp++ = ch;
			len--;
		}
		if (ch >= ' ' && gs.pkt_mon_len < sizeof(gs.pkt_mon))
			gs.pkt_mon[gs.pkt_mon_len++] = ch;
		chr_cnt++;
	}
	return chr_cnt;
}

static unsigned int encode_huff(const struct gtor_huffman_table huff[290], 
				unsigned char *bp, unsigned int len)
{
	unsigned short prev_char = KBD_EOF;
	unsigned int prev_char_len = 0;
	unsigned int last_cnt = 0;
	unsigned int chr_cnt = 0;
	unsigned int bptr = 0;
	unsigned short ch;
	unsigned int bb;
	
	kbd_negack();
	memset(bp, 0, len);
	for (;;) {		
                if ((ch = kbd_get()) == KBD_EOF || (ch & KBD_CHAR) != prev_char) {
			if (last_cnt > 0) {
				while (last_cnt >= rle_offset[prev_char_len] && (bptr + huff[256].len) <= (len * 8)) {
					last_cnt -= rle_offset[prev_char_len];
					chr_cnt += rle_offset[prev_char_len];
					if (last_cnt > 31) {
						bb = huff[256+31].bits;
						last_cnt -= 31;
						chr_cnt += 31;
					} else {
						bb = huff[256+last_cnt].bits;
						chr_cnt += last_cnt;
						last_cnt = 0;
					}
					bb <<= bptr % 8;
					bp[bptr / 8] |= bb & 0xff;
					bp[(bptr / 8)+1] |= (bb >> 8) & 0xff;
					bp[(bptr / 8)+2] |= (bb >> 16) & 0xff;
					bp[(bptr / 8)+3] |= (bb >> 24) & 0xff;
					bptr += huff[256].len;
				}
				while (last_cnt > 0 && (bptr + huff[prev_char].len) <= (len * 8)) {
					bb = huff[prev_char].bits;
					bb <<= bptr % 8;
					bp[bptr / 8] |= bb & 0xff;
					bp[(bptr / 8)+1] |= (bb >> 8) & 0xff;
					bp[(bptr / 8)+2] |= (bb >> 16) & 0xff;
					bp[(bptr / 8)+3] |= (bb >> 24) & 0xff;
					bptr += huff[prev_char].len;
					chr_cnt++;
				}
			}
			if (ch == KBD_EOF || last_cnt > 0) {
				for (; last_cnt > 0; last_cnt--)
					kbd_back();
				while (bptr < len * 8) {
					bb = huff[288].bits;
					bb <<= bptr % 8;
					bp[bptr / 8] |= bb & 0xff;
					bp[(bptr / 8)+1] |= (bb >> 8) & 0xff;
					bp[(bptr / 8)+2] |= (bb >> 16) & 0xff;
					bp[(bptr / 8)+3] |= (bb >> 24) & 0xff;
					bptr += huff[288].len;
				}
				return chr_cnt;
			}
			prev_char = ch & KBD_CHAR;
			last_cnt = 0;
			prev_char_len = huff[ch].len;
			if (bptr + prev_char_len > (len * 8)) {
				kbd_back();
				while (bptr < len * 8) {
					bb = huff[288].bits;
					bb <<= bptr % 8;
					bp[bptr / 8] |= bb & 0xff;
					bp[(bptr / 8)+1] |= (bb >> 8) & 0xff;
					bp[(bptr / 8)+2] |= (bb >> 16) & 0xff;
					bp[(bptr / 8)+3] |= (bb >> 24) & 0xff;
					bptr += huff[288].len;
				}
				return chr_cnt;
			}
			bb = huff[prev_char].bits;
			bb <<= bptr % 8;
			bp[bptr / 8] |= bb & 0xff;
			bp[(bptr / 8)+1] |= (bb >> 8) & 0xff;
			bp[(bptr / 8)+2] |= (bb >> 16) & 0xff;
			bp[(bptr / 8)+3] |= (bb >> 24) & 0xff;
			bptr += prev_char_len;
			if (prev_char >= ' ' && gs.pkt_mon_len < sizeof(gs.pkt_mon))
				gs.pkt_mon[gs.pkt_mon_len++] = ch;
			chr_cnt++;
			continue;
		}
		if ((ch & KBD_CHAR) == prev_char) {
			last_cnt++;
			continue;
		}
	}
	return chr_cnt;
}

/* --------------------------------------------------------------------- */

extern __inline__ unsigned char swap_nibbles(unsigned char c)
{
	return ((c & 0xf) << 4) | ((c >> 4) & 0xf);
}

/* --------------------------------------------------------------------- */

static void encode_conn_disc_packet(int conn, unsigned char *bp)
{
	char buf1[11];
	char buf2[11];
	char *cp;
	unsigned int crc;

	memcpy(buf1, gp.destcall, 10);
	buf1[10] = 0;
	if ((cp = memchr(buf1, 0xf, 10)))
		*cp = 0;
	memcpy(buf2, gp.mycall, 10);
	buf2[10] = 0;
	if ((cp = memchr(buf2, 0xf, 10)))
		*cp = 0;
	if (conn)
		memcpy(bp, gp.destcall, 10);
	else
		memcpy(bp, gs.pkt_destcall, 10);
	memcpy(bp+10, gp.mycall, 10);
	bp[20] = 0;
	if (conn) {
		bp[21] = 0xc0;
		gs.pkt_counter = 0;
	} else 
		bp[21] = 0x80 | (gs.pkt_counter & 3);
	bp[1] = swap_nibbles(bp[1] | 0x80);
	bp[4] = swap_nibbles(bp[4] | 0x80);
	bp[7] = swap_nibbles(bp[7] | 0x80);
	bp[10] = swap_nibbles(bp[10] | 0x80);
	bp[13] = swap_nibbles(bp[13] | 0x80);
	bp[16] = swap_nibbles(bp[16] | 0x80);
	bp[19] = swap_nibbles(bp[19] | 0x80);
	crc = calc_crc_ccitt(bp, 22, get_crc_preset(22));
	bp[22] = crc >> 8;
	bp[23] = crc & 0xff;
	gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "%s: C:%c DESTCALL:%s MYCALL:%s", 
				  conn ? "CONN" : "DISC", '0' + (gs.pkt_counter & 3), buf1, buf2);
}

/* --------------------------------------------------------------------- */

static void encode_packet(unsigned char *bp, unsigned int len)
{
	unsigned int len1, len2, len3, crc;

	if (len == 19) {
		bp[-2] = GTOR_CS3 & 0xff;
		bp[-1] = GTOR_CS3 >> 8;
		gs.flags &= ~FLG_BECOMEIRS;
	} else if (gs.flags & FLG_QRT) {
		memset(bp, 0x1e, len);
		bp[len] = 0x80 | (gs.pkt_counter & 3);
		crc = calc_crc_ccitt(bp, len+1, get_crc_preset(len+1));
		bp[len+1] = crc >> 8;
		bp[len+2] = crc & 0xff;
		gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "QRT: C:%c\n", '0' + (gs.pkt_counter & 3));
		return;
	}
	gs.pkt_mon_len = 0;
	len1 = encode_plain(bp, len);
	gs.pkt_mon_len = 0;
	len2 = encode_huff(huff_table_1, bp, len);
	gs.pkt_mon_len = 0;
	len3 = encode_huff(huff_table_2, bp, len);
	if (len1 >= len2 && len1 >= len3) {
		gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "%s: C:%cP \"", 
					  len == 19 ? "BRKIN" : "DATA", '0' + (gs.pkt_counter & 3));
		encode_plain(bp, len);
		bp[len] = 0;
	} else if (len2 >= len3) {
		gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "%s: C:%ch \"", 
					  len == 19 ? "BRKIN" : "DATA", '0' + (gs.pkt_counter & 3));
		encode_huff(huff_table_1, bp, len);
		bp[len] = 0x04;
	} else {
		gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "%s: C:%cH \"", 
					  len == 19 ? "BRKIN" : "DATA", '0' + (gs.pkt_counter & 3));
		encode_huff(huff_table_2, bp, len);
		bp[len] = 0x08;
 	}
	bp[len] |= (gs.pkt_counter & 3);
        if (gs.flags & FLG_BECOMEIRS && len != 19)
		bp[len] |= 0x40;
	crc = calc_crc_ccitt(bp, len+1, get_crc_preset(len+1));
	bp[len+1] = crc >> 8;
	bp[len+2] = crc & 0xff;
	gs.pkt_mon_len += snprintf(gs.pkt_mon+gs.pkt_mon_len, sizeof(gs.pkt_mon)-gs.pkt_mon_len, "\"");
}

/* --------------------------------------------------------------------- */

#define PKT_NEW   (1<<0)
#define PKT_BRKIN (1<<1)
#define PKT_QRT   (1<<2)
#define PKT_IDLE  (1<<3)
#define PKT_CALL  (1<<4)

static void decode_plain(unsigned char *bp, unsigned int len)
{
	while (len > 0) {
		if (*bp == 0x1c) {
			if (len < 2)
				return;
			bp++;
			len--;
			if (gs.pkt_data_len < sizeof(gs.pkt_data))
				gs.pkt_data[gs.pkt_data_len++] = (*bp)-0x60;
			if (gs.pkt_mon_len < sizeof(gs.pkt_mon))
				gs.pkt_mon[gs.pkt_mon_len++] = '.';
		} else if (*bp != 0x1e) {
			if (gs.pkt_data_len < sizeof(gs.pkt_data))
				gs.pkt_data[gs.pkt_data_len++] = *bp;
			if (gs.pkt_mon_len < sizeof(gs.pkt_mon))
				gs.pkt_mon[gs.pkt_mon_len++] = *bp >= ' ' ? *bp : '.';
		}
		len--;
		bp++;
	}
}

static void decode_huff(const struct gtor_huffman_table huff[290],
			unsigned char *bp, unsigned int len)
{
	unsigned int bptr = 0;
	unsigned int bb;
	unsigned int i;
	int prev_ch = -1;
	unsigned int prev_len = 0;

	for (;;) {
		bb = bp[bptr / 8] | (bp[(bptr / 8)+1] << 8) | (bp[(bptr / 8)+2] << 16) | (bp[(bptr / 8)+3] << 24);
		bb >>= bptr % 8;
		for (i = 0; i < 290 && (bb & ((1 << huff[i].len)-1)) != huff[i].bits; i++);
		if (i >= 290) {
			errprintf(SEV_WARNING, "GTOR: decode_huff: error\n");
			return;
		}
		bptr += huff[i].len;
		if (bptr > 8*len)
			return;
		if (i >= 288)
			continue;
		if (i < 256) {
			prev_ch = i;
			prev_len = huff[i].len;
			if (gs.pkt_data_len < sizeof(gs.pkt_data))
				gs.pkt_data[gs.pkt_data_len++] = prev_ch;
			if (gs.pkt_mon_len < sizeof(gs.pkt_mon))
				gs.pkt_mon[gs.pkt_mon_len++] = prev_ch >= ' ' ? prev_ch : '.';
			continue;
		}
		if (prev_ch < 0)
			continue;
		i += rle_offset[prev_len] - 256;
		if (gs.pkt_mon_len < sizeof(gs.pkt_mon))
			gs.pkt_mon[gs.pkt_mon_len++] = prev_ch >= ' ' ? prev_ch : '.';
		for (; i > 0 && gs.pkt_data_len < sizeof(gs.pkt_data); i--)
			gs.pkt_data[gs.pkt_data_len++] = prev_ch;
	}
}

static unsigned int decode_packet(unsigned char *bp, unsigned int len)
{
	char buf1[11];
	char buf2[11];
	char *cp;
	unsigned int retval = 0;
	
        gs.pkt_data[0] = '\0';
	gs.pkt_data_len = 0;
        gs.pkt_mon[0] = '\0';
	gs.pkt_mon_len = 0;
	if ((bp[len] & 0xc0) >= 0x80) {
		retval = (bp[len] & 0xc0) == 0xc0 ? PKT_CALL : PKT_QRT;
		if (len > 21) {
			gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), 
					   "%s: C:%c", retval & PKT_CALL ? "CONN" : "DISC", '0' + (gs.pkt_counter & 3));
			return retval;
		}
		/* display QRT packet */
		memcpy(buf1, bp, 10);
		memcpy(buf2, bp+10, 10);
		buf1[1] = swap_nibbles(buf1[1]) & 0x7f;
		buf1[4] = swap_nibbles(buf1[4]) & 0x7f;
		buf1[7] = swap_nibbles(buf1[7]) & 0x7f;
		buf2[0] = swap_nibbles(buf2[0]) & 0x7f;
		buf2[3] = swap_nibbles(buf2[3]) & 0x7f;
		buf2[6] = swap_nibbles(buf2[6]) & 0x7f;
		buf2[9] = swap_nibbles(buf2[9]) & 0x7f;
		buf1[10] = 0;
		buf2[10] = 0;
		memcpy(gs.pkt_destcall, buf1, sizeof(gs.pkt_destcall));
		memcpy(gs.pkt_mycall, buf2, sizeof(gs.pkt_mycall));
		if ((cp = memchr(buf1, 0xf, 10)))
			*cp = 0;
		if ((cp = memchr(buf2, 0xf, 10)))
			*cp = 0;
		gs.pkt_counter = bp[len] & 3;
		gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "%s: C:%c DESTCALL: %s MYCALL: %s", 
					  retval & PKT_CALL ? "CONN" : "DISC", '0' + (gs.pkt_counter & 3),
					  buf1, buf2);
		return retval;
	}
        if ((bp[len] & 0x03) != gs.pkt_counter) {
                retval |= PKT_NEW;
                gs.pkt_counter = bp[len] & 0x03;
		if (gs.last_pkt_cnt <= gs.pkt_data_len)
			gs.last_pkt_cnt = 0;
		else
			gs.last_pkt_cnt -= gs.pkt_data_len;
        }
        if (bp[len] & 0x40)
                retval |= PKT_BRKIN;
	switch (bp[len] & 0x0c) {
	case 0x0:
		gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "%s: C:%cP \"", 
					  retval & PKT_BRKIN ? "BRKIN" : "DATA", '0' + (gs.pkt_counter & 3));
		decode_plain(bp, len);
		break;

	case 0x4:
		gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "%s: C:%ch \"", 
					  retval & PKT_BRKIN ? "BRKIN" : "DATA", '0' + (gs.pkt_counter & 3));
		decode_huff(huff_table_1, bp, len);
		break;

	case 0x8:
		gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "%s: C:%cH \"", 
					  retval & PKT_BRKIN ? "BRKIN" : "DATA", '0' + (gs.pkt_counter & 3));
		decode_huff(huff_table_2, bp, len);
		break;

	default:
		gs.pkt_mon_len = snprintf(gs.pkt_mon, sizeof(gs.pkt_mon), "%s: C:%c INVALID COMPRESSION\n", 
					  retval & PKT_BRKIN ? "BRKIN" : "DATA", '0' + (gs.pkt_counter & 3));
		return retval;
	}
	gs.pkt_mon_len += snprintf(gs.pkt_mon+gs.pkt_mon_len, sizeof(gs.pkt_mon)-gs.pkt_mon_len, "\"");
        if (gs.pkt_data_len <= 0)
                retval |= PKT_IDLE; 
	else if (gs.pkt_data_len > gs.last_pkt_cnt) {
		bufwrite(HFAPP_MSG_DATA_RECEIVE, gs.pkt_data + gs.last_pkt_cnt, gs.pkt_data_len-gs.last_pkt_cnt);
		gs.last_pkt_cnt = gs.pkt_data_len;
	}
	return retval;
}

/* --------------------------------------------------------------------- */
/*
 * Timing functions
 */

#define TMSIZE (sizeof(gs.tm.devflt)/sizeof(gs.tm.devflt[0]))

extern __inline__ void tmg_clear(void)
{
	memset(gs.tm.devflt, 0, sizeof(gs.tm.devflt));
	gs.tm.ptr = 0;
}

static void tmg_add(long dev)
{
	long acc = 0;
	int i;
	
	gs.tm.devflt[gs.tm.ptr++] =  dev;
#if 0
	printf("Timing deviation: %ld  ptr: %d  smooth: %ld %ld %ld %ld %ld %ld %ld %ld\n", dev, gs.tm.ptr,
	       gs.tm.devflt[0], gs.tm.devflt[1], gs.tm.devflt[2], gs.tm.devflt[3], gs.tm.devflt[4], 
	       gs.tm.devflt[5], gs.tm.devflt[6], gs.tm.devflt[7]);
#endif
	gs.tm.ptr %= TMSIZE;
	for (i = 0; i < TMSIZE; i++)
		acc += gs.tm.devflt[i];
	acc /= (signed)TMSIZE;
	if (!acc)
		return;
	for (i = 0; i < TMSIZE; i++)
		gs.tm.devflt[i] -= acc;
#if 1
	gs.rxtime += acc;
	if (!gs.is_master)
		gs.txtime += acc;
#endif
	bufprintf(HFAPP_MSG_DATA_MONITOR, "Timing correction: %ld\n", acc);
}

/* --------------------------------------------------------------------- */
/*
 * FEC utility functions
 */

#if 0

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

#else

extern __inline__ unsigned int golay_encode(unsigned int w)
{
	return golay_encode_tab[w & 0xfff];
}

#endif

/* --------------------------------------------------------------------- */

static void hard_interleave(const unsigned char *in, 
			    unsigned char *out, unsigned int len)
{
	unsigned int trip, idx, i, j;

	if (len % 3)
		errprintf(SEV_FATAL, "hard_interleave: invalid length\n");
	memset(out, 0, len);
	len /= 3;
	len *= 2;
	for (i = 0; i < len; i += 2, in += 3) {
		trip = (in[0] << 4) | (in[1] >> 4);
		for (j = 0, idx = i; j < 12; j++, idx += len, trip <<= 1)
			if (trip & 0x800)
				out[idx / 8] |= 1 << (idx % 8);
		
		trip = (in[1] << 8) | in[2];
		for (j = 0, idx = i+1; j < 12; j++, idx += len, trip <<= 1)
			if (trip & 0x800)
				out[idx / 8] |= 1 << (idx % 8);
	}
}

/* --------------------------------------------------------------------- */

static void hard_parity_interleave(const unsigned char *in, 
				   unsigned char *out, unsigned int len)
{
	unsigned int trip, idx, i, j;

	if (len % 3)
		errprintf(SEV_FATAL, "hard_interleave: invalid length\n");
	memset(out, 0, len);
	len /= 3;
	len *= 2;
	for (i = 0; i < len; i += 2, in += 3) {
		trip = (in[0] << 4) | (in[1] >> 4);
		trip = golay_encode(trip);
		for (j = 0, idx = i; j < 12; j++, idx += len, trip <<= 1)
			if (trip & 0x800)
				out[idx / 8] |= 1 << (idx % 8);
		
		trip = (in[1] << 8) | in[2];
		trip = golay_encode(trip);
		for (j = 0, idx = i+1; j < 12; j++, idx += len, trip <<= 1)
			if (trip & 0x800)
				out[idx / 8] |= 1 << (idx % 8);
	}
}

/* --------------------------------------------------------------------- */

extern __inline__ void soft_deinterleave(const int *inp, unsigned short *out, 
					 unsigned int ntrib, unsigned int spacing, int inv)
{
	unsigned int invm = inv ? 0 : ~0;
	unsigned int totspc, i, j, idx, trip;

	totspc = ntrib * spacing;
	for (i = 0; i < ntrib; i++, out++) {
		for (trip = j = 0, idx = i*spacing; j < 12; j++, idx += totspc)
			trip = (trip << 1) | (((inp[idx] >> 31) ^ invm) & 1);
		out[0] = trip;
	}
}

/* --------------------------------------------------------------------- */

extern __inline__ void trib_to_bytes(const unsigned short *in, unsigned char *out, unsigned int ntrib)
{
	for (; ntrib >= 2; ntrib -= 2, in += 2, out += 3) {
		out[0] = in[0] >> 4;
		out[1] = (in[0] << 4) | ((in[1] >> 8) & 0xf);
		out[2] = in[1];
	}
	if (ntrib & 1) {
		out[0] = in[0] >> 4;
		out[1] = in[0] << 4;
	}
}

/* --------------------------------------------------------------------- */

extern __inline__ void trib_parity_to_bytes(const unsigned short *in, unsigned char *out, unsigned int ntrib)
{
	unsigned short t0, t1;

	for (; ntrib >= 2; ntrib -= 2, in += 2, out += 3) {
		t0 = golay_encode(in[0]);
		t1 = golay_encode(in[1]);
		out[0] = t0 >> 4;
		out[1] = (t0 << 4) | ((t1 >> 8) & 0xf);
		out[2] = t1;
	}
	if (ntrib & 1) {
		t0 = golay_encode(in[0]);
		out[0] = t0 >> 4;
		out[1] = t0 << 4;
	}
}

/* --------------------------------------------------------------------- */

static int ecc_packet(const unsigned short *indat, const unsigned short *inpar,
				 unsigned short *out, unsigned int ntrib)
{
	unsigned int syn, tbl, corr = 0;

	for (; ntrib > 0; ntrib--, indat++, inpar++, out++) {
		/* first calculate the syndrome vector (we use the fact that golay_encode(golay_encode(p)) == p) */
		syn = (golay_encode(*indat) ^ (*inpar)) & 0xfff;
		tbl = golay_error_tab[syn];
		if (tbl >= 0x4000)
			return -1;
		*out = ((*indat) ^ tbl) & 0xfff;
		corr += ((tbl >> 12) & 0xf);
	}
	return corr;
}

/* --------------------------------------------------------------------- */
/*
 * misc utility functions
 */

#if 0
static void prhex(const unsigned char *bp, int len)
{
	char buf[256];
	int i = 0;
	
	for (; len > 0; len--, bp++)
		i += snprintf(buf+i, sizeof(buf)-i, " %02x", bp[0]);
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: TX:%s\n", buf);
}

static void prsoft(const int *inp, int samp, int spac)
{
	unsigned char buf[128];

	soft_to_hard(inp, buf, samp, spac, gs.rxinv);
	prhex(buf, (samp+7) >> 3);
}

#include <stdio.h>
static void prgraph(const char *title, const int *inp, int samp)
{
	FILE *f = popen("xgraph", "w");
	int i;

	if (!f)
		return;
	fprintf(f, "TitleText: %s\n\"Demod\"\n", title);
	for (i = 0; i < samp; i++)
		fprintf(f, "%d %d\n", i, inp[i]);
	pclose(f);
}
#endif

extern __inline__ void cycle_end(void)
{
	gs.rxtime += GTOR_CYCLE_ARQ;
	gs.txtime += GTOR_CYCLE_ARQ;
	gs.golay_flag = !gs.golay_flag;
}

extern __inline__ void ack_transmit(void)
{
	kbd_ack();
	gs.pkt_counter = (gs.pkt_counter + 1) & 3;
}

extern __inline__ void fec_clear(void)
{
	gs.rx_validmask = 0;
	memset(gs.rx_dat, 0, sizeof(gs.rx_dat));
	memset(gs.rx_par, 0, sizeof(gs.rx_par));
}

extern __inline__ int retry(void)
{
	if ((--gs.retry) <= 0)
		return 1;
	return 0;
}

#ifdef FREQ_TRACKING
extern __inline__ void gtor_freq_tracking(int trk, l1_soft_t trkl, l1_soft_t trkm, l1_soft_t trkh)
{
	gs.rxfreqdev += trk;
	if (!gs.is_master)
		gs.txfreqdev += trk;
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: TRACKING: %d  %d/%d/%d  FREQDEVIATION: %d/%d\n", 
		  trk, trkl, trkm, trkh, gs.rxfreqdev, gs.txfreqdev);
}
#endif /* FREQ_TRACKING */

static void prepare_tx_packet_100(void)
{
	unsigned char buf[24];
	
	encode_packet(buf, 21);
	hard_interleave(buf, gs.txb_dat, 24);
	hard_parity_interleave(buf, gs.txb_par, 24);
}

static void prepare_tx_packet_200(void)
{
	unsigned char buf[48];
	
	encode_packet(buf, 45);
	hard_interleave(buf, gs.txb_dat, 48);
	hard_parity_interleave(buf, gs.txb_par, 48);
}

static void prepare_tx_packet_300(void)
{
	unsigned char buf[72];
	
	encode_packet(buf, 69);
	hard_interleave(buf, gs.txb_dat, 72);
	hard_parity_interleave(buf, gs.txb_par, 72);
}

static void prepare_tx_packet_conndisc(int conn)
{
	unsigned char buf[24];
	
	encode_conn_disc_packet(conn, buf);
	hard_interleave(buf, gs.txb_dat, 24);
	hard_parity_interleave(buf, gs.txb_par, 24);
}

static void prepare_tx_packet_brkin(void)
{
	gs.pkt_counter = 0;
	encode_packet(gs.txb_dat+2, 19);
	memcpy(gs.txb_par, gs.txb_dat, 24);
}

/* --------------------------------------------------------------------- */

static void send_cs(int cs)
{
	static unsigned int csbits[5] = { GTOR_CS1, GTOR_CS2, GTOR_CS3, GTOR_CS4, GTOR_CS5 };
	unsigned int bits;

	assert(cs >= 1 && cs <= 5);
	bits = csbits[cs-1];
	l1_fsk_tx_request(gs.txtime-gp.txdelay, gp.txdelay, gs.txfreqdev, 0, 0x100, 1, gs.txb_dat);
	gs.txb_dat[0] = bits;
	gs.txb_dat[1] = bits >> 8;
	l1_fsk_tx_request(gs.txtime, 1000000/100, gs.txfreqdev, gs.txinv, 0x101, 16, gs.txb_dat);
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: TX: CS%c\n", '0'+cs);
}

static void send_packet(unsigned int bd)
{
	unsigned char *bp = gs.golay_flag ? gs.txb_par : gs.txb_dat;

	assert(bd >= 1 && bd <= 3);
	l1_fsk_tx_request(gs.txtime-gp.txdelay, gp.txdelay, gs.txfreqdev, 0, 0x100, 1, gs.txb_dat);
	l1_fsk_tx_request(gs.txtime, 1000000/100/bd, gs.txfreqdev, gs.txinv, 0x101, 8*24*bd, bp);
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: TX%c00(%c): %s  data: %02x %02x %02x %02x  time: %lu\n", 
		  '0'+bd, gs.golay_flag ? 'G' : 'D', gs.pkt_mon, bp[0], bp[1], bp[2], bp[3], gs.txtime);
}
	
static int receive_cs(void)
{
	unsigned char csbuf[2];
	unsigned int csbits;
	long dev;
	unsigned int diff1, diff2, diff3, diff4, diff5;
#ifdef FREQ_TRACKING
	l1_soft_t trkm, trkl, trkh;
	int trk;
#endif /* FREQ_TRACKING */

	l1_fsk_rx_request(gs.rxtime, 1000000/100/RXOVERSAMPLING, gs.rxfreqdev, 
			  100, 0x200, 16*RXOVERSAMPLING, gs.rxbuf);
#ifdef FREQ_TRACKING
	l1_fsk_rx_request(gs.rxtime, 1000000/100, gs.rxfreqdev-FREQ_TRACKING_DIST, 100, 0x210, 16, gs.trk.rxb1);
	l1_fsk_rx_request(gs.rxtime, 1000000/100, gs.rxfreqdev+FREQ_TRACKING_DIST, 100, 0x211, 16, gs.trk.rxb2);
#endif /* FREQ_TRACKING */
	while (l1_fsk_wait_request() != 0x200);
	soft_to_hard(gs.rxbuf, csbuf, 16, RXOVERSAMPLING, gs.rxinv);
	csbits = (csbuf[0] | (csbuf[1] << 8)) & 0xffff;
	dev = soft_time_dev(gs.rxbuf, 16, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
	tmg_add(dev);
#ifdef FREQ_TRACKING
	trk = freq_tracking(gs.rxbuf, gs.trk.rxb1, gs.trk.rxb2, 16, RXOVERSAMPLING, &trkm, &trkl, &trkh);
	gtor_freq_tracking(trk, trkl, trkm, trkh);
#endif /* FREQ_TRACKING */
	diff1 = hweight16(csbits ^ GTOR_CS1);
	if (diff1 == 0) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS1 dT=%ld\n", dev);
		return 1;
	} else if (diff1 < 4) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS1 diff=%d dT=%ld bits: %03x\n", diff1, dev, csbits);
		return 6;
	} else if (diff1 == 16) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: INV CS1 dT=%ld\n", dev);
		return 11;
	}
	diff2 = hweight16(csbits ^ GTOR_CS2);
	if (diff2 == 0) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS2 dT=%ld\n", dev);
		return 2;
	} else if (diff2 < 4) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS2 diff=%d dT=%ld bits: %03x\n", diff2, dev, csbits);
		return 7;
	} else if (diff2 == 16) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: INV CS2 dT=%ld\n", dev);
		return 12;
	}
	diff3 = hweight16(csbits ^ GTOR_CS3);
	if (diff3 == 0) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS3 dT=%ld\n", dev);
		return 3;
	} else if (diff3 < 4) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS3 diff=%d dT=%ld bits: %03x\n", diff3, dev, csbits);
		return 8;
	} else if (diff3 == 16) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: INV CS3 dT=%ld\n", dev);
		return 13;
	}
	diff4 = hweight16(csbits ^ GTOR_CS4);
	if (diff4 == 0) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS4 dT=%ld\n", dev);
		return 4;
	} else if (diff4 < 4) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS4 diff=%d dT=%ld bits: %03x\n", diff4, dev, csbits);
		return 9;
	} else if (diff4 == 16) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: INV CS4 dT=%ld\n", dev);
		return 14;
	}
	diff5 = hweight16(csbits ^ GTOR_CS5);
	if (diff5 == 0) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS5 dT=%ld\n", dev);
		return 5;
	} else if (diff5 < 4) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS5 diff=%d dT=%ld bits: %03x\n", diff5, dev, csbits);
		return 10;
	} else if (diff5 == 16) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: INV CS5 dT=%ld\n", dev);
		return 15;
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS? dT=%ld bits: %03x\n", dev, csbits);
	return 0;
}

static const unsigned int inv_fcs[4] = {
	GTOR_INVERTED_FCS_22, GTOR_INVERTED_FCS_24, GTOR_INVERTED_FCS_48, GTOR_INVERTED_FCS_72
};


static int receive_bkpacket(void)
{
	unsigned char pkt[22];
	unsigned int crc;
	int i;
	long dev;
#ifdef FREQ_TRACKING
	l1_soft_t trkm, trkl, trkh;
	int trk = 0;
#endif /* FREQ_TRACKING */

	/* special case: 100 baud breakin packet (no interleaving/FEC) */
	l1_fsk_rx_request(gs.rxtime+16*1000000/100, 1000000/100/RXOVERSAMPLING, gs.txfreqdev, 
			  100, 0x200, 22*8*RXOVERSAMPLING, gs.rxbuf);
#ifdef FREQ_TRACKING
	l1_fsk_rx_request(gs.rxtime+16*1000000/100, 1000000/100, gs.rxfreqdev-FREQ_TRACKING_DIST, 
			  100, 0x210, 22*8, gs.trk.rxb1);
	l1_fsk_rx_request(gs.rxtime+16*1000000/100, 1000000/100, gs.rxfreqdev+FREQ_TRACKING_DIST, 
			  100, 0x211, 22*8, gs.trk.rxb2);
#endif /* FREQ_TRACKING */
	while (l1_fsk_wait_request() != 0x200);
	dev = soft_time_dev(gs.rxbuf, 22*8, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
	tmg_add(dev);
#ifdef FREQ_TRACKING
	trk = freq_tracking(gs.rxbuf, gs.trk.rxb1, gs.trk.rxb2, 22*8, RXOVERSAMPLING, &trkm, &trkl, &trkh);
	gtor_freq_tracking(trk, trkl, trkm, trkh);
#endif /* FREQ_TRACKING */
	soft_to_hard(gs.rxbuf, pkt, 22*8, RXOVERSAMPLING, gs.rxinv);
	crc = gtor_calc_crc_ccitt(pkt, 22, get_crc_preset(22));
	if (crc != GTOR_CORRECT_FCS) {
		if (crc == GTOR_INVERTED_FCS_22)
			bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RXBRKIN: inverted!\n");
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RXBRKIN(%c): dev: %ld  data: %02x %02x %02x %02x  crc: %04x\n",
			  gs.golay_flag ? 'G' : 'D', dev, pkt[0], pkt[1], pkt[2], pkt[3], crc);
		return 0;
	}
	fec_clear();
	i = 0x8000 | decode_packet(pkt, 19);
	if (gs.pkt_data_len > gs.last_pkt_cnt) {
		bufwrite(HFAPP_MSG_DATA_RECEIVE, gs.pkt_data+gs.last_pkt_cnt, gs.pkt_data_len - gs.last_pkt_cnt);
		gs.last_pkt_cnt = gs.pkt_data_len;
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RXBRKIN(%c): %s  dT=%ld\n", gs.golay_flag ? 'G' : 'D', gs.pkt_mon, dev);
	return i;
}


static int receive_packet(unsigned int bd)
{
	static const unsigned int inv_fcs[4] = {
		GTOR_INVERTED_FCS_22, GTOR_INVERTED_FCS_24, GTOR_INVERTED_FCS_48, GTOR_INVERTED_FCS_72
	};
	l1_time_t tinc;
	unsigned char pkt[72];
	unsigned short eccpkt[48];
	long dev;
	unsigned int crc;
	int i, j;
#ifdef AUTOMATIC_SPEEDUP
	unsigned short trib_spdup[32];
#endif /* AUTOMATIC_SPEEDUP */
#ifdef FREQ_TRACKING
	l1_soft_t trkm, trkl, trkh;
	int trk = 0;
#endif /* FREQ_TRACKING */

	assert(bd >= 1 && bd <= 3);
	tinc = 1000000/100/RXOVERSAMPLING/bd;
	switch (bd) {
	case 3:
		/* split receive frame into four parts to increase accuracy */
		l1_fsk_rx_request(gs.rxtime, tinc, gs.txfreqdev, 300, 0x203, 18*8*RXOVERSAMPLING, gs.rxbuf);
		l1_fsk_rx_request(gs.rxtime+18*8*1000000/300, tinc, gs.txfreqdev, 300, 0x202, 18*8*RXOVERSAMPLING, 
				  gs.rxbuf+18*8*RXOVERSAMPLING);
		l1_fsk_rx_request(gs.rxtime+36*8*1000000/300, tinc, gs.txfreqdev, 300, 0x201, 18*8*RXOVERSAMPLING, 
				  gs.rxbuf+36*8*RXOVERSAMPLING);
		l1_fsk_rx_request(gs.rxtime+54*8*1000000/300, tinc, gs.txfreqdev, 300, 0x200, 18*8*RXOVERSAMPLING, 
				  gs.rxbuf+54*8*RXOVERSAMPLING);
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(gs.rxtime, 1000000/300, gs.rxfreqdev-FREQ_TRACKING_DIST, 300, 0x210, 
				  72*8, gs.trk.rxb1);
		l1_fsk_rx_request(gs.rxtime, 1000000/300, gs.rxfreqdev+FREQ_TRACKING_DIST, 300, 0x211, 
				  72*8, gs.trk.rxb2);
#endif /* FREQ_TRACKING */
		break;
		
	case 2:
		l1_fsk_rx_request(gs.rxtime, tinc, gs.txfreqdev, 200, 0x200, 48*8*RXOVERSAMPLING, gs.rxbuf);
#ifdef AUTOMATIC_SPEEDUP
		l1_fsk_rx_request(gs.rxtime, 1000000/200, gs.txfreqdev, 300, 0x201, 48*8, gs.spdup.b200.rxb1);
		l1_fsk_rx_request(gs.rxtime+1000000/600, 1000000/200, gs.txfreqdev, 
				  300, 0x202, 48*8, gs.spdup.b200.rxb2);
#endif /* AUTOMATIC_SPEEDUP */
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(gs.rxtime, 1000000/200, gs.rxfreqdev-FREQ_TRACKING_DIST, 200, 0x210, 
				  48*8, gs.trk.rxb1);
		l1_fsk_rx_request(gs.rxtime, 1000000/200, gs.rxfreqdev+FREQ_TRACKING_DIST, 200, 0x211, 
				  48*8, gs.trk.rxb2);
#endif /* FREQ_TRACKING */
		break;

	case 1:
		l1_fsk_rx_request(gs.rxtime, tinc, gs.txfreqdev, 100, 0x200, 24*8*RXOVERSAMPLING, gs.rxbuf);
#ifdef AUTOMATIC_SPEEDUP
		l1_fsk_rx_request(gs.rxtime, 1000000/100, gs.txfreqdev, 200, 0x201, 24*8, gs.spdup.b100.rxb1);
		l1_fsk_rx_request(gs.rxtime+1000000/200, 1000000/100, gs.txfreqdev,
				  200, 0x202, 24*8, gs.spdup.b100.rxb2);
		l1_fsk_rx_request(gs.rxtime, 1000000/100, gs.txfreqdev, 300, 0x203, 24*8, gs.spdup.b100.rxb3);
		l1_fsk_rx_request(gs.rxtime+1000000*2/300, 1000000/100, gs.txfreqdev,
				  300, 0x204, 24*8, gs.spdup.b100.rxb4);
#endif /* AUTOMATIC_SPEEDUP */
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(gs.rxtime, 1000000/100, gs.rxfreqdev-FREQ_TRACKING_DIST, 100, 0x210, 
				  24*8, gs.trk.rxb1);
		l1_fsk_rx_request(gs.rxtime, 1000000/100, gs.rxfreqdev+FREQ_TRACKING_DIST, 100, 0x211, 
				  24*8, gs.trk.rxb2);
#endif /* FREQ_TRACKING */
		break;

	default:
		errprintf(SEV_FATAL, "gtor: receive_packet: invalid baud\n");
	}
	while (l1_fsk_wait_request() != 0x200);
	dev = soft_time_dev(gs.rxbuf, bd*24*8, RXOVERSAMPLING, tinc);
	tmg_add(dev);
#ifdef FREQ_TRACKING
	trk = freq_tracking(gs.rxbuf, gs.trk.rxb1, gs.trk.rxb2, 24*8*bd, RXOVERSAMPLING, &trkm, &trkl, &trkh);
	gtor_freq_tracking(trk, trkl, trkm, trkh);
#endif /* FREQ_TRACKING */
	soft_deinterleave(gs.rxbuf, gs.golay_flag ? gs.rx_par : gs.rx_dat, 16*bd, RXOVERSAMPLING, gs.rxinv);
	if (gs.golay_flag) {
		gs.rx_validmask |= 2;
		trib_parity_to_bytes(gs.rx_par, pkt, 16*bd);
	} else {
		gs.rx_validmask |= 1;
		trib_to_bytes(gs.rx_dat, pkt, 16*bd);
	}
	crc = gtor_calc_crc_ccitt(pkt, 24*bd, get_crc_preset(24*bd));
	if (crc == GTOR_CORRECT_FCS) {
#ifdef AUTOMATIC_SPEEDUP
		switch (bd) {
		case 1:
			soft_deinterleave(gs.spdup.b100.rxb3, trib_spdup, 16, 1, gs.rxinv);
			if (!memcmp(gs.golay_flag ? gs.rx_par : gs.rx_dat, trib_spdup, 16*sizeof(unsigned short))) {
				soft_deinterleave(gs.spdup.b100.rxb4, trib_spdup, 16, 1, gs.rxinv);
				if (!memcmp(gs.golay_flag ? gs.rx_par : gs.rx_dat, trib_spdup, 16*sizeof(unsigned short))) {
					bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX100: speedup possible 300 baud\n");
					break;
				}
			}
			else bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX100: speedup: %04x %04x %04x %04x %04x %04x\n",
				       (gs.golay_flag ? gs.rx_par : gs.rx_dat)[0], trib_spdup[0],
				       (gs.golay_flag ? gs.rx_par : gs.rx_dat)[1], trib_spdup[1],
				       (gs.golay_flag ? gs.rx_par : gs.rx_dat)[2], trib_spdup[2]);
			soft_deinterleave(gs.spdup.b100.rxb1, trib_spdup, 16, 1, gs.rxinv);
			if (!memcmp(gs.golay_flag ? gs.rx_par : gs.rx_dat, trib_spdup, 16*sizeof(unsigned short))) {
				soft_deinterleave(gs.spdup.b100.rxb2, trib_spdup, 16, 1, gs.rxinv);
				if (!memcmp(gs.golay_flag ? gs.rx_par : gs.rx_dat, trib_spdup, 16*sizeof(unsigned short))) {
					bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX100: speedup possible 200 baud\n");
					break;
				}
			}
			break;
			
		case 2:
			soft_deinterleave(gs.spdup.b200.rxb1, trib_spdup, 32, 1, gs.rxinv);
			if (!memcmp(gs.golay_flag ? gs.rx_par : gs.rx_dat, trib_spdup, 32*sizeof(unsigned short))) {
				soft_deinterleave(gs.spdup.b200.rxb2, trib_spdup, 32, 1, gs.rxinv);
				if (!memcmp(gs.golay_flag ? gs.rx_par : gs.rx_dat, trib_spdup, 32*sizeof(unsigned short))) {
					bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX200: speedup possible 300 baud\n");
					break;
				}
			}
			break;

		default:
			break;
		}
#endif /* AUTOMATIC_SPEEDUP */
		fec_clear();
		i = 0x8000 | decode_packet(pkt, 24*bd-3);
		if (gs.pkt_data_len > gs.last_pkt_cnt) {
			bufwrite(HFAPP_MSG_DATA_RECEIVE, gs.pkt_data+gs.last_pkt_cnt, gs.pkt_data_len - gs.last_pkt_cnt);
			gs.last_pkt_cnt = gs.pkt_data_len;
		}
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX%c00(%c): %s  dT=%ld\n", '0'+bd, 
			  gs.golay_flag ? 'G' : 'D', gs.pkt_mon, dev);
		return i;
	}
	if (crc == inv_fcs[bd])
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX%c00(%c): inverted!\n", '0'+bd, 
			  gs.golay_flag ? 'G' : 'D');
	if ((gs.rx_validmask & 3) == 3) {
		j = ecc_packet(gs.rx_dat, gs.rx_par, eccpkt, 16*bd);
		if (j >= 0) {
			trib_to_bytes(eccpkt, pkt, 16*bd);
			crc = gtor_calc_crc_ccitt(pkt, 24*bd, get_crc_preset(24*bd));
			if (crc == GTOR_CORRECT_FCS) {
				fec_clear();
				i = 0x8000 | decode_packet(pkt, 24*bd-3);
				if (gs.pkt_data_len > gs.last_pkt_cnt) {
					bufwrite(HFAPP_MSG_DATA_RECEIVE, gs.pkt_data+gs.last_pkt_cnt, 
						 gs.pkt_data_len - gs.last_pkt_cnt);
					gs.last_pkt_cnt = gs.pkt_data_len;
				}
				bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX%c00(GOLAY %d): %s  dT=%ld\n", '0'+bd, 
					  j, gs.pkt_mon, dev);
				return i;
			}
			if (crc == inv_fcs[bd])
				bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX%c00(%c): inverted!\n", 
					  '0'+bd, gs.golay_flag ? 'G' : 'D');
		}
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX%c00(%c): ECC: %d\n", '0'+bd, gs.golay_flag ? 'G' : 'D', j);
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX%c00(%c): dev: %ld  data: %02x %02x %02x %02x  "
		  "crc: %04x  time: %lu\n", '0'+bd, gs.golay_flag ? 'G' : 'D', dev, pkt[0], pkt[1], pkt[2], pkt[3], crc,
		  gs.rxtime);
	return 0;
}

/* --------------------------------------------------------------------- */

static void disp_status(const char *modename)
{
#if 0
	bufprintf(HFAPP_MSG_DATA_STATUS, "GTOR %s\nTX: %c i:%c g:%c cntr:%d df:%d\n"
		  "RX: i:%c rt:%d df:%d\nCMD:%s%s%s%s\n", modename, gs.is_master ? 'M' : 'S',
		  gs.txinv ? '-' : '+', gs.golay_flag ? 'D' : 'G', 
		  (int)gs.pkt_counter, gs.rxfreqdev, gs.rxinv ? '-' : '+', gs.retry, gs.txfreqdev,
		  (gs.flags & FLG_QRT) ? " QRT" : "", (gs.flags & FLG_BECOMEIRS) ? " IRS" : "", 
		  (gs.flags & FLG_BECOMEISS) ? " ISS" : "", (gs.flags & FLG_SPEEDUP) ? " SPEEDUP" : "");
#endif
#if 1
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR %s  TX: %c i:%c g:%c cntr:%d df:%d"
		  "  RX: i:%c rt:%d df:%d  CMD:%s%s%s%s\n", modename, gs.is_master ? 'M' : 'S',
		  gs.txinv ? '-' : '+', gs.golay_flag ? 'D' : 'G', 
		  (int)gs.pkt_counter, gs.rxfreqdev, gs.rxinv ? '-' : '+', gs.retry, gs.txfreqdev,
		  (gs.flags & FLG_QRT) ? " QRT" : "", (gs.flags & FLG_BECOMEIRS) ? " IRS" : "", 
		  (gs.flags & FLG_BECOMEISS) ? " ISS" : "", (gs.flags & FLG_SPEEDUP) ? " SPEEDUP" : "");
#endif
}

/* --------------------------------------------------------------------- */

static void arq_statemachine(int invalidext)
{
	int i;

	fec_clear();
	tmg_clear();

	gs.last_pkt_cnt = gs.pkt_data_len = 0;
	gs.retry = gp.retry;
	gs.flags = 0;
	gs.txinv = 0;

	if (gs.is_master) {
		send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_MASTERCONNECT, ERR_NOERR);
		goto tx_100_cs1;
	} else {
		gs.pkt_counter = 0;
		send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_SLAVECONNECT, ERR_NOERR);
		if (invalidext)
			goto rx_down_100;
		goto rx_100_cs1;
	}

	/*
	 * 100 baud part
	 */	
rx_down_100:
	send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED100, ERR_NOERR);
	gs.retry = gp.retry;
	for (;;) {
		send_cs(5);
		disp_status("ARQ RX ->100");
		cycle_end();
		i = receive_packet(1);
		if (i & PKT_BRKIN)
			goto rx_tx_100;
		if (i & PKT_QRT)
			goto rx_qrt_cs1;
		if (i) {
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_100;
			goto rx_100_cs1;
		}
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


rx_100_cs1:
	gs.retry = gp.retry;
	for (;;) {
		send_cs(1);
		disp_status("ARQ RX 100 CS1");
		cycle_end();
		i = receive_packet(1);
		if (i & PKT_BRKIN)
			goto rx_tx_100;
		if (i & PKT_QRT)
			goto rx_qrt_cs2;
		if (i & PKT_NEW) {
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_100;
			if (gs.flags & FLG_SPEEDUP2) 
				goto rx_up_300_cs2;
			if (gs.flags & FLG_SPEEDUP) 
				goto rx_up_200_cs2;
			goto rx_100_cs2;
		}
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
rx_100_cs2:
	gs.retry = gp.retry;
	for (;;) {
		send_cs(2);
		disp_status("ARQ RX 100 CS2");
		cycle_end();
		i = receive_packet(1);
		if (i & PKT_BRKIN)
			goto rx_tx_100;
		if (i & PKT_QRT)
			goto rx_qrt_cs1;
		if (i & PKT_NEW) {
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_100;
			if (gs.flags & FLG_SPEEDUP2) 
				goto rx_up_300_cs2;
			if (gs.flags & FLG_SPEEDUP) 
				goto rx_up_200_cs1;
			goto rx_100_cs1;
		}
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
rx_tx_100:
	gs.flags &= ~FLG_BECOMEISS;
	gs.rxtime += (24*8-16)*(1000000/100);
	gs.golay_flag = !gs.golay_flag;
	gs.retry = gp.retry;
	prepare_tx_packet_brkin();
	send_short_msg(HFAPP_MSG_STATE_GTOR_ISS, ERR_NOERR);
	for (;;) {
		send_packet(1);
		disp_status("ARQ RX->TX 100");
		cycle_end();
		i = receive_cs();
		if (i == 2) 
			goto tx_100_cs2;
		else if (i == 3) {
			ack_transmit();
			goto tx_rx_100;
		} else if (i == 1) {
			ack_transmit();
			goto tx_100_cs1;
		}
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_100_cs1:
	gs.retry = gp.retry;
	if (gs.flags & FLG_QRT) 
		goto tx_qrt_cs1;
	prepare_tx_packet_100();
	for (;;) {
		send_packet(1);
		disp_status("ARQ TX 100 CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2 || i == 7) {
			ack_transmit();
			goto tx_100_cs2;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_100;
		} else if (i == 4) {
			ack_transmit();
			goto tx_up_300_cs2;
		} else if (i == 5) {
			ack_transmit();
			goto tx_up_200_cs2;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_100_cs2:
	gs.retry = gp.retry;
	if (gs.flags & FLG_QRT) 
		goto tx_qrt_cs2;
	prepare_tx_packet_100();
	for (;;) {
		send_packet(1);
		disp_status("ARQ TX 100 CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1 || i == 6) {
			ack_transmit();
			goto tx_100_cs1;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_100;
		} else if (i == 4) {
			ack_transmit();
			goto tx_up_300_cs1;
		} else if (i == 5) {
			ack_transmit();
			goto tx_up_200_cs1;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_qrt_cs1:
	gs.retry = GTOR_RETRY_QRT;
	prepare_tx_packet_conndisc(0);
	for (;;) {
		send_packet(1);
		disp_status("ARQ TX QRT CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2 || i == 7) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_NOERR);
			return;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

tx_qrt_cs2:
	gs.retry = GTOR_RETRY_QRT;
	prepare_tx_packet_conndisc(0);
	for (;;) {
		send_packet(2);
		disp_status("ARQ TX QRT CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1 || i == 6) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_NOERR);
			return;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_rx_100:
	gs.flags &= ~FLG_BECOMEIRS;
	gs.last_pkt_cnt = gs.pkt_data_len = 0;
	gs.pkt_counter = 3;
	gs.txtime += (24*8-16)*(1000000/100);
	ack_transmit();
	send_short_msg(HFAPP_MSG_STATE_GTOR_IRS, ERR_NOERR);
	i = receive_bkpacket();
	if (i & PKT_BRKIN)
		goto rx_tx_100;
	if (i) 
		goto rx_100_cs1;
	goto rx_100_cs2;


tx_down_100:
	send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED100, ERR_NOERR);
	gs.retry = gp.retry;
	prepare_tx_packet_100();
	for (;;) {
		send_packet(1);
		disp_status("ARQ TX ->100");
		cycle_end();
		i = receive_cs();
		if (i == 1) {
			ack_transmit();
			goto tx_100_cs1;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_100;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}
	

rx_qrt_cs1:
	send_cs((gs.qrt_cs = 1));
	while (l1_fsk_wait_request() != 0x101);
	send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_NOERR);
	return;

rx_qrt_cs2:
	send_cs((gs.qrt_cs = 2));
	while (l1_fsk_wait_request() != 0x101);
	send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_NOERR);
	return;

	/*
	 * 200 baud part
	 */
rx_up_200_cs1:
	gs.flags &= ~FLG_SPEEDUP;
	gs.retry = GTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(5);
		disp_status("ARQ RX ->200 CS1");
		cycle_end();
		i = receive_packet(2);
		if (i & PKT_BRKIN) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
			goto rx_tx_200;
		}
		if (i & PKT_QRT)
			goto rx_100_cs1;
		if (i) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			goto rx_200_cs2;
		} else if (retry())
			goto rx_100_cs1;
	}
	

rx_up_200_cs2:
	gs.flags &= ~FLG_SPEEDUP;
	gs.retry = GTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(5);
		disp_status("ARQ RX ->200 CS2");
		cycle_end();
		i = receive_packet(2);
		if (i & PKT_BRKIN) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
			goto rx_tx_200;
		}
		if (i & PKT_QRT)
			goto rx_100_cs2;
		if (i) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			goto rx_200_cs1;
		} else if (retry())
			goto rx_100_cs2;
	}
	

tx_up_200_cs1:
	gs.flags &= ~FLG_SPEEDUP;
	gs.retry = gp.retry;
	prepare_tx_packet_200();
	for (;;) {
		send_packet(2);
		disp_status("ARQ TX ->200 CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
			goto tx_200_cs2;
	        } else if (i == 1)
			goto tx_100_cs1;
		else if (i == 3) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
			goto tx_rx_200;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
tx_up_200_cs2:
	gs.flags &= ~FLG_SPEEDUP;
	gs.retry = gp.retry;		
	prepare_tx_packet_200();
	for (;;) {
		send_packet(2);
		disp_status("ARQ TX ->200 CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
			goto tx_200_cs1;
	        } else if (i == 2)
			goto tx_100_cs2;
		else if (i == 3) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
			goto tx_rx_200;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
rx_down_200:
	send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
	gs.retry = GTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(4);
		disp_status("ARQ RX ->200");
		cycle_end();
		i = receive_packet(2);
		if (i & PKT_BRKIN)
			goto rx_tx_200;
		if (i & PKT_QRT)
			goto rx_down_100;
		if (i) {
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			goto rx_200_cs1;
		}
		if (retry())
			goto rx_down_100;
	}


tx_down_200:
	send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED200, ERR_NOERR);
	gs.retry = gp.retry;
	prepare_tx_packet_200();
	for (;;) {
		send_packet(2);
		disp_status("ARQ TX ->200");
		cycle_end();
		i = receive_cs();
		if (i == 1) {
			ack_transmit();
			goto tx_200_cs1;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_100;
		} else if (i == 5)
			goto tx_down_100;
		else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}
	

rx_200_cs1:
	gs.retry = GTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(1);
		disp_status("ARQ RX 200 CS1");
		cycle_end();
		i = receive_packet(2);
		if (i & PKT_BRKIN)
			goto rx_tx_200;
		if (i & PKT_QRT)
			goto rx_down_100;
		if (i & PKT_NEW) {
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			if (gs.flags & FLG_SPEEDUP2) 
				goto rx_up_300_cs2;
			goto rx_200_cs2;
		} else if (retry())
			goto rx_down_100;
	}

	
rx_200_cs2:
	gs.retry = GTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(2);
		disp_status("ARQ RX 200 CS2");
		cycle_end();
		i = receive_packet(2);
		if (i & PKT_BRKIN)
			goto rx_tx_200;
		if (i & PKT_QRT)
			goto rx_down_100;
		if (i & PKT_NEW) {
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			if (gs.flags & FLG_SPEEDUP2) 
				goto rx_up_300_cs1;
			goto rx_200_cs1;
		} else if (retry())
			goto rx_down_100;
	}

	
rx_tx_200:
	gs.flags &= ~FLG_BECOMEISS;
	gs.pkt_counter = 0;
	gs.rxtime += (24*8-16)*(1000000/100);
	gs.golay_flag = !gs.golay_flag;
	gs.retry = gp.retry;
	prepare_tx_packet_brkin();
	send_short_msg(HFAPP_MSG_STATE_GTOR_ISS, ERR_NOERR);
	for (;;) {
		send_packet(1);
		disp_status("ARQ RX->TX 200");
		cycle_end();
		i = receive_cs();
		if (i == 2)
			goto tx_200_cs2;
		else if (i == 3) {
			ack_transmit();
			goto tx_rx_200;
		} else if (i == 1) {
			ack_transmit();
			goto tx_200_cs1;
		} else if (i == 5) 
			goto tx_down_100;
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_200_cs1:
	gs.retry = gp.retry;
	prepare_tx_packet_200();
	for (;;) {
		send_packet(2);
		disp_status("ARQ TX 200 CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2 || i == 7) {
			ack_transmit();
			goto tx_200_cs2;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_200;
		} else if (i == 4) {
			ack_transmit();
			goto tx_up_300_cs2;
		} else if (i == 5) 
			goto tx_down_100;
		else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_200_cs2:
	gs.retry = gp.retry;		
	prepare_tx_packet_200();
	for (;;) {
		send_packet(2);
		disp_status("ARQ TX 200 CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1 || i == 6) {
			ack_transmit();
			goto tx_200_cs1;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_200;
		} else if (i == 4) {
			ack_transmit();
			goto tx_up_300_cs1;
		} else if (i == 5) 
			goto tx_down_100;
		else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_rx_200:
	gs.flags &= ~FLG_BECOMEIRS;
	gs.last_pkt_cnt = gs.pkt_data_len = 0;
	gs.pkt_counter = 3;
	gs.txtime += (24*8-16)*(1000000/100);
	ack_transmit();
	send_short_msg(HFAPP_MSG_STATE_GTOR_IRS, ERR_NOERR);
	i = receive_bkpacket();
	if (i & PKT_BRKIN)
		goto rx_tx_200;
	if (i) 
		goto rx_200_cs1;
	goto rx_200_cs2;


	/*
	 * 300 baud part
	 */
rx_up_300_cs1:
	gs.flags &= ~(FLG_SPEEDUP|FLG_SPEEDUP2);
	gs.retry = GTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(4);
		disp_status("ARQ RX ->300 CS1");
		cycle_end();
		i = receive_packet(3);
		if (i & PKT_BRKIN) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED300, ERR_NOERR);
			goto rx_tx_300;
		}
		if (i & PKT_QRT)
			goto rx_up_200_cs1;
		if (i) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED300, ERR_NOERR);
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_300;
			goto rx_300_cs2;
		} else if (retry())
			goto rx_200_cs1; /* rx_up_200_cs1 */
	}
	

rx_up_300_cs2:
	gs.flags &= ~(FLG_SPEEDUP|FLG_SPEEDUP2);
	gs.retry = GTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(4);
		disp_status("ARQ RX ->300 CS2");
		cycle_end();
		i = receive_packet(3);
		if (i & PKT_BRKIN) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED300, ERR_NOERR);
			goto rx_tx_300;
		}
		if (i & PKT_QRT)
			goto rx_up_200_cs2;
		if (i) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED300, ERR_NOERR);
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_300;
			goto rx_300_cs1;
		} else if (retry())
			goto rx_200_cs2; /* rx_up_200_cs2 */
	}
	

tx_up_300_cs1:
	gs.flags &= ~(FLG_SPEEDUP|FLG_SPEEDUP2);
	gs.retry = gp.retry;
	prepare_tx_packet_300();
	for (;;) {
		send_packet(3);
		disp_status("ARQ TX ->300 CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED300, ERR_NOERR);
			goto tx_300_cs2;
	        } else if (i == 1)
			goto tx_200_cs1;
		else if (i == 3) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED300, ERR_NOERR);
			goto tx_rx_300;
		} else if (i == 5)
			goto tx_up_200_cs1;
		else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
tx_up_300_cs2:
	gs.flags &= ~(FLG_SPEEDUP|FLG_SPEEDUP2);
	gs.retry = gp.retry;		
	prepare_tx_packet_300();
	for (;;) {
		send_packet(3);
		disp_status("ARQ TX ->300 CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED300, ERR_NOERR);
			goto tx_300_cs1;
	        } else if (i == 2)
			goto tx_200_cs2;
		else if (i == 3) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_GTOR_SPEED300, ERR_NOERR);
			goto tx_rx_300;
		} else if (i == 5)
			goto tx_up_200_cs2;
		else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


rx_300_cs1:
	gs.retry = GTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(1);
		disp_status("ARQ RX 300 CS1");
		cycle_end();
		i = receive_packet(3);
		if (i & PKT_BRKIN)
			goto rx_tx_300;
		if (i & PKT_QRT)
			goto rx_down_100;
		if (i & PKT_NEW) {
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_300;
			goto rx_300_cs2;
		} else if (retry())
			goto rx_down_200;
	}

	
rx_300_cs2:
	gs.retry = GTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(2);
		disp_status("ARQ RX 300 CS2");
		cycle_end();
		i = receive_packet(3);
		if (i & PKT_BRKIN)
			goto rx_tx_300;
		if (i & PKT_QRT)
			goto rx_down_100;
		if (i & PKT_NEW) {
			if (gs.flags & FLG_BECOMEISS)
				goto rx_tx_300;
			goto rx_300_cs1;
		} else if (retry())
			goto rx_down_200;
	}

	
rx_tx_300:
	gs.flags &= ~FLG_BECOMEISS;
	gs.pkt_counter = 0;
	gs.rxtime += (24*8-16)*(1000000/100);
	gs.golay_flag = !gs.golay_flag;
	gs.retry = gp.retry;
	prepare_tx_packet_brkin();
	send_short_msg(HFAPP_MSG_STATE_GTOR_ISS, ERR_NOERR);
	for (;;) {
		send_packet(1);
		disp_status("ARQ RX->TX 300");
		cycle_end();
		i = receive_cs();
		if (i == 2)
			goto tx_300_cs2;
		else if (i == 3) {
			ack_transmit();
			goto tx_rx_300;
		} else if (i == 1) {
			ack_transmit();
			goto tx_300_cs1;
		} else if (i == 4)
			goto tx_down_200;
		else if (i == 5) 
			goto tx_down_100;
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_300_cs1:
	gs.retry = gp.retry;
	prepare_tx_packet_300();
	for (;;) {
		send_packet(3);
		disp_status("ARQ TX 300 CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2 || i == 7) {
			ack_transmit();
			goto tx_300_cs2;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_300;
		} else if (i == 4)
			goto tx_down_200;
		else if (i == 5) 
			goto tx_down_100;
		else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_300_cs2:
	gs.retry = gp.retry;		
	prepare_tx_packet_300();
	for (;;) {
		send_packet(3);
		disp_status("ARQ TX 300 CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1 || i == 6) {
			ack_transmit();
			goto tx_300_cs1;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_300;
		} else if (i == 4)
			goto tx_down_200;
		else if (i == 5) 
			goto tx_down_100;
		else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_rx_300:
	gs.flags &= ~FLG_BECOMEIRS;
	gs.last_pkt_cnt = gs.pkt_data_len = 0;
	gs.pkt_counter = 3;
	gs.txtime += (24*8-16)*(1000000/100);
	ack_transmit();
	send_short_msg(HFAPP_MSG_STATE_GTOR_IRS, ERR_NOERR);
	i = receive_bkpacket();
	if (i & PKT_BRKIN)
		goto rx_tx_300;
	if (i) 
		goto rx_300_cs1;
	goto rx_300_cs2;

}

/* --------------------------------------------------------------------- */

int gtor_monitor(unsigned int bd, l1_soft_t *cur, l1_soft_t *prev, l1_time_t tm, unsigned int ovrsampl, int freqdev)
{
	unsigned char pkt[72];
	unsigned short tribd[48];
	unsigned short tribp[48];
	unsigned short eccpkt[48];
	unsigned int crc;
	int i, eccst = -1;
	long dev;

	soft_deinterleave(cur, tribd, 16*bd, ovrsampl, 0);
	trib_to_bytes(tribd, pkt, 16*bd);
	crc = gtor_calc_crc_ccitt(pkt, 24*bd, get_crc_preset(24*bd));
	gs.rxinv = gs.golay_flag = 0;
	if (crc == inv_fcs[bd]) {
		crc = GTOR_CORRECT_FCS;
		gs.rxinv = 1;
		invert(pkt, 24*bd);
	} else if (crc != GTOR_CORRECT_FCS) {
		trib_parity_to_bytes(tribd, pkt, 16*bd);
		gs.golay_flag = 1;
		crc = gtor_calc_crc_ccitt(pkt, 24*bd, get_crc_preset(24*bd));
		if (crc == inv_fcs[bd]) {
			crc = GTOR_CORRECT_FCS;
			gs.rxinv = 1;
			invert(pkt, 24*bd);
		}
	}
	if (crc != GTOR_CORRECT_FCS && prev) {
		soft_deinterleave(prev, tribp, 16*bd, ovrsampl, 0);
		gs.golay_flag = 0;
		eccst = ecc_packet(tribd, tribp, eccpkt, 16*bd);
		if (eccst >= 0) {
			trib_to_bytes(eccpkt, pkt, 16*bd);
			crc = gtor_calc_crc_ccitt(pkt, 24*bd, get_crc_preset(24*bd));
			if (crc == inv_fcs[bd]) {
				crc = GTOR_CORRECT_FCS;
				gs.rxinv = 1;
				invert(pkt, 24*bd);
			}
		}
		if (crc != GTOR_CORRECT_FCS) {
			gs.golay_flag = 1;
			eccst = ecc_packet(tribd, tribp, eccpkt, 16*bd);
			if (eccst >= 0) {
				trib_to_bytes(eccpkt, pkt, 16*bd);
				crc = gtor_calc_crc_ccitt(pkt, 24*bd, get_crc_preset(24*bd));
				if (crc == inv_fcs[bd]) {
					crc = GTOR_CORRECT_FCS;
					gs.rxinv = 1;
					invert(pkt, 24*bd);
				}
			}
		}
	}
	if (crc != GTOR_CORRECT_FCS)
		return 0;
	dev = soft_time_dev(cur, bd*24*8, ovrsampl, 1000000/100/bd/ovrsampl);
	i = decode_packet(pkt, 24*bd-3);
	if (gs.pkt_data_len > gs.last_pkt_cnt) {
		bufwrite(HFAPP_MSG_DATA_RECEIVE, gs.pkt_data+gs.last_pkt_cnt, gs.pkt_data_len - gs.last_pkt_cnt);
		gs.last_pkt_cnt = gs.pkt_data_len;
	}
	if (eccst >= 0)
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX%c00 GOLAY(%d): %s  dT=%ld\n", '0'+bd, eccst, gs.pkt_mon, dev);
	else
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX%c00: %s  dT=%ld\n", '0'+bd, gs.pkt_mon, dev);
	if (bd != 1)
		return 0;
	if (!(i & PKT_CALL))
		return 0;
	if (memcmp(gs.pkt_mycall, gp.mycall, 10))
		return 0;
	gs.is_master = 0;
	gs.txtime = (gs.rxtime = tm+dev) + 1920000 + gp.txdelay + 10000;
	gs.rxfreqdev = gs.txfreqdev = freqdev;
	l1_fsk_clear_requests();
	arq_statemachine(pkt[21] & 0x30);
	return 1;
}

/* --------------------------------------------------------------------- */

static int master_verify_cs(int bsy)
{
	unsigned char csbuf[2];
	unsigned int csbits;
	long dev;

	l1_fsk_tx_request(gs.txtime, 1000000/100, gs.txfreqdev, gs.txinv, 0x101, 8*24, 
			  gs.golay_flag ? gs.txb_par : gs.txb_dat);
	l1_fsk_tx_request(gs.txtime+GTOR_CYCLE_ARQ-gp.txdelay, gp.txdelay, gs.rxfreqdev,
			  0, 0x100, 1, gs.txb_dat);
	l1_fsk_rx_request(gs.rxtime, 1000000/100/RXOVERSAMPLING, gs.rxfreqdev, 
			  100, 0x200, 16*RXOVERSAMPLING, gs.rxbuf);
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: TX(%c): %s\n", gs.golay_flag ? 'G' : 'D', gs.pkt_mon);
	while (l1_fsk_wait_request() != 0x200);
	soft_to_hard(gs.rxbuf, csbuf, 16, RXOVERSAMPLING, gs.rxinv);
	csbits = (csbuf[0] | (csbuf[1] << 8)) & 0xffff;
	dev = soft_time_dev(gs.rxbuf, 16, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
	if (csbits == (bsy ? GTOR_CS2 : GTOR_CS1)) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS%c  dT=%ld\n", bsy ? '2' : '1', dev);
		if (bsy)
			return -1;
		return 1;
	}
	if (csbits == (bsy ? (0xffff ^ GTOR_CS2) : (0xffff ^ GTOR_CS1))) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS%c inverted!!\n", bsy ? '2' : '1');
		return 0;
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS?  bits=%03x\n", csbits);
	return 0;
}

/* --------------------------------------------------------------------- */

static int master_call(void)
{
	unsigned int nsamps = (GTOR_CYCLE_ARQ-1920000-gp.txdelay)/(1000000/100)*RXOVERSAMPLING;
	int i;
	l1_soft_t *s;
	unsigned char csbuf[2];
	unsigned int cs;

	gs.txinv = 0;
	gs.txtime = l1_get_current_time() + 200000;
	gs.rxfreqdev = gs.txfreqdev = 0;
	prepare_tx_packet_conndisc(1);
	kbd_clear_and_fill(NULL, 0);
	gs.retry = gp.retry;
	gs.is_master = 1;
	l1_fsk_tx_request(gs.txtime-gp.txdelay, gp.txdelay, gs.txfreqdev, 0, 0x100, 1, gs.txb_dat);
	for (;;) {
		l1_fsk_tx_request(gs.txtime, 1000000/100, gs.txfreqdev, gs.txinv, 0x101, 8*24, 
				  gs.golay_flag ? gs.txb_par : gs.txb_dat);
		l1_fsk_tx_request(gs.txtime+GTOR_CYCLE_ARQ-gp.txdelay, gp.txdelay, gs.txfreqdev,
				  0, 0x100, 1, gs.txb_dat);
		gs.rxtime = gs.txtime+1920000;
		l1_fsk_rx_request(gs.rxtime, 1000000/100/RXOVERSAMPLING, gs.txfreqdev, 100, 0x200, nsamps, gs.rxbuf);
		bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: TX(%c): %s\n", gs.golay_flag ? 'G' : 'D', gs.pkt_mon);
		disp_status("ARQ MASTER CALL");
		while (l1_fsk_wait_request() != 0x200);
		for (s = gs.rxbuf, i = 0; i < nsamps-RXOVERSAMPLING*(16-1); 
		     i++, s++, gs.rxtime += 1000000/100/RXOVERSAMPLING) {
			soft_to_hard(s, csbuf, 16, RXOVERSAMPLING, 0);
			cs = (csbuf[0] | (csbuf[1] << 8)) & 0xffff;
			gs.rxinv = (cs == (0xffff ^ GTOR_CS1)) || (cs == (0xffff ^ GTOR_CS2));
			if (gs.rxinv)
				cs ^= 0xffff;
			if ((cs == GTOR_CS1) || (cs == GTOR_CS2)) {
				gs.rxtime += GTOR_CYCLE_ARQ+soft_time_dev(s, 16, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
				gs.txtime += GTOR_CYCLE_ARQ;
				gs.golay_flag = !gs.golay_flag;
				bufprintf(HFAPP_MSG_DATA_MONITOR, "GTOR: RX: CS%c  dT=%ld\n", (cs == GTOR_CS2) ? '2' : '1',
					  gs.rxtime-gs.txtime-1920000);
				i = master_verify_cs((cs == GTOR_CS2));
				if (i == -1)
					return -1;
				if (i) {
					gs.txtime += GTOR_CYCLE_ARQ;
					gs.golay_flag = !gs.golay_flag;
					gs.pkt_counter = 1;
					return 0;
				}
			}
		}
		if ((--gs.retry) <= 0)
			return -1;
		gs.txtime += GTOR_CYCLE_ARQ;
		gs.golay_flag = !gs.golay_flag;
	}
}

/* --------------------------------------------------------------------- */

void *mode_gtor_arq(void *dummy)
{
	int i;

	l1_fsk_clear_requests();
	errprintf(SEV_INFO, "mode: gtor arq call\n");
	i = master_call();
	if (i >= 0)
		arq_statemachine(0);
	else
	    send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_MASTERCONNECT, ERR_TIMEOUT);
	    send_short_msg(HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
	return NULL;
}

/* --------------------------------------------------------------------- */
