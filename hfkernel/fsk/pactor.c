/*****************************************************************************/

/*
 *      pactor.c  --  Pactor 1 protocol.
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
#include <assert.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <stdio.h>

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"
#include "pactor.h"
#include "standby.h"

/* --------------------------------------------------------------------- */
/*
 * Pactor defines
 */

#define PACTOR_CYCLE_SP           1250000L      /* 1250ms */
#define PACTOR_CYCLE_LP           1400000L      /* 1400ms */
#define PACTOR_CYCLE_STREAM_FEC    960000L      /* 960ms */
#define PACTOR_CYCLE_ARQ          (ps.is_longpath ? PACTOR_CYCLE_LP : PACTOR_CYCLE_SP)
#define PACTOR_HEADER1            0x55
#define PACTOR_HEADER2            0xaa
#define PACTOR_CS1                0x4d5
#define PACTOR_CS2                0xab2
#define PACTOR_CS3                0x34b
#define PACTOR_CS4                0xd2c
#define PACTOR_INV_CS1            (0xfff^PACTOR_CS1)
#define PACTOR_INV_CS2            (0xfff^PACTOR_CS2)
#define PACTOR_INV_CS3            (0xfff^PACTOR_CS3)
#define PACTOR_INV_CS4            (0xfff^PACTOR_CS4)
#define PACTOR_IDLE               0x1e          /* idle code */
#define PACTOR_SB                 0x1c
#define PACTOR_SIC_LEVEL          'A'
#define PACTOR_SIC_MYCALL         'B'
#define PACTOR_CORRECT_FCS        0xf47
#define PACTOR_INVERTED_FCS_10    0x14a5
#define PACTOR_INVERTED_FCS_11    0xc438
#define PACTOR_INVERTED_FCS_21    0x1d9c
#define PACTOR_INVERTED_FCS_23    0x77ff
#define PACTOR_QRT_HEADER         0x55
#define PACTOR_CALL_HEADER        0x55

#define PACTOR_RETRY_QRT          4
#define PACTOR_RETRY_CALL         20
#define PACTOR_RETRY_QSO          80
#define PACTOR_RETRY_HISPEED      4
#define PACTOR_RETRY_FEC          2

/* --------------------------------------------------------------------- */

#define RXOVERSAMPLING 8
#define FREQ_TRACKING_DIST 10 /* Hz */

#define AUTOMATIC_SPEEDUP
#define FREQ_TRACKING

#undef STANDBY_CRC_PRINT
#undef DEBUG_STATUSDISPLAY
/* --------------------------------------------------------------------- */

static struct {
	unsigned char destcall[8];
	unsigned char mycall[8];
	l1_time_t txdelay;
	unsigned int retry;
	unsigned int longpath;
	unsigned int crc_preset[4];
} pp = { 
	"\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f", "\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f", 
	30000, 30, 0, { 0xffff, 0xffff, 0xffff, 0xffff } 
};

static struct {
	/*
	 * written by decode_packet
	 */
	unsigned char pkt_mon[64];
	unsigned char pkt_data[64];
	unsigned int pkt_mon_len;
	unsigned int pkt_data_len;
	unsigned char pkt_qrtcall[7];

	unsigned char pkt_counter;
	/*
	 * misc state
	 */
	int is_master;
	int is_longpath;
	int retry;
	int rxinv, txinv;
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
		long devflt[8];
		unsigned int ptr;
	} tm;

	struct {
		l1_soft_t acc[23*8];
		unsigned char hdr;
	} marq;

	/*
	 * tx/rx storage
	 */
	unsigned char txdata[23*8];
	unsigned char txbuf[24*8];
	l1_soft_t rxbuf[24*8*RXOVERSAMPLING];
#ifdef AUTOMATIC_SPEEDUP
	struct {
		l1_soft_t rxb1[12*8];
		l1_soft_t rxb2[12*8];
	} spd;
#endif /* AUTOMATIC_SPEEDUP */
#ifdef FREQ_TRACKING
	struct {
		l1_soft_t rxb1[24*8];
		l1_soft_t rxb2[24*8];
	} trk;
#endif /* FREQ_TRACKING */
} ps;

/* --------------------------------------------------------------------- */
/* 
 * pactor specific utility functions
 */

static struct huffman {
        unsigned short len;
        unsigned short code;
} huffman[] = {{15,0x31cf},{15,0x51cf},{15,0x11cf},{15,0x61cf},
               {15,0x21cf},{15,0x41cf},{15, 0x1cf},{15,0x7f0f},
               {15,0x3f0f},{15,0x5f0f},{ 6,  0x2c},{15,0x1f0f},
               {15,0x6f0f},{ 6,   0xc},{15,0x2f0f},{15,0x4f0f},
               {15,0x770f},{15,0x370f},{15,0x570f},{15,0x170f},
               {15,0x670f},{15,0x270f},{15,0x470f},{15, 0x70f},
               {15,0x7b0f},{15,0x3b0f},{15,0x71cf},{15,0x5b0f},
               {10, 0x263},{15,0x1b0f},{15, 0xf0f},{15,0x7f63},
               { 2,   0x1},{11, 0x5cf},{12, 0x363},{13,0x1b14},
               {13,0x13a8},{12, 0xb63},{12, 0x9cf},{12, 0x763},
               { 9, 0x1b3},{ 9,  0x73},{12, 0x314},{12, 0x7cf},
               { 7,  0x53},{11, 0x7a8},{ 7,  0x13},{11, 0x3cf},
               { 8, 0x0e3},{ 9,  0x14},{10, 0x168},{10, 0x368},
               {10, 0x0e8},{10, 0x2a8},{10, 0x2e8},{10, 0x1e8},
               {10, 0x3e8},{10, 0x128},{11, 0x714},{14, 0xb0f},
               {13, 0xba8},{10, 0x10f},{12, 0xfcf},{10, 0x2b3},
               {13, 0x3a8},{ 8,  0x94},{ 8,  0xf3},{ 8,  0x8f},
               { 7,  0x58},{ 8,   0x3},{ 8,  0x33},{11, 0x728},
               {10, 0x114},{ 8,  0x4f},{10, 0x183},{10,  0xb3},
               { 9, 0x173},{ 9,  0xaf},{ 9,   0xf},{ 9,  0x28},
               { 9,  0x68},{10, 0x3af},{ 9,  0x83},{ 7,  0x6f},
               { 8,  0x2f},{10, 0x383},{10,  0x63},{10,  0xa8},
               {13,0x1ba8},{14,0x2b14},{ 9,  0xcf},{15, 0xb14},
               {14,0x2b0f},{15,0x4b14},{15,0x3f63},{12, 0x30f},
               {15,0x5f63},{ 5,   0x2},{ 7,  0x30},{ 6,  0x32},
               { 5,  0x1c},{ 3,   0x6},{ 7,  0x70},{ 6,  0x38},
               { 6,   0x8},{ 4,   0xb},{11, 0x328},{ 7,  0x54},
               { 6,  0x10},{ 6,  0x34},{ 4,   0xa},{ 6,  0x12},
               { 8,  0x43},{10, 0x1af},{ 4,   0x7},{ 5,   0x4},
               { 5,   0x0},{ 5,  0x1f},{ 8,  0xc3},{ 7,  0x18},
               {10, 0x163},{10, 0x1a8},{ 7,  0x23},{15,0x1f63},
               {15,0x6f63},{15,0x2f63},{15,0x4f63},{15, 0xf63}};

/* --------------------------------------------------------------------- */

static unsigned char conv200to100(unsigned char b1, unsigned char b2)
{
        unsigned val = (b2 << 8) | b1, maskv = 1;
        unsigned char retval = 0, maskr = 1;
        int cnt;

        for (cnt = 8; cnt > 0; cnt--, maskv <<= 2, maskr <<= 1)
                if (val & maskv)
                        retval |= maskr;
        return retval;
}

/* --------------------------------------------------------------------- */

#define FLG_QRT         (1<<0)
#define FLG_BECOMEISS   (1<<1)
#define FLG_BECOMEIRS   (1<<2)
#define FLG_SPEEDUP     (1<<3)

/*
 * this flags purpose is to delay the output of 200 baud receive data
 * to prevent double data in case of switch back to 100 baud... hairy...
 */
#define FLG_VALIDDATA   (1<<31)

void pactor_mode_qrt(void)
{
	ps.flags |= FLG_QRT;
}

void pactor_mode_irs(void)
{
	ps.flags |= FLG_BECOMEIRS;
}

void pactor_mode_iss(void)
{
	ps.flags |= FLG_BECOMEISS;
}

void pactor_mode_speedup(void)
{
	ps.flags |= FLG_SPEEDUP;
}

/* --------------------------------------------------------------------- */

extern __inline__ unsigned int get_crc_preset(int pktlen)
{
	switch (pktlen) {
	case 23:
		return pp.crc_preset[3];

	case 21:
		return pp.crc_preset[2];

	case 10:
		return pp.crc_preset[0];

	default:
		return pp.crc_preset[1];
	}
}

/* --------------------------------------------------------------------- */

#define PKT_NEW   (1<<0)
#define PKT_BRKIN (1<<1)
#define PKT_QRT   (1<<2)
#define PKT_IDLE  (1<<3)

extern __inline__ void decode_qrt_packet(unsigned char *data, int pktlen)
{
	unsigned char buf[8];
	unsigned char *bp2;
	int i;
	char c;

	if (pktlen >= 20) {
		switch (conv200to100(data[15], data[16])) {
		case PACTOR_HEADER1:
			c = 'v';
			break;
		
		case PACTOR_HEADER2:
			c = '^';
			break;
                        
		default:
			c = '?';
			break;
		}
		for (i = 0, bp2 = data + 13; i < 7; i++, bp2 -= 2) 
			if ((ps.pkt_qrtcall[i] = buf[i] = conv200to100(bp2[0], bp2[1])) < ' ') 
				buf[i] = '\0';
		buf[7] = 0;
	} else {
		switch (data[7]) {
		case PACTOR_HEADER1:
			c = 'v';
			break;
		
		case PACTOR_HEADER2:
			c = '^';
			break;
		
		default:
			c = '?';
			break;
		}
		for (i = 0, bp2 = data + 6; i < 7; i++, bp2--)
			if ((ps.pkt_qrtcall[i] = buf[i] = *bp2) < ' ') 
				buf[7] = 0;
	}
	ps.pkt_mon_len = snprintf(ps.pkt_mon, sizeof(ps.pkt_mon), "QRT: BD:%c00 C:%1d H:%c CALL:%.7s", 
				  '1' + (pktlen >= 16), ps.pkt_counter, c, buf);
}

static int decode_packet(unsigned char *data, int pktlen)
{
        unsigned char stat = data[pktlen-3];
        int retval = 0;
        struct huffman *huff;
        int i;
        char buf[64];
        unsigned char *bp, *pp;
        int bitptr, pktbitlen;
        unsigned bitbuf;
        static const unsigned short bitmasks[] = { 
                0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,
                0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff 
	};
                                                           
        if ((stat & 0x03) != ps.pkt_counter) {
                retval |= PKT_NEW;
                ps.pkt_counter = stat & 0x03;
		if (ps.last_pkt_cnt <= ps.pkt_data_len)
			ps.last_pkt_cnt = 0;
		else
			ps.last_pkt_cnt -= ps.pkt_data_len;
        }
        ps.pkt_data[0] = '\0';
	ps.pkt_data_len = 0;
        ps.pkt_mon[0] = '\0';
	ps.pkt_mon_len = 0;
        if (stat & 0x40)
                retval |= PKT_BRKIN;
        if (stat & 0x80) {
		if (pktlen != 23 && pktlen != 11) {
			ps.pkt_mon_len = snprintf(ps.pkt_mon, sizeof(ps.pkt_mon), "PKT: BD:%c00 C:%1d INVALID "
						  "QRT PACKET LENGTH", '1' + (pktlen >= 16), ps.pkt_counter);
			return retval;
		}
		decode_qrt_packet(data, pktlen);
                return retval | PKT_QRT;
        }
        bp = buf;
        switch (stat & 0x0c) {
        case 0x0:
                pp = data;
                for (i = pktlen-3; i > 0; i--, pp++)
                        if (*pp != PACTOR_IDLE) {
				ps.pkt_data[ps.pkt_data_len++] = *pp;
				*bp++ = ((*pp) >= ' ') ? *pp : '.';
                        }
                break;
		
        case 0x4:
                bitptr = 0;
                pktbitlen = (pktlen - 3) * 8;
                while (bitptr < pktbitlen) {
                        pp = data + (bitptr >> 3);
                        bitbuf = (pp[2] << 16) | (pp[1] << 8) | pp[0];
                        bitbuf >>= bitptr & 7;
                        for (i = 0, huff = huffman; (i < 128) && 
				     (((bitbuf & bitmasks[huff->len]) != huff->code) || 
				      (bitptr + huff->len > pktbitlen)); i++, huff++);
                        if (i > 127)
                                break;
			switch (i) {
                        case PACTOR_IDLE:
                                break;
                        case 14:
                                ps.pkt_data[ps.pkt_data_len++] = *bp++ = 0204;
                                break;
                       
			case 15:
                                ps.pkt_data[ps.pkt_data_len++] = *bp++ = 0224;
                                break;
                        
			case 16: 
                                ps.pkt_data[ps.pkt_data_len++] = *bp++ = 0201;
				break;
   
			case 20: 
                                ps.pkt_data[ps.pkt_data_len++] = *bp++ = 0216;
                                break;
                        
			case 21:
                                ps.pkt_data[ps.pkt_data_len++] = *bp++ = 0231;
                                break;
                        
			case 22:
                                ps.pkt_data[ps.pkt_data_len++] = *bp++ = 0232;
                                break;
                        
			case 23:
                                ps.pkt_data[ps.pkt_data_len++] = *bp++ = 0341;
                                break;
                        
			default:
                                ps.pkt_data[ps.pkt_data_len++] = i;
                                *bp++ = (i >= 32) ? i : '.';
                                break;
                        }
                        bitptr += huff->len;
                }
                break;
		
        default:
                ps.pkt_data_len = 0;
                ps.pkt_mon_len = snprintf(ps.pkt_mon, sizeof(ps.pkt_mon), "PKT: BD:%c00 C:%1d UNKNOWN COMPRESSION"
					 " METHOD %d", '1' + (pktlen >= 16), ps.pkt_counter, (stat & 0x0c) >> 2);
                return retval;
        }
        if (ps.pkt_data_len <= 0)
                retval |= PKT_IDLE; 
	else if (ps.pkt_data_len > ps.last_pkt_cnt) {
		bufwrite(HFAPP_MSG_DATA_RECEIVE, ps.pkt_data + ps.last_pkt_cnt, ps.pkt_data_len-ps.last_pkt_cnt);
		ps.last_pkt_cnt = ps.pkt_data_len;
	}

	*bp = 0;
        ps.pkt_mon_len = snprintf(ps.pkt_mon, sizeof(ps.pkt_mon), "PKT: BD:%c00 C:%1d DATA:\"%.64s\"%s", 
				  '1' + (pktlen >= 16), ps.pkt_counter, buf, 
				  (retval & PKT_BRKIN) ? " BREAKIN" : "");
        return retval;
}

/* --------------------------------------------------------------------- */

static const unsigned dbl_tab[] = {  
	0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
	0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff 
};


extern __inline__ void encode_qrt_packet(int pktlen)
{
        unsigned char *p1, *p2;
        unsigned int crc;
        char buf[8];
        int i;

	if (pktlen >= 22) {
		p1 = ps.txdata + pktlen - 22;
		p2 = pp.mycall + 6;
		for (i = 7; i > 0; i--, p2--) {
			*p1++ = dbl_tab[*p2 & 0xf];
			*p1++ = dbl_tab[(*p2 >> 4) & 0xf];
		}
		*p1++ = dbl_tab[PACTOR_QRT_HEADER & 0xf];
		*p1++ = dbl_tab[(PACTOR_QRT_HEADER >> 4) & 0xf];
	} else {
		p1 = ps.txdata + pktlen - 11;
		p2 = pp.mycall + 6;
		for (i = 7; i > 0; i--, p2--)
			*p1++ = *p2;
		*p1++ = PACTOR_QRT_HEADER;
	}
	memcpy(buf, pp.mycall, sizeof(buf));
	buf[7] = 0;
	if ((p1 = memchr(buf, 0x0f, sizeof(buf))))
		*p1 = 0;
	ps.txdata[pktlen - 3] = (ps.pkt_counter & 3) | 0x80;
	crc = calc_crc_ccitt(ps.txdata, pktlen - 2, get_crc_preset(pktlen));
	ps.txdata[pktlen - 2] = crc & 0xff;
	ps.txdata[pktlen - 1] = crc >> 8;
	ps.pkt_mon_len = snprintf(ps.pkt_mon, sizeof(ps.pkt_mon), "QRT: H:v C:%c CALL:%s", 
				  '0' + (ps.pkt_counter & 3), buf);
}

static int encode_packet(int pktlen) 
{
        int retval = 0;
        unsigned char stat = ps.pkt_counter & 3;
        unsigned char *p1, *p2;
        int i;
        unsigned int crc;
        char buf[64];
        int char_cnt;
        int bitptr, pktbitlen;
        unsigned bitbuf;
        unsigned short ch;
        struct huffman *huff;

        if ((pktlen == 11 || pktlen == 23) && (ps.flags & FLG_QRT)) {
		encode_qrt_packet(pktlen);
                return PKT_QRT;
        }
        kbd_negack();
        pktbitlen = (pktlen - 3) * 8;
        p2 = buf;
        bitbuf = bitptr = char_cnt = 0;
        if (ps.flags & FLG_BECOMEIRS) {
                stat |= 0x40;
		retval |= PKT_BRKIN;
	}
        memset(ps.txdata, 0, pktlen - 3);
        while (bitptr < pktbitlen) {
                if ((ch = kbd_get()) == KBD_EOF) {
                        retval |= PKT_IDLE;
                        char_cnt = 255;
                        break;
                }
                switch (ch) {
        	case 0204:      
                        ch = 12; 
                        break;
                case 0224:      
                        ch = 13; 
                        break;
                case 0201:      
                        ch = 14; 
                        break;
                case 0216:      
                        ch = 20; 
                        break;
                case 0231:      
                        ch = 21; 
                        break;
                case 0232:      
                        ch = 22; 
                        break;
                case 0341:      
                        ch = 23; 
                        break;
                }
                if (ch & 0x80) {
                        char_cnt = 0;
                        break;
                }
		ch &= KBD_CHAR;
                huff = huffman + ch;
                if (bitptr + huff->len > pktbitlen) {
                        kbd_back();
                        break;
                }
                bitbuf = (huff->code) << (bitptr & 7);
                p1 = ps.txdata + (bitptr >> 3);
                *p1++ |= bitbuf;
                *p1++ |= bitbuf >> 8;
                *p1++ |= bitbuf >> 16;
                bitptr += huff->len;
                *p2++ = (ch >= ' ') ? ch : '.';
                char_cnt++;
        }
        if (char_cnt > pktlen - 3) {
                huff = huffman + PACTOR_IDLE;
                while (bitptr < pktbitlen) {
                        bitbuf = (huff->code) << (bitptr & 7);
                        p1 = ps.txdata + (bitptr >> 3);
                        *p1++ |= bitbuf;
                        *p1++ |= bitbuf >> 8;
                        *p1++ |= bitbuf >> 16;
                        bitptr += huff->len;
                }
                stat |= 0x04;
        } else {
                kbd_negack();
                p2 = buf;
                p1 = ps.txdata;
                for (i = pktlen - 3; i > 0; i--)
                        if (retval & PKT_IDLE)
                                *p1++ = PACTOR_IDLE;
                        else {
                                if ((ch = kbd_get()) == KBD_EOF) {
                                        retval |= PKT_IDLE;
                                        *p1++ = PACTOR_IDLE;
                                } else {
                                        *p1++ = ch;
                                        *p2++ = (ch >= ' ') ? ch : '.';
                                }
                        }
        }
        *p2 = '\0';
        ps.txdata[pktlen - 3] = stat;
        crc = calc_crc_ccitt(ps.txdata, pktlen - 2, get_crc_preset(pktlen));
        ps.txdata[pktlen - 2] = crc & 0xff;
        ps.txdata[pktlen - 1] = crc >> 8;
	ps.pkt_mon_len = snprintf(ps.pkt_mon, sizeof(ps.pkt_mon), "C:%c %sDATA: \"%s\"", '0' + (stat & 3),
				  (stat & 0x40) ? "BRK " : "", buf);
        return retval;
}

/* --------------------------------------------------------------------- */
/*
 * Timing functions
 */

#define TMSIZE (sizeof(ps.tm.devflt)/sizeof(ps.tm.devflt[0]))

extern __inline__ void tmg_clear(void)
{
	memset(ps.tm.devflt, 0, sizeof(ps.tm.devflt));
	ps.tm.ptr = 0;
}

static void tmg_add(long dev)
{
	long acc = 0;
	int i;
	
	ps.tm.devflt[ps.tm.ptr++] =  dev;
#if 0
	printf("Timing deviation: %ld  ptr: %d  smooth: %ld %ld %ld %ld %ld %ld %ld %ld\n", dev, ps.tm.ptr,
	       ps.tm.devflt[0], ps.tm.devflt[1], ps.tm.devflt[2], ps.tm.devflt[3], ps.tm.devflt[4], 
	       ps.tm.devflt[5], ps.tm.devflt[6], ps.tm.devflt[7]);
#endif
	ps.tm.ptr %= TMSIZE;
	for (i = 0; i < TMSIZE; i++)
		acc += ps.tm.devflt[i];
	acc /= (signed)TMSIZE;
	if (!acc)
		return;
	for (i = 0; i < TMSIZE; i++)
		ps.tm.devflt[i] -= acc;
#if 1
	ps.rxtime += acc;
	if (!ps.is_master)
		ps.txtime += acc;
#endif
	bufprintf(HFAPP_MSG_DATA_MONITOR, "Timing correction: %ld\n", acc);
}

/* --------------------------------------------------------------------- */

extern __inline__ void marq_clear(void)
{
	memset(ps.marq.acc, 0, sizeof(ps.marq.acc));
	ps.marq.hdr = 0xff;
}

/* --------------------------------------------------------------------- */
/*
 * misc utility functions
 */

extern __inline__ void cycle_end(void)
{
	ps.rxtime += PACTOR_CYCLE_ARQ;
	ps.txtime += PACTOR_CYCLE_ARQ;
	ps.rxinv = !ps.rxinv;
	ps.txinv = !ps.txinv;
}

extern __inline__ void ack_transmit(void)
{
	kbd_ack();
	ps.pkt_counter = (ps.pkt_counter + 1) & 3;
	ps.cur_hdr ^= 0xff;
}

extern __inline__ int retry(void)
{
	if ((--ps.retry) <= 0)
		return 1;
	return 0;
}

#ifdef FREQ_TRACKING
extern __inline__ void pct_freq_tracking(int trk, l1_soft_t trkl, l1_soft_t trkm, l1_soft_t trkh)
{
	ps.rxfreqdev += trk;
	if (!ps.is_master)
		ps.txfreqdev += trk;
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: TRACKING: %d  %d/%d/%d  FREQDEVIATION: %d/%d\n", trk, trkl, trkm, trkh,
		  ps.rxfreqdev, ps.txfreqdev);
}
#endif /* FREQ_TRACKING */

static void send_cs(int cs)
{
	static unsigned int csbits[4] = { PACTOR_CS1, PACTOR_CS2, PACTOR_CS3, PACTOR_CS4 };
	unsigned int bits;

	assert(cs >= 1 && cs <= 4);
	bits = csbits[cs-1];
	l1_fsk_tx_request(ps.txtime-pp.txdelay, pp.txdelay, ps.txfreqdev, 0, 0x100, 1, ps.txbuf);
	ps.txbuf[0] = bits;
	ps.txbuf[1] = bits >> 8;
	l1_fsk_tx_request(ps.txtime, 1000000/100, ps.txfreqdev, ps.txinv, 0x101, 12, ps.txbuf);
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: TX: CS%c\n", '0'+cs);
}

/*
 * length implies 100/200 baud and wether CS3 has to be sent
 */
static void send_packet(unsigned int len)
{
	unsigned char *bp = NULL;

	l1_fsk_tx_request(ps.txtime-pp.txdelay, pp.txdelay, ps.txfreqdev, 0, 0x100, 1, ps.txbuf);
	switch (len) {
	case 21:
		ps.txbuf[0] = dbl_tab[PACTOR_CS3 & 0xf];
		ps.txbuf[1] = dbl_tab[(PACTOR_CS3 >> 4) & 0xf];
		ps.txbuf[2] = dbl_tab[(PACTOR_CS3 >> 8) & 0xf];
		bp = ps.txbuf+3;
		break;

	case 10:
		ps.txbuf[0] = PACTOR_CS3 & 0xff;
		ps.txbuf[1] = PACTOR_CS3 >> 8;
		bp = ps.txbuf+2;
		break;

	case 11:
	case 23:
		ps.txbuf[0] = ps.cur_hdr;
		bp = ps.txbuf+1;
		break;
		
	default:
		errprintf(SEV_FATAL, "send_packet: invalid packet length\n");
	}
	memcpy(bp, ps.txdata, len);
	len = (len >= 16) ? 24 : 12;
	l1_fsk_tx_request(ps.txtime, len >= 16 ? 1000000/200 : 1000000/100, ps.txfreqdev, ps.txinv, 0x101, 
		      8*len, ps.txbuf);
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: TX: %s\n", ps.pkt_mon);
}
	
static int receive_cs(void)
{
	unsigned char csbuf[2];
	unsigned int csbits;
	long dev;
	unsigned int diff1, diff2, diff3, diff4;
#ifdef FREQ_TRACKING
	l1_soft_t trkm, trkl, trkh;
	int trk;
#endif /* FREQ_TRACKING */

	l1_fsk_rx_request(ps.rxtime, 1000000/100/RXOVERSAMPLING, ps.rxfreqdev, 100, 0x200, 12*RXOVERSAMPLING, ps.rxbuf);
#ifdef FREQ_TRACKING
	l1_fsk_rx_request(ps.rxtime, 1000000/100, ps.rxfreqdev-FREQ_TRACKING_DIST, 100, 0x210, 12, ps.trk.rxb1);
	l1_fsk_rx_request(ps.rxtime, 1000000/100, ps.rxfreqdev+FREQ_TRACKING_DIST, 100, 0x211, 12, ps.trk.rxb2);
#endif /* FREQ_TRACKING */
	while (l1_fsk_wait_request() != 0x200);
	soft_to_hard(ps.rxbuf, csbuf, 12, RXOVERSAMPLING, ps.rxinv);
	csbits = (csbuf[0] | (csbuf[1] << 8)) & 0xfff;
	dev = soft_time_dev(ps.rxbuf, 12, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
	tmg_add(dev);
#ifdef FREQ_TRACKING
	trk = freq_tracking(ps.rxbuf, ps.trk.rxb1, ps.trk.rxb2, 12, RXOVERSAMPLING, &trkm, &trkl, &trkh);
	pct_freq_tracking(trk, trkl, trkm, trkh);
#endif /* FREQ_TRACKING */
	diff1 = hweight16(csbits ^ PACTOR_CS1);
	if (diff1 == 0) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS1 dT=%ld\n", dev);
		return 1;
	} else if (diff1 < 2) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS1 diff=%d dT=%ld bits: %03x\n", diff1, dev, csbits);
		return 5;
	} else if (diff1 == 12) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: INV CS1 dT=%ld\n", dev);
		return 9;
	}
	diff2 = hweight16(csbits ^ PACTOR_CS2);
	if (diff2 == 0) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS2 dT=%ld\n", dev);
		return 2;
	} else if (diff2 < 2) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS2 diff=%d dT=%ld bits: %03x\n", diff2, dev, csbits);
		return 6;
	} else if (diff2 == 12) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: INV CS2 dT=%ld\n", dev);
		return 10;
	}
	diff3 = hweight16(csbits ^ PACTOR_CS3);
	if (diff3 == 0) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS3 dT=%ld\n", dev);
		return 3;
	} else if (diff3 < 2) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS3 diff=%d dT=%ld bits: %03x\n", diff3, dev, csbits);
		return 7;
	} else if (diff3 == 12) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: INV CS3 dT=%ld\n", dev);
		return 11;
	}
	diff4 = hweight16(csbits ^ PACTOR_CS4);
	if (diff4 == 0) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS4 dT=%ld\n", dev);
		return 4;
	} else if (diff4 < 2) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS4 diff=%d dT=%ld bits: %03x\n", diff4, dev, csbits);
		return 8;
	} else if (diff4 == 12) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: INV CS4 dT=%ld\n", dev);
		return 12;
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS? dT=%ld bits: %03x\n", dev, csbits);
	return 0;
}

static int receive_packet(unsigned int len)
{
	unsigned char pkt[24];
#ifdef AUTOMATIC_SPEEDUP
	unsigned char pktspdup[12];
	unsigned int spdup;
#endif /* AUTOMATIC_SPEEDUP */
	long dev = 0;
	unsigned int crc, inv_crc = 0;
	unsigned char hdr = 0xff;
	l1_soft_t *pktsdata = NULL;
	unsigned char *pktbegin = NULL;
	int i;
#ifdef FREQ_TRACKING
	l1_soft_t trkm, trkl, trkh;
	int trk = 0;
#endif /* FREQ_TRACKING */

	switch (len) {
	case 10:
		l1_fsk_rx_request(ps.rxtime+160000, 1000000/100/RXOVERSAMPLING, ps.rxfreqdev, 100, 0x200, 
				  10*8*RXOVERSAMPLING, ps.rxbuf);
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(ps.rxtime+160000, 1000000/100, ps.rxfreqdev-FREQ_TRACKING_DIST, 100, 0x210, 10*8, ps.trk.rxb1);
		l1_fsk_rx_request(ps.rxtime+160000, 1000000/100, ps.rxfreqdev+FREQ_TRACKING_DIST, 100, 0x211, 10*8, ps.trk.rxb2);
#endif /* FREQ_TRACKING */
		while (l1_fsk_wait_request() != 0x200);
		dev = soft_time_dev(ps.rxbuf, 10*8, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
		tmg_add(dev);
		soft_to_hard(pktsdata = ps.rxbuf, pktbegin = pkt, 10*8, RXOVERSAMPLING, ps.rxinv);
		hdr = 2;
		inv_crc = PACTOR_INVERTED_FCS_10;
#ifdef FREQ_TRACKING
		trk = freq_tracking(ps.rxbuf, ps.trk.rxb1, ps.trk.rxb2, 10*8, RXOVERSAMPLING, &trkm, &trkl, &trkh);
#endif /* FREQ_TRACKING */
		break;

	case 21:
		l1_fsk_rx_request(ps.rxtime+120000, 1000000/200/RXOVERSAMPLING, ps.rxfreqdev, 200, 0x200, 
				  21*8*RXOVERSAMPLING, ps.rxbuf);
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(ps.rxtime+120000, 1000000/200, ps.rxfreqdev-FREQ_TRACKING_DIST, 200, 0x210, 21*8, ps.trk.rxb1);
		l1_fsk_rx_request(ps.rxtime+120000, 1000000/200, ps.rxfreqdev+FREQ_TRACKING_DIST, 200, 0x211, 21*8, ps.trk.rxb2);
#endif /* FREQ_TRACKING */
		while (l1_fsk_wait_request() != 0x200);
		dev = soft_time_dev(ps.rxbuf, 21*8, RXOVERSAMPLING, 1000000/200/RXOVERSAMPLING);
		tmg_add(dev);
		soft_to_hard(pktsdata = ps.rxbuf, pktbegin = pkt, 21*8, RXOVERSAMPLING, ps.rxinv);
		hdr = 2;
		inv_crc = PACTOR_INVERTED_FCS_21;
#ifdef FREQ_TRACKING
		trk = freq_tracking(ps.rxbuf, ps.trk.rxb1, ps.trk.rxb2, 21*8, RXOVERSAMPLING, &trkm, &trkl, &trkh);
#endif /* FREQ_TRACKING */
		break;

	case 11:
		l1_fsk_rx_request(ps.rxtime, 1000000/100/RXOVERSAMPLING, ps.rxfreqdev, 100, 0x200, 12*8*RXOVERSAMPLING, ps.rxbuf);
#ifdef AUTOMATIC_SPEEDUP
		l1_fsk_rx_request(ps.rxtime, 1000000/100, ps.rxfreqdev, 200, 0x201, 12*8, ps.spd.rxb1);
		l1_fsk_rx_request(ps.rxtime+1000000/200, 1000000/100, ps.rxfreqdev, 200, 0x202, 12*8, ps.spd.rxb2);
#endif /* AUTOMATIC_SPEEDUP */
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(ps.rxtime, 1000000/100, ps.rxfreqdev-FREQ_TRACKING_DIST, 100, 0x210, 12*8, ps.trk.rxb1);
		l1_fsk_rx_request(ps.rxtime, 1000000/100, ps.rxfreqdev+FREQ_TRACKING_DIST, 100, 0x211, 12*8, ps.trk.rxb2);
#endif /* FREQ_TRACKING */
		while (l1_fsk_wait_request() != 0x200);
		dev = soft_time_dev(ps.rxbuf, 12*8, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
		tmg_add(dev);
		soft_to_hard(ps.rxbuf, pkt, 12*8, RXOVERSAMPLING, ps.rxinv);
		hdr = (hweight8(pkt[0] ^ PACTOR_HEADER1) >= 4);
		pktbegin = pkt + 1;
		pktsdata = ps.rxbuf+8*RXOVERSAMPLING;
		inv_crc = PACTOR_INVERTED_FCS_11;
#ifdef FREQ_TRACKING
		trk = freq_tracking(ps.rxbuf, ps.trk.rxb1, ps.trk.rxb2, 12*8, RXOVERSAMPLING, &trkm, &trkl, &trkh);
#endif /* FREQ_TRACKING */
		break;

	case 23:
		l1_fsk_rx_request(ps.rxtime, 1000000/200/RXOVERSAMPLING, ps.rxfreqdev, 200, 0x200, 24*8*RXOVERSAMPLING, ps.rxbuf);
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(ps.rxtime, 1000000/200, ps.rxfreqdev-FREQ_TRACKING_DIST, 200, 0x210, 24*8, ps.trk.rxb1);
		l1_fsk_rx_request(ps.rxtime, 1000000/200, ps.rxfreqdev+FREQ_TRACKING_DIST, 200, 0x211, 24*8, ps.trk.rxb2);
#endif /* FREQ_TRACKING */
		while (l1_fsk_wait_request() != 0x200);
		dev = soft_time_dev(ps.rxbuf, 24*8, RXOVERSAMPLING, 1000000/200/RXOVERSAMPLING);
		tmg_add(dev);
		soft_to_hard(ps.rxbuf, pkt, 24*8, RXOVERSAMPLING, ps.rxinv);
		hdr = (hweight8(pkt[0] ^ PACTOR_HEADER1) >= 4);
		pktsdata = ps.rxbuf+8*RXOVERSAMPLING;
		pktbegin = pkt + 1;
		inv_crc = PACTOR_INVERTED_FCS_23;
#ifdef FREQ_TRACKING
		trk = freq_tracking(ps.rxbuf, ps.trk.rxb1, ps.trk.rxb2, 24*8, RXOVERSAMPLING, &trkm, &trkl, &trkh);
#endif /* FREQ_TRACKING */
		break;

	default:
		errprintf(SEV_FATAL, "receive_packet: invalid packet length\n");
	}
#ifdef FREQ_TRACKING
	pct_freq_tracking(trk, trkl, trkm, trkh);
#endif /* FREQ_TRACKING */
	crc = calc_crc_ccitt(pktbegin, len, get_crc_preset(len));
	if (crc == PACTOR_CORRECT_FCS) {
#ifdef AUTOMATIC_SPEEDUP
		if ((spdup = (len == 11))) {
			soft_to_hard(ps.spd.rxb1, pktspdup, 12*8, 1, ps.rxinv);
			if (memcmp(pktspdup, pkt, 12))
				spdup = 0;
			else {
				soft_to_hard(ps.spd.rxb2, pktspdup, 12*8, 1, ps.rxinv);
				if (memcmp(pktspdup, pkt, 12))
					spdup = 0;
				else
					bufprintf(HFAPP_MSG_DATA_STATUS, 
					    "PACTOR RX: speedup");
					pactor_mode_speedup(); // by günther and it worked
			}
		}
#endif /* AUTOMATIC_SPEEDUP */
		marq_clear();
		i = 0x8000 | decode_packet(pktbegin, len);
		if (ps.pkt_data_len > ps.last_pkt_cnt) {
			bufwrite(HFAPP_MSG_DATA_RECEIVE, ps.pkt_data+ps.last_pkt_cnt, ps.pkt_data_len - ps.last_pkt_cnt);
			ps.last_pkt_cnt = ps.pkt_data_len;
		}
		bufprintf(HFAPP_MSG_DATA_MONITOR, "%s  dT=%ld hdr=%d\n", ps.pkt_mon, dev, (int)hdr);
		return i;
	}
	if (crc == inv_crc)
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: inverted!\n");

	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: dev: %ld  data: %02x %02x %02x %02x  crc: %04x\n", 
		  dev, pkt[0], pkt[1], pkt[2], pkt[3], crc);
	/*
	 * try memory ARQ
	 */
	if (hdr != ps.marq.hdr) {
		marq_clear();
		ps.marq.hdr = hdr;
	}
	if (ps.rxinv) 
		for (i = 0; i < len*8; i++, pktsdata += RXOVERSAMPLING)
			ps.marq.acc[i] -= *pktsdata;
	else
		for (i = 0; i < len*8; i++, pktsdata += RXOVERSAMPLING)
			ps.marq.acc[i] += *pktsdata;
	soft_to_hard(ps.marq.acc, pkt, len*8, 1, 0);
	crc = calc_crc_ccitt(pkt, len, get_crc_preset(len));
	if (crc == PACTOR_CORRECT_FCS) {
		marq_clear();
		i = 0x8000 | decode_packet(pkt, len);
		if (ps.pkt_data_len > ps.last_pkt_cnt) {
			bufwrite(HFAPP_MSG_DATA_RECEIVE, ps.pkt_data+ps.last_pkt_cnt, ps.pkt_data_len - ps.last_pkt_cnt);
			ps.last_pkt_cnt = ps.pkt_data_len;
		}
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: MARQ: %s  dT=%ld hdr=%d\n", ps.pkt_mon, dev, (int)hdr);
		return i;
	}
	if (crc == inv_crc)
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: MARQ: inverted!\n");
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: MARQ:  data: %02x %02x %02x %02x  crc: %04x\n", 
		  pkt[0], pkt[1], pkt[2], pkt[3], crc);
	return 0;
}

/* --------------------------------------------------------------------- */

static void disp_status(const char *modename)
{
#ifdef DEBUG_STATUSDISPLAY
	bufprintf(HFAPP_MSG_DATA_STATUS, "PACTOR %s\nTX: %c %cP i:%c hdr:%02x cntr:%d df:%d\n"
		  "RX: i:%c rt:%d df:%d\nCMD:%s%s%s%s\n", modename, ps.is_master ? 'M' : 'S',
		  ps.is_longpath ? 'L' : 'S', ps.txinv ? '-' : '+', (int)ps.cur_hdr,
		  (int)ps.pkt_counter, ps.txfreqdev, ps.rxinv ? '-' : '+', ps.retry, ps.rxfreqdev,
		  (ps.flags & FLG_QRT) ? " QRT" : "", (ps.flags & FLG_BECOMEIRS) ? " IRS" : "", 
		  (ps.flags & FLG_BECOMEISS) ? " ISS" : "", (ps.flags & FLG_SPEEDUP) ? " SPEEDUP" : "");

#endif /* DEBUG_STATUSDISPLAY*/

#if 1
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR %s  TX: %c %cP i:%c hdr:%02x cntr:%d df:%d"
		  "  RX: i:%c rt:%d df:%d  CMD:%s%s%s%s\n", modename, ps.is_master ? 'M' : 'S',
		  ps.is_longpath ? 'L' : 'S', ps.txinv ? '-' : '+', (int)ps.cur_hdr,
		  (int)ps.pkt_counter, ps.txfreqdev, ps.rxinv ? '-' : '+', ps.retry, ps.rxfreqdev,
		  (ps.flags & FLG_QRT) ? " QRT" : "", (ps.flags & FLG_BECOMEIRS) ? " IRS" : "", 
		  (ps.flags & FLG_BECOMEISS) ? " ISS" : "", (ps.flags & FLG_SPEEDUP) ? " SPEEDUP" : "");
#endif
}

/* --------------------------------------------------------------------- */

static void arq_statemachine(int ok200)
{
	int i, eflg;

	marq_clear();
	tmg_clear();

	ps.last_pkt_cnt = ps.pkt_data_len = 0;
	ps.retry = pp.retry;
	ps.flags = 0;

	if (ps.is_master) {
		ps.pkt_counter = 1;
		ps.cur_hdr = PACTOR_HEADER1;
		send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_MASTERCONNECT, ERR_NOERR);
		if (ok200) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			goto tx_200_cs1;
		}
		goto tx_200_100;
		
	} else {
		ps.pkt_counter = 0;
		send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_SLAVECONNECT, ERR_NOERR);
		if (ok200) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			goto rx_200_cs1;
		}
		goto rx_200_100;
	}

	/*
	 * 100 baud part
	 */	
rx_200_100:
	send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED100, ERR_NOERR);
	ps.retry = pp.retry;
	for (;;) {
		send_cs(4);
		disp_status("ARQ RX 200->100");
		cycle_end();
		i = receive_packet(11);
		if (i & PKT_BRKIN)
			goto rx_tx_100;
		if (i & PKT_QRT)
			goto rx_qrt_100_cs1;
		if (i) {
			if (ps.flags & FLG_BECOMEISS)
				goto rx_tx_100;
			goto rx_100_cs1;
		}
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


rx_100_cs1:
	ps.retry = pp.retry;
	for (;;) {
		send_cs(1);
		disp_status("ARQ RX 100 CS1");
		cycle_end();
		i = receive_packet(11);
		if (i & PKT_BRKIN)
			goto rx_tx_100;
		if (i & PKT_QRT)
			goto rx_qrt_100_cs2;
		if (i & PKT_NEW) {
			if (ps.flags & FLG_BECOMEISS)
				goto rx_tx_100;
			if (ps.flags & FLG_SPEEDUP) 
				goto rx_100_200_cs2;
			goto rx_100_cs2;
		}
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
rx_100_cs2:
	ps.retry = pp.retry;
	for (;;) {
		send_cs(2);
		disp_status("ARQ RX 100 CS2");
		cycle_end();
		i = receive_packet(11);
		if (i & PKT_BRKIN)
			goto rx_tx_100;
		if (i & PKT_QRT)
			goto rx_qrt_100_cs1;
		if (i & PKT_NEW) {
			if (ps.flags & FLG_BECOMEISS)
				goto rx_tx_100;
			if (ps.flags & FLG_SPEEDUP) 
				goto rx_100_200_cs1;
			goto rx_100_cs1;
		}
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
rx_tx_100:
	ps.flags &= ~FLG_BECOMEISS;
	ps.pkt_counter = 0;
	ps.rxtime += (12*8-12)*(1000000/100);
	ps.retry = pp.retry;
	eflg = encode_packet(10);
	send_short_msg(HFAPP_MSG_STATE_PACTOR_ISS, ERR_NOERR);
	for (;;) {
		send_packet(10);
		disp_status("ARQ RX->TX 100");
		cycle_end();
		i = receive_cs();
		if (i == 3) {
			ack_transmit();
			goto tx_rx_100;
		} else if (i == 4) {
			ack_transmit();
			ps.cur_hdr = PACTOR_HEADER1;
			goto tx_100_200_cs1;
		} else if (i == 1) {
			ack_transmit();
			if (eflg & PKT_QRT) {
				send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_NOERR);
				return;
			}
			ps.cur_hdr = PACTOR_HEADER1;
			goto tx_100_cs1;
		}
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_100_cs1:
	ps.retry = pp.retry;
	eflg = encode_packet(11);
	for (;;) {
		send_packet(11);
		disp_status("ARQ TX 100 CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2 || i == 6) {
			ack_transmit();
			if (eflg & PKT_QRT) {
				send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_NOERR);
				return;
			}
			goto tx_100_cs2;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_100;
		} else if (i == 4) {
			ack_transmit();
			goto tx_100_200_cs2;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_100_cs2:
	ps.retry = pp.retry;
	eflg = encode_packet(11);
	for (;;) {
		send_packet(11);
		disp_status("ARQ TX 100 CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1 || i == 5) {
			ack_transmit();
			if (eflg & PKT_QRT) {
				send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_NOERR);
				return;
			}
			goto tx_100_cs1;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_100;
		} else if (i == 4) {
			ack_transmit();
			goto tx_100_200_cs1;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_rx_100:
	ps.flags &= ~FLG_BECOMEIRS;
	ps.last_pkt_cnt = ps.pkt_data_len = 0;
	ps.pkt_counter = 3;
	ps.txtime += (12*8-12)*(1000000/100);
	ps.retry = pp.retry;		
	ack_transmit();
	send_short_msg(HFAPP_MSG_STATE_PACTOR_IRS, ERR_NOERR);
	for (;;) {
		i = receive_packet(10);
		if (i & PKT_BRKIN)
			goto rx_tx_100;
		if (i) {
			if (ps.flags & FLG_SPEEDUP)
				goto rx_100_200_cs2;
			if (ps.flags & FLG_BECOMEISS)
				goto rx_tx_100;
			goto rx_100_cs1;
		}
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
		send_cs(2);
		disp_status("ARQ TX->RX 100");
		cycle_end();
	}


tx_200_100:
	send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED100, ERR_NOERR);
	ps.retry = pp.retry;		
	eflg = encode_packet(11);
	for (;;) {
		send_packet(11);
		disp_status("ARQ TX 200->100");
		cycle_end();
		i = receive_cs();
		if (i == 1) {
			ack_transmit();
			if (eflg & PKT_QRT) {
				send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_NOERR);
				return;
			}
			goto tx_100_cs1;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_100;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}
	

rx_qrt_100_cs1:
rx_qrt_200_cs1:
	send_cs((ps.qrt_cs = 1));
	while (l1_fsk_wait_request() != 0x101);
	send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_NOERR);
	return;

rx_qrt_100_cs2:
rx_qrt_200_cs2:
	send_cs((ps.qrt_cs = 2));
	while (l1_fsk_wait_request() != 0x101);
	send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_NOERR);
	return;

	/*
	 * 200 baud part
	 */
rx_100_200_cs1:
	ps.flags &= ~FLG_SPEEDUP;
	ps.retry = PACTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(4);
		disp_status("ARQ RX 100->200 CS1");
		cycle_end();
		i = receive_packet(23);
		if (i & PKT_BRKIN) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			goto rx_tx_200;
		}
		if (i & PKT_QRT)
			goto rx_qrt_200_cs2;
		if (i) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			if (ps.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			goto rx_200_cs2;
		} else if (retry())
			goto rx_100_cs1;
	}
	

rx_100_200_cs2:
	ps.flags &= ~FLG_SPEEDUP;
	ps.retry = PACTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(4);
		disp_status("ARQ RX 100->200 CS2");
		cycle_end();
		i = receive_packet(23);
		if (i & PKT_BRKIN) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			goto rx_tx_200;
		}
		if (i & PKT_QRT)
			goto rx_qrt_200_cs1;
		if (i) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			if (ps.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			goto rx_200_cs1;
		} else if (retry())
			goto rx_100_cs2;
	}
	

tx_100_200_cs1:
	ps.flags &= ~FLG_SPEEDUP;
	ps.retry = pp.retry;		
	eflg = encode_packet(23);
	for (;;) {
		send_packet(23);
		disp_status("ARQ TX 100->200 CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			goto tx_200_cs2;
	        } else if (i == 1)
			goto tx_100_cs1;
		else if (i == 3) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			goto tx_rx_200;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
tx_100_200_cs2:
	ps.flags &= ~FLG_SPEEDUP;
	ps.retry = pp.retry;		
	eflg = encode_packet(23);
	for (;;) {
		send_packet(23);
		disp_status("ARQ TX 100->200 CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			goto tx_200_cs1;
	        } else if (i == 2)
			goto tx_100_cs2;
		else if (i == 3) {
			ack_transmit();
			send_short_msg(HFAPP_MSG_STATE_PACTOR_SPEED200, ERR_NOERR);
			goto tx_rx_200;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
rx_200_cs1:
	ps.retry = PACTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(1);
		disp_status("ARQ RX 200 CS1");
		cycle_end();
		i = receive_packet(23);
		if (i & PKT_BRKIN)
			goto rx_tx_200;
		if (i & PKT_QRT)
			goto rx_qrt_200_cs2;
		if (i & PKT_NEW) {
			if (i && ps.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			goto rx_200_cs2;
		} else if (retry())
			goto rx_200_100;
	}

	
rx_200_cs2:
	ps.retry = PACTOR_RETRY_HISPEED;
	for (;;) {
		send_cs(2);
		disp_status("ARQ RX 200 CS2");
		cycle_end();
		i = receive_packet(23);
		if (i & PKT_BRKIN)
			goto rx_tx_200;
		if (i & PKT_QRT)
			goto rx_qrt_200_cs1;
		if (i & PKT_NEW) {
			if (i && ps.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			goto rx_200_cs1;
		} else if (retry())
			goto rx_200_100;
	}

	
rx_tx_200:
	ps.flags &= ~FLG_BECOMEISS;
	ps.pkt_counter = 0;
	ps.rxtime += (12*8-12)*(1000000/100);
	ps.retry = pp.retry;
	eflg = encode_packet(21);
	send_short_msg(HFAPP_MSG_STATE_PACTOR_ISS, ERR_NOERR);
	for (;;) {
		send_packet(21);
		disp_status("ARQ RX->TX 200");
		cycle_end();
		i = receive_cs();
		if (i == 3) {
			ack_transmit();
			goto tx_rx_200;
		} else if (i == 4) {
			ps.cur_hdr = PACTOR_HEADER1;
			goto tx_200_100;
		} else if (i == 1) {
			if (eflg & PKT_QRT) {
				send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_NOERR);
				return;
			}
			ack_transmit();
			ps.cur_hdr = PACTOR_HEADER1;
			goto tx_200_cs1;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_200_cs1:
	ps.retry = pp.retry;		
	eflg = encode_packet(23);
	for (;;) {
		send_packet(23);
		disp_status("ARQ TX 200 CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2 || i == 6) {
			ack_transmit();
			if (eflg & PKT_QRT) {
				send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_NOERR);
				return;
			}
			goto tx_200_cs2;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_200;
		} else if (i == 4) 
			goto tx_200_100;
		else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_200_cs2:
	ps.retry = pp.retry;		
	eflg = encode_packet(23);
	for (;;) {
		send_packet(23);
		disp_status("ARQ TX 200 CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1 || i == 5) {
			ack_transmit();
			if (eflg & PKT_QRT) {
				send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_NOERR);
				return;
			}
			goto tx_200_cs1;
		} else if (i == 3) {
			ack_transmit();
			goto tx_rx_200;
		} else if (i == 4) 
			goto tx_200_100;
		else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}


tx_rx_200:
	ps.flags &= ~FLG_BECOMEIRS;
	ps.last_pkt_cnt = ps.pkt_data_len = 0;
	ps.pkt_counter = 0;
	ps.txtime += (12*8-12)*(1000000/100);
	ps.retry = PACTOR_RETRY_HISPEED;		
	send_short_msg(HFAPP_MSG_STATE_PACTOR_IRS, ERR_NOERR);
	for (;;) {
		i = receive_packet(21);
		if (i & PKT_BRKIN)
			goto rx_tx_200;
		if (i) {
			if (i && ps.flags & FLG_BECOMEISS)
				goto rx_tx_200;
			goto rx_200_cs1;
		}
		if (retry())
			goto rx_200_100;
		send_cs(2);
		disp_status("ARQ TX->RX 200");
		cycle_end();
	}


}

/* --------------------------------------------------------------------- */

extern __inline__ void guess_crc(unsigned char *pkt, unsigned int len)
{
#ifdef STANDBY_CRC_PRINT
	unsigned char buf[24];
	unsigned char monbuf[128];
	unsigned char *pktptr;
	unsigned char *bp = monbuf;
	int i;

	if (pkt[0] != 0x55 && pkt[0] != 0xaa)
		return;
	pktptr = pkt+1;
	if ((pkt[len-2] & 0xf8) == 0xf8) {
		for (i = 0; i < 23; i++)
			buf[i] = ~*pktptr++;
		pktptr = buf;
	} else if ((pkt[len-2] & 0xf8) != 0)
		return;
	bp = monbuf + snprintf(monbuf, sizeof(monbuf), "PACTOR: CRC:%04x ", 
			       calc_crc_ccitt(pktptr, len, get_crc_preset(len)));
	for (i = 0; i <= len; i++)
		bp += snprintf(bp, monbuf + sizeof(monbuf) - bp, " %02x", pkt[i]);
	i = decode_packet(pktptr, 23);
	bufprintf(HFAPP_MSG_DATA_MONITOR, "%s\n", monbuf);
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX200: %s\n", ps.pkt_mon);
#endif /* STANDBY_CRC_PRINT */
}


/* --------------------------------------------------------------------- */

void pactor_monitor_init(l1_time_t tm)
{
	ps.mon_mute = tm;
	ps.last_pkt_cnt = 0;
	ps.pkt_data_len	= 0;
}

/* --------------------------------------------------------------------- */

void pactor_monitor_200(unsigned char *pkt, l1_time_t tm)
{
        unsigned char buf[23];
        unsigned char *pktptr;
	unsigned int crc;
        int i;
        int inv = 0;
	int len;
	l1_time_t tmdiff;

	if ((signed)(tm - ps.mon_mute) < 0 && (signed)(tm - ps.mon_mute) > -1000000)
		return;
	guess_crc(pkt, 23);
	switch (crc = calc_crc_ccitt(pktptr = pkt+1, len = 23, pp.crc_preset[3])) {
	case PACTOR_CORRECT_FCS:
		break;

	case PACTOR_INVERTED_FCS_23:
		inv = 1;
		break;

	default:
		switch (crc = calc_crc_ccitt(pktptr = pkt+3, len = 21, pp.crc_preset[2])) {
		case PACTOR_CORRECT_FCS:
			break;
		
		case PACTOR_INVERTED_FCS_21:
			inv = 1;
			break;
			
		default:
			return;
		}
	}
	if (inv) {
		for (i = 0; i < len; i++)
			buf[i] = ~*pktptr++;
		pktptr = buf;
	}
	i = decode_packet(pktptr, len);
	if ((i & PKT_QRT) && !memcmp(ps.pkt_qrtcall, pp.mycall, 7) && len == 23 && ps.qrt_cs >= 1 && ps.qrt_cs <= 2) {
		tmdiff = (tm - (24*8-1)*(1000000/200) - ps.rxtime + PACTOR_CYCLE_ARQ/2) / PACTOR_CYCLE_ARQ;
		if (tmdiff & 1) {
			ps.rxinv = !ps.rxinv;
			ps.txinv = !ps.txinv;
		}
		tmdiff *= PACTOR_CYCLE_ARQ;
		if (tmdiff > 30000000) {
			ps.qrt_cs = 0;
			bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: QRT packet long after last QSO\n");
		} else {
			ps.rxtime += tmdiff;
			ps.txtime += tmdiff;
			if (inv != ps.rxinv)
				bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: Wrong QRT packet inversion!\n");
			else
				send_cs(ps.qrt_cs);
		}
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX200: %s\n", ps.pkt_mon);
	ps.mon_mute = tm + (len * 8) * (1000000/200);
}

void pactor_monitor_100(unsigned char *pkt, l1_time_t tm)
{
        unsigned char buf[11];
        unsigned char *pktptr;
	unsigned int crc;
        int i;
        int inv = 0;
	int len;
	l1_time_t tmdiff;

	if ((signed)(tm - ps.mon_mute) < 0 && (signed)(tm - ps.mon_mute) > -1000000)
		return;
	guess_crc(pkt, 11);
	switch (crc = calc_crc_ccitt(pktptr = pkt+1, len = 11, pp.crc_preset[1])) {
	case PACTOR_CORRECT_FCS:
		break;

	case PACTOR_INVERTED_FCS_11:
		inv = 1;
		break;

	default:
		switch (crc = calc_crc_ccitt(pktptr = pkt+2, len = 10, pp.crc_preset[0])) {
		case PACTOR_CORRECT_FCS:
			break;
		
		case PACTOR_INVERTED_FCS_10:
			inv = 1;
			break;
			
		default:
			return;
		}
	}
	if (inv) {
		for (i = 0; i < len; i++)
			buf[i] = ~*pktptr++;
		pktptr = buf;
	}
	i = decode_packet(pktptr, len);
	if ((i & PKT_QRT) && !memcmp(ps.pkt_qrtcall, pp.mycall, 7) && len == 11 && ps.qrt_cs >= 1 && ps.qrt_cs <= 2) {
		tmdiff = (tm - (12*8-1)*(1000000/100) - ps.rxtime + PACTOR_CYCLE_ARQ/2) / PACTOR_CYCLE_ARQ;
		if (tmdiff & 1) {
			ps.rxinv = !ps.rxinv;
			ps.txinv = !ps.txinv;
		}
		tmdiff *= PACTOR_CYCLE_ARQ;
		if (tmdiff > 30000000) {
			ps.qrt_cs = 0;
			bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: QRT packet long after last QSO\n");
		} else {
			ps.rxtime += tmdiff;
			ps.txtime += tmdiff;
			if (inv != ps.rxinv)
				bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: Wrong QRT packet inversion!\n");
			else
				send_cs(ps.qrt_cs);
		}
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX100: %s\n", ps.pkt_mon);
	ps.mon_mute = tm + (len * 8) * (1000000/100);
}

/* --------------------------------------------------------------------- */

int pactor_check_call(unsigned char *pkt, l1_soft_t *s, l1_time_t tm, int freqdev)
{
	unsigned int hw = hweight8(pkt[0] ^ PACTOR_CALL_HEADER);
	unsigned char invm;
	unsigned char inv;
	unsigned char longpath;
	unsigned int hw200;
//	unsigned char buf200[6];
// 	the code for this is outcommented below
	unsigned char pbuf[9];
	long tmdev;
	unsigned char b, *bp;
	int i;

	hw += hweight32(((pkt[2] ^ pp.mycall[1]) << 16) |
			((pkt[3] ^ pp.mycall[2]) << 8) | ((pkt[4] ^ pp.mycall[3])));
	hw += hweight32(((pkt[5] ^ pp.mycall[4]) << 24) | ((pkt[6] ^ pp.mycall[5]) << 16) |
			((pkt[7] ^ pp.mycall[6]) << 8) | ((pkt[8] ^ pp.mycall[7])));
	hw200 = hweight8(pkt[1] ^ pp.mycall[0]);
	longpath = (hw >= 8*8/2 && hw200 <= 4) || (hw <= 8*8/2 && hw200 >= 4);
	hw += longpath ? 8-hw200 : hw200;
	if ((inv = (hw >= 9*8/2)))
		hw = 9*8-hw;
	if (hw == 0) {
		tmdev = soft_time_dev(s, 9*8, STANDBY_OVERSAMPLING_100, 1000000/100/STANDBY_OVERSAMPLING_100);
		/* currently 100 baud only monitor, therefore no 200 baud connects */
#if 0
		soft_to_hard(s+9*8*STANDBY_OVERSAMPLING_100+(tmdev/(1000000/100/STANDBY_OVERSAMPLING_100)), 
			     buf200, sizeof(buf200)*8, STANDBY_OVERSAMPLING_100, inv);
		hw200 = hweight32(((buf200[0] ^ pp.mycall[0]) << 24) | ((buf200[1] ^ pp.mycall[1]) << 16) |
				  ((buf200[2] ^ pp.mycall[2]) << 8) | (buf200[3] ^ pp.mycall[3]));
		hw200 += hweight16(((buf200[4] ^ pp.mycall[4]) << 8) | (buf200[5] ^ pp.mycall[5]));
#else
		hw200 = 0;
#endif
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: CALL Packet: T=%ld  dT=%ld  200bd_wt=%d  inv=%d  lp=%d\n", 
			  tm+tmdev, tmdev, hw200, inv, longpath);
		ps.is_master = 0;
		ps.is_longpath = longpath;
		ps.rxinv = ps.txinv = inv;
		ps.txtime = (ps.rxtime = tm+tmdev) + PACTOR_CYCLE_STREAM_FEC + pp.txdelay + 10000;
		ps.rxfreqdev = ps.txfreqdev = freqdev;
		l1_fsk_clear_requests();
		arq_statemachine(hw200 == 6*8);
		return 1;
	} else if (hw < 6)
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: CALL Packet: almost mycall, diff %d\n", hw);
	if ((signed)(tm - ps.mon_callmute) < 0 && (signed)(tm - ps.mon_callmute) > -1000000)
		return 0;
	invm = longpath = 0;
	if (pkt[0] != PACTOR_CALL_HEADER) {
		if (pkt[0] != (PACTOR_CALL_HEADER ^ 0xff))
			return 0;
		invm = 0xff;
	}
	pbuf[0] = b = pkt[1] ^ invm;
	if (!isupper(b) && !isdigit(b)) {
		b ^= 0xff;
		if (!isupper(b) && !isdigit(b))
			return 0;
		pbuf[0] = b;
		longpath = 1;
	}
	for (i = 1; i < 8; i++) {
		pbuf[i] = b = pkt[i+1] ^ invm;
		if (!isupper(b) && !isdigit(b) && !(b == 0x0f))
			return 0;
	}
	ps.mon_callmute = tm + (9*8+6*8/2) * (1000000/100);
	if ((bp = memchr(pbuf, 0x0f, sizeof(pbuf))))
		*bp = 0;
	pbuf[8] = 0;
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: CALL Packet: %s%s\n", pbuf, longpath ? " longpath" : "");
	return 0;
}

/* --------------------------------------------------------------------- */

static void strnupr(unsigned char *c, size_t len)
{
	for (; len > 0 && *c; len--, c++)
		if (islower(*c))
			*c -= 'a'-'A';
}

/* --------------------------------------------------------------------- */

void pactor_set_params(const char *destcall, const char *mycall, int txd, int retry, int lp, 
		       unsigned int crc100chg, unsigned int crc100, unsigned int crc200chg, 
		       unsigned int crc200)
{
	unsigned char *bp;
	
	errprintf(SEV_INFO, "pactor params: destcall %-8s mycall %-8s txdelay %dms retries %d %spath  "
		  "CRC preset %04x %04x %04x %04x\n", destcall, mycall, txd, retry, lp ? "long" : "short", 
		  crc100chg, crc100, crc200chg, crc200);
	strncpy(pp.destcall, destcall, sizeof(pp.destcall));
	if ((bp = memchr(pp.destcall, 0, sizeof(pp.destcall))))
		memset(bp, 0x0f, pp.destcall+sizeof(pp.destcall)-bp);
	strnupr(pp.destcall, sizeof(pp.destcall));
	strncpy(pp.mycall, mycall, sizeof(pp.mycall));
	if ((bp = memchr(pp.mycall, 0, sizeof(pp.mycall))))
		memset(bp, 0x0f, pp.mycall+sizeof(pp.mycall)-bp);
	strnupr(pp.mycall, sizeof(pp.mycall));
	pp.txdelay = txd * 1000;
	pp.retry = retry;
	pp.longpath = !!lp;
	pp.crc_preset[0] = crc100chg & 0xffff;
	pp.crc_preset[1] = crc100 & 0xffff;
	pp.crc_preset[2] = crc200chg & 0xffff;
	pp.crc_preset[3] = crc200 & 0xffff;
}

/* --------------------------------------------------------------------- */

static int master_verify_cs(const char *moncall, int bd200)
{
	unsigned char csbuf[2];
	unsigned int csbits;
	long dev;

	l1_fsk_tx_request(ps.txtime, 1000000/100, ps.txfreqdev, ps.txinv, 0x101, 9*8, ps.txbuf);
	l1_fsk_tx_request(ps.txtime+1000000/100*9*8, 1000000/200, ps.txfreqdev, ps.txinv, 0x102, 6*8, ps.txbuf+1);
	l1_fsk_tx_request(ps.txtime+PACTOR_CYCLE_ARQ-pp.txdelay, pp.txdelay, ps.txfreqdev, 0, 0x100, 1, ps.txbuf);
	l1_fsk_rx_request(ps.rxtime, 1000000/100/RXOVERSAMPLING, ps.rxfreqdev, 100, 0x200, 12*RXOVERSAMPLING, ps.rxbuf);
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: TX: CALL %s (CS verify cycle)\n", moncall);
	while (l1_fsk_wait_request() != 0x200);
	soft_to_hard(ps.rxbuf, csbuf, 12, RXOVERSAMPLING, ps.rxinv);
	csbits = (csbuf[0] | (csbuf[1] << 8)) & 0xfff;
	dev = soft_time_dev(ps.rxbuf, 12, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
	if ((csbits == PACTOR_INV_CS1 && bd200) || (csbits == PACTOR_INV_CS4 && !bd200)) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS%c inverted!!\n", bd200 ? '1' : '4');
		return 0;
	}
	if ((csbits == PACTOR_CS1 && bd200) || (csbits == PACTOR_CS4 && !bd200)) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS%c  dT=%ld\n", bd200 ? '1' : '4', dev);
		return 1;
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS?  bits=%03x\n", csbits);
	return 0;
}

/* --------------------------------------------------------------------- */

static int master_call(void)
{
	unsigned char moncall[10];
	unsigned char *bp;
	unsigned int nsamps = (PACTOR_CYCLE_ARQ-PACTOR_CYCLE_STREAM_FEC-pp.txdelay)/(1000000/100)*RXOVERSAMPLING;
	int i;
	l1_soft_t *s;
	unsigned char csbuf[2];
	unsigned int cs;

	memcpy(moncall+1, pp.mycall, 8);
	moncall[9] = 0;
	moncall[0] = '1';
	if (!(bp = memchr(moncall+1, 0xf, 8)))
		bp = moncall+9;
	bp[0] = '\r';
	kbd_clear_and_fill(moncall, bp-moncall+1);

	memcpy(moncall, pp.destcall, 8);
	moncall[8] = 0;
	if ((bp = memchr(moncall+1, 0xf, 8)))
		*bp = 0;
	ps.txtime = l1_get_current_time() + 200000;
	ps.txinv = 0;
	ps.rxfreqdev = ps.txfreqdev = 0;
	ps.txbuf[0] = PACTOR_CALL_HEADER;
	memcpy(ps.txbuf+1, pp.destcall, 8);
	ps.retry = pp.retry;
	ps.is_master = 1;
	ps.is_longpath = pp.longpath;
	l1_fsk_tx_request(ps.txtime-pp.txdelay, pp.txdelay, ps.txfreqdev, 0, 0x100, 1, ps.txbuf);
	for (;;) {
		l1_fsk_tx_request(ps.txtime, 1000000/100, ps.txfreqdev, ps.txinv, 0x101, 9*8, ps.txbuf);
		l1_fsk_tx_request(ps.txtime+1000000/100*9*8, 1000000/200, ps.txfreqdev, ps.txinv, 0x102, 6*8, ps.txbuf+1);
		l1_fsk_tx_request(ps.txtime+PACTOR_CYCLE_ARQ-pp.txdelay, pp.txdelay, ps.txfreqdev, 0, 0x100, 1, ps.txbuf);
		ps.rxtime = ps.txtime+PACTOR_CYCLE_STREAM_FEC;
		l1_fsk_rx_request(ps.rxtime, 1000000/100/RXOVERSAMPLING, ps.rxfreqdev, 100, 0x200, nsamps, ps.rxbuf);
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: TX: CALL %s\n", moncall);
		disp_status("ARQ MASTER CALL");
		while (l1_fsk_wait_request() != 0x200);
		for (s = ps.rxbuf, i = 0; i < nsamps-RXOVERSAMPLING*(12-1); 
		     i++, s++, ps.rxtime += 1000000/100/RXOVERSAMPLING) {
			soft_to_hard(s, csbuf, 12, RXOVERSAMPLING, 0);
			cs = (csbuf[0] | (csbuf[1] << 8)) & 0xfff;
			ps.rxinv = (cs == PACTOR_INV_CS1) || (cs == PACTOR_INV_CS4);
			if (ps.rxinv || (cs == PACTOR_CS1) || (cs == PACTOR_CS4)) {
				if (ps.rxinv)
					cs ^= 0xfff;
				ps.rxtime += PACTOR_CYCLE_ARQ+soft_time_dev(s, 12, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
				ps.txtime += PACTOR_CYCLE_ARQ;
				bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: RX: CS%c  dT=%ld\n", (cs == PACTOR_CS4) ? '4' : '1',
					  ps.rxtime-ps.txtime-PACTOR_CYCLE_STREAM_FEC);
				ps.txinv = !ps.txinv;
				ps.rxinv = !ps.rxinv;
				if (master_verify_cs(moncall, (cs == PACTOR_CS1))) {
					ps.txtime += PACTOR_CYCLE_ARQ;
					ps.txinv = !ps.txinv;
					return (cs == PACTOR_CS1);
				}
			}
		}
		if ((--ps.retry) <= 0)
			return -1;
		ps.txtime += PACTOR_CYCLE_ARQ;
		ps.txinv = !ps.txinv;
	}
}

/* --------------------------------------------------------------------- */

void *mode_pactor_arq(void *dummy)
{
	int i;

	l1_fsk_clear_requests();
	errprintf(SEV_INFO, "mode: pactor arq call\n");
	i = master_call();
	if (i >= 0)
		arq_statemachine(i);
	else
	    send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_MASTERCONNECT, ERR_TIMEOUT);
	    send_short_msg(HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT, ERR_TIMEOUT);		
	return NULL;
}

/* --------------------------------------------------------------------- */

void *mode_pactor_fec(void *dummy)
{
	int i;

	l1_fsk_clear_requests();
	errprintf(SEV_INFO, "mode: pactor fec\n");
	ps.txtime = l1_get_current_time() + 100000;
	ps.txinv = 0;
	ps.rxfreqdev = ps.txfreqdev = 0;
	ps.cur_hdr = PACTOR_HEADER1;
	l1_fsk_tx_request(ps.txtime, pp.txdelay, ps.txfreqdev, 0, 1, 1, ps.txbuf);
	ps.txtime += pp.txdelay;
	kbd_clear_and_fill(NULL, 0);
	send_short_msg(HFAPP_MSG_STATE_PACTOR_FEC_CONNECT, ERR_NOERR);
	ps.flags = 0;
	for (;;) {
		if (ps.flags & FLG_QRT)
			return NULL;
		ps.flags = 0;
		encode_packet(11);
		ps.txbuf[0] = ps.cur_hdr ? PACTOR_HEADER2 : PACTOR_HEADER1;
		memcpy(ps.txbuf+1, ps.txdata, 11);
		for (i = 0; i < PACTOR_RETRY_FEC; i++) {
			l1_fsk_tx_request(ps.txtime, 1000000/100, ps.txfreqdev, ps.txinv, i, 8*12, ps.txbuf);
			ps.txtime += PACTOR_CYCLE_STREAM_FEC;
			ps.txinv = !ps.txinv;
		}
		bufprintf(HFAPP_MSG_DATA_MONITOR, "PACTOR: TX: %s\n", ps.pkt_mon);
		ack_transmit();
		disp_status("FEC TX 100");
       		while (l1_fsk_wait_request() != 0);
	}
}

/* --------------------------------------------------------------------- */
