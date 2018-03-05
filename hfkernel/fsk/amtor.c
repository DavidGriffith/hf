/*****************************************************************************/

/*
 *      amtor.c  --  Amtor protocol.
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

#include <stdlib.h> 
#include <assert.h>
#include <syslog.h>
#include <string.h>
#include <sys/types.h>

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"
#include "amtor.h"
#include "standby.h"

/* --------------------------------------------------------------------- */

#define AMTOR_CS1   0x12
#define AMTOR_CS2   0x00
#define AMTOR_CS3   0x0c
#define AMTOR_BETA  0x20
#define AMTOR_ALPHA 0x21
#define AMTOR_RQ    0x22
#define AMTOR_PS1   AMTOR_ALPHA
#define AMTOR_PS2   AMTOR_RQ

#define RXOVERSAMPLING 8

/* --------------------------------------------------------------------- */

static const short amtor_check_table[128] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 33,
        -1, -1, -1, -1, -1, -1, -1, 11, -1, -1, -1, 13, -1, 14, 15, -1,
        -1, -1, -1, -1, -1, -1, -1, 19, -1, -1, -1, 21, -1, 22, 23, -1,
        -1, -1, -1, 32, -1, 26, 27, -1, -1, 28, 29, -1, 30, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1,  3, -1, -1, -1,  5, -1,  6,  7, -1,
        -1, -1, -1,  9, -1, 10,  1, -1, -1, 12, 31, -1,  4, -1, -1, -1,
        -1, -1, -1, 17, -1, 18, 34, -1, -1, 20,  0, -1,  2, -1, -1, -1,
        -1, 24, 25, -1, 16, -1, -1, -1,  8, -1, -1, -1, -1, -1, -1, -1
};

static const unsigned char amtor_encode_table[35] = {
        0x6a, 0x56, 0x6c, 0x47, 0x5c, 0x4b, 0x4d, 0x4e, 0x78, 0x53, 0x55, 0x17, 0x59, 0x1b, 0x1d, 0x1e,
        0x74, 0x63, 0x65, 0x27, 0x69, 0x2b, 0x2d, 0x2e, 0x71, 0x72, 0x35, 0x36, 0x39, 0x3a, 0x3c, 0x5a,
        0x33, 0x0f, 0x66
};

/* --------------------------------------------------------------------- */

#define SHIFT_NUMBER    27
#define SHIFT_LETTER    31
#define SHIFT_LOWERCASE  0

#define IDLE_SHIFT_FREQ 10

#define SYMBOL_PLUS  17
#define SYMBOL_QMARK 25

/* --------------------------------------------------------------------- */

#define INVAL "\001"
#define BELL  "\007" 
#define WRU   "$"

#define LETTERS INVAL "E\nA SIU\rDRJNFCKTZLWHYPQOBG" INVAL "MXV" INVAL
#define NUMBERS_DL INVAL"3\n- '87\r" WRU "4ß,Ä:(5+)2Ü6019?Ö" INVAL "./=" INVAL
#define NUMBERS_GB INVAL"3\n- '87\r" WRU "4" BELL ",%:(5+)2#6019?@" INVAL "./=" INVAL
#define NUMBERS_USA INVAL "3\n- " BELL "87\r" WRU "4',!:(5\")2*6019?&" INVAL "./;" INVAL

static const char amtor_chars[65] = LETTERS NUMBERS_DL;

#define LETTER_MASK 0x33fffeea
#define IS_LETTER(x) ((LETTER_MASK >> (x)) & 1)

#define FREQ_TRACKING_DIST 10 /* Hz */
#define FREQ_TRACKING

/* --------------------------------------------------------------------- */

struct {
	unsigned char destcall[4];
	unsigned char selfeccall[4];
	unsigned char mycall[4];
	int rxinvert;
	int txinvert;
	unsigned int retry;
	unsigned int txdelay;
} ap = { "\0\0\0\0", "\0\0\0\0", "\0\0\0\0", 0, 0, 30, 30000 };

#define MONNUM 8

struct {
	unsigned int flags;
	unsigned int figmode;
	unsigned int lcflag;
	unsigned int last_ch;
	l1_time_t rxtime;
	l1_time_t txtime;
	int rxfreqdev, txfreqdev;
	unsigned int is_master;
	int retry;

	l1_time_t mon_mute;

	struct {
		long devflt[8];
		unsigned int ptr;
	} tm;

	struct {
		int state;
		l1_time_t tm;
	} mon[MONNUM];

	unsigned char txtriple[3];
	l1_soft_t rxbuf[56*RXOVERSAMPLING];
	unsigned char txbuf[8];
#ifdef FREQ_TRACKING
	struct {
		l1_soft_t rxb1[56];
		l1_soft_t rxb2[56];
	} trk;
#endif /* FREQ_TRACKING */
} as;

/* --------------------------------------------------------------------- */
/*
 * Timing functions
 */

#define TMSIZE (sizeof(as.tm.devflt)/sizeof(as.tm.devflt[0]))

extern __inline__ void tmg_clear(void)
{
	memset(as.tm.devflt, 0, sizeof(as.tm.devflt));
	as.tm.ptr = 0;
}

static void tmg_add(long dev)
{
	long acc = 0;
	int i;
	
	as.tm.devflt[as.tm.ptr++] =  dev;
#if 0
	printf("Timing deviation: %ld  ptr: %d  smooth: %ld %ld %ld %ld %ld %ld %ld %ld\n", dev, as.tm.ptr,
	       as.tm.devflt[0], as.tm.devflt[1], as.tm.devflt[2], as.tm.devflt[3], as.tm.devflt[4], 
	       as.tm.devflt[5], as.tm.devflt[6], as.tm.devflt[7]);
#endif
	as.tm.ptr %= TMSIZE;
	for (i = 0; i < TMSIZE; i++)
		acc += as.tm.devflt[i];
	acc /= (signed)TMSIZE;
	if (!acc)
		return;
	for (i = 0; i < TMSIZE; i++)
		as.tm.devflt[i] -= acc;
#if 1
	as.rxtime += acc;
	if (!as.is_master)
		as.txtime += acc;
#endif
	bufprintf(HFAPP_MSG_DATA_MONITOR, "Timing correction: %ld\n", acc);
}

/* --------------------------------------------------------------------- */

static int setcall(unsigned char *call, const char *in)
{
	int i;
	char c;
	unsigned char buf[4];

	for (i = 0; i < 4; i++, in++) {
		c = *in;
		if (c >= 'a' && c <= 'z')
			c -= 'a'-'A';
		if (c < 'A' || c > 'Z')
			return 0;
		buf[i] = (char *)memchr(amtor_chars, c, 32) - amtor_chars;
	}
	memcpy(call, buf, 4);
	return 1;
}
		
/* --------------------------------------------------------------------- */

void amtor_set_params(const unsigned char *destcall, const unsigned char *selfeccall,
		      const unsigned char *mycall, unsigned int txdelay,
		      unsigned int retry, int rxinvert, int txinvert)
{
	errprintf(SEV_INFO, "amtor params: destcall %-4.4s selfeccall %-4.4s mycall %-4.4s "
		  "txdelay %dms retries %d invert%s%s\n", destcall, selfeccall, mycall, 
		  txdelay, retry, rxinvert ? " RX" : "", txinvert ? " TX" : "");
	if (!setcall(ap.destcall, destcall))
		errprintf(SEV_WARNING, "invalid destination call\n");
	if (!setcall(ap.selfeccall, selfeccall))
		errprintf(SEV_WARNING, "invalid selective FEC call\n");
	if (!setcall(ap.mycall, mycall))
		errprintf(SEV_WARNING, "invalid mycall\n");
	ap.rxinvert = !!rxinvert;
	ap.txinvert = !!txinvert;
	ap.retry = retry;
	ap.txdelay = txdelay * 1000;
}

/* --------------------------------------------------------------------- */
	
static const char *monitor_names[35] = {
	"LCS", "E", "LF", "A", "SP", "S", "I", "U", "CR", "D", "R", "J",
	"N", "F", "C", "K", "T", "Z", "L", "W", "H", "Y", "P", "Q", "O", "B",
	"G", "FS", "M", "X", "V", "LS", "BETA", "ALPHA", "RQ"
};

static void monitor_triple(const char *name, unsigned short s1, unsigned short s2, unsigned short s3)
{

	if (s1 >= 35 || s2 >= 35 || s3 >= 35)
		return;
	bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: %sTriple: %s_%s_%s\n", name, 
		  monitor_names[s1], monitor_names[s2], monitor_names[s3]);
}

static void output_char(unsigned short c)
{
	char ch;

	if (c >= 32)
		return;
	switch (c) {
	case SHIFT_LETTER:
		as.figmode = 0;
		as.lcflag = 0;
		break;
		
	case SHIFT_NUMBER:
		as.figmode = 0x20;
		as.lcflag = 0;
		break;
		
	case SHIFT_LOWERCASE:
		as.lcflag = !as.lcflag;
		break;
		
	default:
		ch = amtor_chars[as.figmode + c];
		if (as.lcflag && ch >= 'A' && ch <= 'Z')
			ch += 'a' - 'A';
		bufwrite(HFAPP_MSG_DATA_RECEIVE, &ch, 1);
	}
}

static void output_triple(unsigned short s1, unsigned short s2, unsigned short s3)
{
	output_char(s1);
	output_char(s2);
	output_char(s3);
}

/* --------------------------------------------------------------------- */

#define FLG_QRT       (1<<0)
#define FLG_BECOMEISS (1<<1)
#define FLG_BECOMEIRS (1<<2)

void amtor_mode_qrt(void)
{
	as.flags |= FLG_QRT;
}

void amtor_mode_irs(void)
{
	as.flags |= FLG_BECOMEIRS;
}

void amtor_mode_iss(void)
{
	as.flags |= FLG_BECOMEISS;
}

/* --------------------------------------------------------------------- */

void amtor_reset_uppercase(void)
{
	as.figmode = 0;
	as.lcflag = 0;
}

void amtor_reset_lowercase(void)
{
	as.figmode = 0;
	as.lcflag = 1;
}

void amtor_reset_figurecase(void)
{
	as.figmode = 0x20;
}

/* --------------------------------------------------------------------- */

static unsigned short get_tx_char(void)
{
	unsigned short s;
	unsigned char *bp;
	unsigned int lc;
	unsigned int nr;

	for (;;) {
		kbd_negack();
		if ((s = kbd_get()) == KBD_EOF) {
			kbd_ack(); // by günther, 
				    // ack of end of data must be for autorx
			return AMTOR_BETA;
		}
		s &= KBD_CHAR;
		if (s >= 'a' && s <= 'z') {
			lc = 1;
			s -= 'a' - 'A';
		} else if (s >= 'A' && s <= 'Z')
			lc = 0;
		else
			lc = as.lcflag;
		if (!(bp = memchr(amtor_chars, s, 64))) {
			kbd_ack();
			continue;
		}
		nr = bp - (unsigned char *)amtor_chars;
		if ((nr & 0x20) != as.figmode) {
			as.figmode = nr & 0x20;
			as.lcflag = 0;
			return as.figmode ? SHIFT_NUMBER : SHIFT_LETTER;
		}
		if (lc != as.lcflag) {
			as.lcflag = lc;
			return SHIFT_LOWERCASE;
		}
		kbd_ack();
		return nr - as.figmode;
	}
}

/* --------------------------------------------------------------------- */
/*
 * frequency tracking utility functions
 */

#ifdef FREQ_TRACKING
extern __inline__ void amtor_freq_tracking(int trk, l1_soft_t trkl, l1_soft_t trkm, l1_soft_t trkh)
{
	as.rxfreqdev += trk;
	if (!as.is_master)
		as.txfreqdev += trk;
	bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TRACKING: %d  %d/%d/%d  FREQDEVIATION: %d/%d\n", 
		  trk, trkl, trkm, trkh, as.rxfreqdev, as.txfreqdev);
}
#endif /* FREQ_TRACKING */

/* --------------------------------------------------------------------- */

static void fec_rx_cont(const char *name, unsigned int msg, l1_id_t id, int inv)
{
	l1_soft_t *s;
	unsigned char chdx, chrx;
	short sym_dx, sym_rx, sym_cdx;
#ifdef FREQ_TRACKING
	l1_soft_t trkm, trkl, trkh;
	int trk = 0;
#endif /* FREQ_TRACKING */

	errprintf(SEV_INFO, "%s\n", name);
	send_short_msg(msg, ERR_NOERR);
	as.retry = ap.retry;
	l1_fsk_rx_request(as.rxtime, 1000000/100/RXOVERSAMPLING, as.rxfreqdev, 100,
			  id, 14*RXOVERSAMPLING, as.rxbuf+14*id*RXOVERSAMPLING);
#ifdef FREQ_TRACKING
	l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev-FREQ_TRACKING_DIST, 100, 
			  0x210+2*id, 14, as.trk.rxb1+14*id);
	l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev+FREQ_TRACKING_DIST, 100, 
			  0x211+2*id, 14, as.trk.rxb2+14*id);
#endif /* FREQ_TRACKING */
	as.rxtime += 140000;
	for (;;) {
		id = l1_fsk_wait_request();
		if (id < 0 || id >= 4)
			continue;
		s = as.rxbuf+14*id*RXOVERSAMPLING;
		tmg_add(soft_time_dev(s, 14, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING));
#ifdef FREQ_TRACKING
		trk = freq_tracking(s, as.trk.rxb1+14*id, as.trk.rxb2+14*id, 14, RXOVERSAMPLING, &trkm, &trkl, &trkh);
		amtor_freq_tracking(trk, trkl, trkm, trkh);
#endif /* FREQ_TRACKING */
		soft_to_hard(s, &chdx, 7, RXOVERSAMPLING, inv);
		soft_to_hard(s+7*RXOVERSAMPLING, &chrx, 7, RXOVERSAMPLING, inv);
		as.last_ch = (as.last_ch << 8) | chdx;
		sym_dx = amtor_check_table[(as.last_ch >> 16) & 0x7f];
		sym_rx = amtor_check_table[chrx & 0x7f];
		sym_cdx = amtor_check_table[chdx & 0x7f];
		if (sym_dx >= 0) {
			if (sym_rx == AMTOR_PS1 && sym_dx == AMTOR_PS2)
				as.retry += 2;
			else if (sym_rx == AMTOR_PS2 || sym_dx == AMTOR_PS1)
				as.retry -= 8;
			else if (sym_rx >= 0 && sym_rx != sym_dx && sym_rx != AMTOR_PS1) 
				as.retry -= 8;
			else if (sym_rx >= 0)
				as.retry += 2;
			if (as.retry > ap.retry)
				as.retry = ap.retry;
			output_char(sym_dx);
		} else if (sym_rx >= 0)
			output_char(sym_rx);
		else {
			as.retry--;
			bufprintf(HFAPP_MSG_DATA_RECEIVE, "_"); /* error symbol */
		}
		if ((as.last_ch & 0x7f) == amtor_encode_table[AMTOR_ALPHA] && 
		    ((as.last_ch >> 8) & 0x7f) == amtor_encode_table[AMTOR_ALPHA]) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_FEC_DISCONNECT, ERR_NOERR);
			return;
		}			
		if (as.retry < 0) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_FEC_DISCONNECT, ERR_TIMEOUT);
			return;
		}
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: FECRX: %s_%s  (%02x_%02x)\n", 
			  sym_cdx >= 0 ? monitor_names[sym_cdx] : "?",
			  sym_rx >= 0 ? monitor_names[sym_rx] : "?",
			  chdx & 0x7f, chrx & 0x7f);
		l1_fsk_rx_request(as.rxtime, 1000000/100/RXOVERSAMPLING, as.rxfreqdev, 100,
				  id, 14*RXOVERSAMPLING, s);
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev-FREQ_TRACKING_DIST, 100, 
				  0x210+2*id, 14, as.trk.rxb1+14*id);
		l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev+FREQ_TRACKING_DIST, 100, 
				  0x211+2*id, 14, as.trk.rxb2+14*id);
#endif /* FREQ_TRACKING */
		as.rxtime += 140000;
//		bufprintf(HFAPP_MSG_DATA_STATUS, "%s\nRetry: %d\n", name, as.retry);
	}
}

/* --------------------------------------------------------------------- */

static void fec_rx(void)
{
	l1_soft_t *s;
	l1_id_t id;
	unsigned char chdx, chrx;
	short sym_dx, sym_cdx, sym_rx, sym_idx, sym_irx;
	unsigned short selfcall[5];
#ifdef FREQ_TRACKING
	l1_soft_t trkm, trkl, trkh;
	int trk = 0;
#endif /* FREQ_TRACKING */

	l1_fsk_clear_requests();
	errprintf(SEV_INFO, "mode: amtor fec rx\n");
	kbd_clear_and_fill(NULL, 0);
	as.retry = ap.retry;
	as.figmode = 0;
	as.lcflag = 0;
	as.is_master = 0;
	tmg_clear();
	for (id = 0; id < 4; id++) {
		l1_fsk_rx_request(as.rxtime, 1000000/100/RXOVERSAMPLING, as.rxfreqdev, 100,
			      id, 14*RXOVERSAMPLING, as.rxbuf+14*id*RXOVERSAMPLING);
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev-FREQ_TRACKING_DIST, 100, 
				  0x210+2*id, 14, as.trk.rxb1+14*id);
		l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev+FREQ_TRACKING_DIST, 100, 
				  0x211+2*id, 14, as.trk.rxb2+14*id);
#endif /* FREQ_TRACKING */
		as.rxtime += 140000;
	}
	for (;;) {
		id = l1_fsk_wait_request();
		if (id < 0 || id >= 4)
			continue;
		s = as.rxbuf+14*id*RXOVERSAMPLING;
		tmg_add(soft_time_dev(s, 14, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING));
#ifdef FREQ_TRACKING
		trk = freq_tracking(s, as.trk.rxb1+14*id, as.trk.rxb2+14*id, 14, RXOVERSAMPLING, &trkm, &trkl, &trkh);
		amtor_freq_tracking(trk, trkl, trkm, trkh);
#endif /* FREQ_TRACKING */
		soft_to_hard(s, &chdx, 7, RXOVERSAMPLING, ap.rxinvert);
		soft_to_hard(s+7*RXOVERSAMPLING, &chrx, 7, RXOVERSAMPLING, ap.rxinvert);
		as.last_ch = (as.last_ch << 8) | chdx;
		sym_dx = amtor_check_table[(as.last_ch >> 16) & 0x7f];
		sym_rx = amtor_check_table[chrx & 0x7f];
		sym_cdx = amtor_check_table[chdx & 0x7f];
		sym_idx = amtor_check_table[(~as.last_ch >> 16) & 0x7f];
		sym_irx = amtor_check_table[(~chrx) & 0x7f];
		selfcall[4] = selfcall[3];
		selfcall[3] = selfcall[2];
		selfcall[2] = selfcall[1];
		selfcall[1] = selfcall[0];
		if (sym_irx >= 0) 
			selfcall[0] = sym_irx;
		else if (sym_idx >= 0)
			selfcall[0] = sym_idx;
		else 
			selfcall[0] = -1;
		if (selfcall[0] == AMTOR_BETA && selfcall[4] == ap.selfeccall[0] && selfcall[3] == ap.selfeccall[1] &&
		    selfcall[2] == ap.selfeccall[2] && selfcall[1] == ap.selfeccall[3]) {
			bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: Selective FEC call received\n");
			as.last_ch = ~as.last_ch;
			fec_rx_cont("AMTOR SELECTIVE FEC RX", HFAPP_MSG_STATE_AMTOR_SELFEC_RX, id, !ap.rxinvert);
			return;
		}
		if (selfcall[0] == AMTOR_BETA && IS_LETTER(selfcall[4]) && IS_LETTER(selfcall[3]) && 
		    IS_LETTER(selfcall[2]) && IS_LETTER(selfcall[1]))
			bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: Selective FEC call %c%c%c%c\n",
				  amtor_chars[selfcall[4]], amtor_chars[selfcall[3]], 
				  amtor_chars[selfcall[2]], amtor_chars[selfcall[1]]);
		if (sym_dx >= 0 || sym_rx >= 0) {
			output_char(sym_dx >= 0 ? sym_dx : sym_rx);
			if (sym_dx >= 0 && sym_dx < 32 && sym_dx == sym_rx) {
				fec_rx_cont("AMTOR COLLECTIVE FEC RX", HFAPP_MSG_STATE_AMTOR_COLFEC_RX, id, ap.rxinvert);
				return;
			}
		}
		if (sym_rx == AMTOR_PS2 || sym_dx == AMTOR_PS1)
			return;
		if (sym_rx < 0 || sym_dx < 0)
			if ((--as.retry) < 0) 
				return;
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: FECRX: %s_%s  (%02x_%02x)\n", 
			  sym_cdx >= 0 ? monitor_names[sym_cdx] : "?",
			  sym_rx >= 0 ? monitor_names[sym_rx] : "?",
			  chdx & 0x7f, chrx & 0x7f);
		l1_fsk_rx_request(as.rxtime, 1000000/100/RXOVERSAMPLING, as.rxfreqdev, 100,
				  id, 14*RXOVERSAMPLING, s);
#ifdef FREQ_TRACKING
		l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev-FREQ_TRACKING_DIST, 100, 
				  0x210+2*id, 14, as.trk.rxb1+14*id);
		l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev+FREQ_TRACKING_DIST, 100, 
				  0x211+2*id, 14, as.trk.rxb2+14*id);
#endif /* FREQ_TRACKING */
		as.rxtime += 140000;
		bufprintf(HFAPP_MSG_DATA_STATUS, "AMTOR FEC RECEIVE START\nRetry: %d\n", as.retry);
	}
}

/* --------------------------------------------------------------------- */
/*
 * misc utility functions
 */

static void send_cs(int cs)
{
	static unsigned int csidx[4] = { AMTOR_CS1, AMTOR_CS2, AMTOR_CS3, AMTOR_RQ };

	assert(cs >= 1 && cs <= 4);
	as.txbuf[0] = amtor_encode_table[csidx[cs-1]];
	l1_fsk_tx_request(as.txtime-ap.txdelay, ap.txdelay, as.txfreqdev, 0, 0x100, 1, as.txbuf);
	l1_fsk_tx_request(as.txtime, 1000000/100, as.txfreqdev, ap.txinvert, 0x101, 7, as.txbuf);
	if (cs == 4) 
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TX: RQ\n");
	else
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TX: CS%c\n", '0'+cs);
}

static void send_triple(int mode)
{
	unsigned int bits;

	l1_fsk_tx_request(as.txtime-ap.txdelay, ap.txdelay, as.txfreqdev, 0, 0x100, 1, as.txbuf);
	if (mode == 2) {
		bits = amtor_encode_table[AMTOR_BETA] |
			(amtor_encode_table[AMTOR_ALPHA] << 7) |
			(amtor_encode_table[AMTOR_BETA] << 14);
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TX: BETA_ALPHA_BETA\n");
	} else if (mode) {
		bits = amtor_encode_table[AMTOR_RQ];
		bits |= (bits << 7) | (bits << 14);
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TX: RQ_RQ_RQ\n");
	} else {
		bits = amtor_encode_table[as.txtriple[0]] |
			(amtor_encode_table[as.txtriple[1]] << 7) |
			(amtor_encode_table[as.txtriple[2]] << 14);
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TX: %s_%s_%s\n", monitor_names[as.txtriple[0]],
			  monitor_names[as.txtriple[1]], monitor_names[as.txtriple[2]]);
	}
	as.txbuf[0] = bits & 0xff;
	as.txbuf[1] = (bits >> 8) & 0xff;
	as.txbuf[2] = (bits >> 16) & 0xff;
	l1_fsk_tx_request(as.txtime, 1000000/100, as.txfreqdev, ap.txinvert, 0x101, 21, as.txbuf);
}
	
static int receive_cs(void)
{
	unsigned char csbuf;
	long dev;
	short sym;
#ifdef FREQ_TRACKING
	l1_soft_t trkm, trkl, trkh;
	int trk;
#endif /* FREQ_TRACKING */

	l1_fsk_rx_request(as.rxtime, 1000000/100/RXOVERSAMPLING, as.rxfreqdev, 100, 0x200,
			  7*RXOVERSAMPLING, as.rxbuf);
#ifdef FREQ_TRACKING
	l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev-FREQ_TRACKING_DIST, 100, 0x210, 7, as.trk.rxb1);
	l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev+FREQ_TRACKING_DIST, 100, 0x211, 7, as.trk.rxb2);
#endif /* FREQ_TRACKING */
	while (l1_fsk_wait_request() != 0x200);
	soft_to_hard(as.rxbuf, &csbuf, 7, RXOVERSAMPLING, ap.rxinvert);
	sym = amtor_check_table[csbuf & 0x7f];
	dev = soft_time_dev(as.rxbuf, 7, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
	tmg_add(dev);
#ifdef FREQ_TRACKING
	trk = freq_tracking(as.rxbuf, as.trk.rxb1, as.trk.rxb2, 7, RXOVERSAMPLING, &trkm, &trkl, &trkh);
	amtor_freq_tracking(trk, trkl, trkm, trkh);
#endif /* FREQ_TRACKING */
	if (sym == AMTOR_CS1) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: RX: CS1 dT=%ld\n", dev);
		return 1;
	}
	if (sym == AMTOR_CS2) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: RX: CS2 dT=%ld\n", dev);
		return 2;
	}
	if (sym == AMTOR_CS3) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: RX: CS3 dT=%ld\n", dev);
		return 3;
	}
	if (sym == AMTOR_RQ) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: RX: RQ dT=%ld\n", dev);
		return 4;
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: RX: CS?  sym: %2d  bits: %02x  dT=%ld\n", sym, csbuf & 0x7f, dev);
	return 0;
}

#define PKT_OK      (1<<0)
#define PKT_QRT     (1<<1)
#define PKT_BRKIN   (1<<2)
#define PKT_OVER    (1<<3)
#define PKT_REQUEST (1<<4)
#define PKT_IDLE    (1<<5)

static int receive_triple(void)
{
	long dev;
	unsigned char trp[3];
	unsigned short s1, s2, s3;
	int retval = PKT_OK;
#ifdef FREQ_TRACKING
	l1_soft_t trkm, trkl, trkh;
	int trk = 0;
#endif /* FREQ_TRACKING */

	l1_fsk_rx_request(as.rxtime, 1000000/100/RXOVERSAMPLING, as.rxfreqdev, 100, 
			  0x200, 21*RXOVERSAMPLING, as.rxbuf);
#ifdef FREQ_TRACKING
	l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev-FREQ_TRACKING_DIST, 100, 0x210, 21, as.trk.rxb1);
	l1_fsk_rx_request(as.rxtime, 1000000/100, as.rxfreqdev+FREQ_TRACKING_DIST, 100, 0x211, 21, as.trk.rxb2);
#endif /* FREQ_TRACKING */
	while (l1_fsk_wait_request() != 0x200);
	dev = soft_time_dev(as.rxbuf, 21, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
	tmg_add(dev);
#ifdef FREQ_TRACKING
	trk = freq_tracking(as.rxbuf, as.trk.rxb1, as.trk.rxb2, 21, RXOVERSAMPLING, &trkm, &trkl, &trkh);
	amtor_freq_tracking(trk, trkl, trkm, trkh);
#endif /* FREQ_TRACKING */
	soft_to_hard(as.rxbuf, trp, 21, RXOVERSAMPLING, ap.rxinvert);
	s1 = amtor_check_table[trp[0] & 0x7f];
	s2 = amtor_check_table[((trp[0] >> 7) | (trp[1] << 1)) & 0x7f];
	s3 = amtor_check_table[((trp[1] >> 6) | (trp[2] << 2)) & 0x7f];
	if (s1 >= 35 || s2 >= 35 || s3 >= 35) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: RX: %s_%s_%s   %02x%02x%02x  dT=%ld\n", 
			  (s1 < 35) ? monitor_names[s1] : "?", (s2 < 35) ? monitor_names[s2] : "?",
			  (s3 < 35) ? monitor_names[s3] : "?", trp[2] & 0x1f, trp[1], trp[0], dev);
		return 0;
	}
	monitor_triple("RX: ", s1, s2, s3);
	if (s1 == AMTOR_RQ || s2 == AMTOR_RQ || s3 == AMTOR_RQ)
		return PKT_REQUEST;
	if (s1 == AMTOR_ALPHA && s2 == AMTOR_ALPHA && s3 == AMTOR_ALPHA)
		return PKT_QRT;
	if (s1 == AMTOR_BETA && s2 == AMTOR_ALPHA && s3 == AMTOR_BETA)
		return PKT_OVER;
	if ((s1 >= 32 && s1 != AMTOR_BETA) || (s2 >= 32 && s2 != AMTOR_BETA) || 
	    (s3 >= 32 && s3 != AMTOR_BETA))
		return 0;
	if (s1 == AMTOR_BETA || s2 == AMTOR_BETA || s3 == AMTOR_BETA)
		retval |= PKT_IDLE;
	output_triple(s1, s2, s3);
	as.last_ch = (as.last_ch << 8) | (s1 & 0xff);
	if ((as.last_ch & 0xffffff) == ((SHIFT_NUMBER << 16) | (SYMBOL_PLUS << 8) | SYMBOL_QMARK))
		retval |= PKT_BRKIN;
	as.last_ch = (as.last_ch << 8) | (s2 & 0xff);
	if ((as.last_ch & 0xffffff) == ((SHIFT_NUMBER << 16) | (SYMBOL_PLUS << 8) | SYMBOL_QMARK))
		retval |= PKT_BRKIN;
	as.last_ch = (as.last_ch << 8) | (s3 & 0xff);
	if ((as.last_ch & 0xffffff) == ((SHIFT_NUMBER << 16) | (SYMBOL_PLUS << 8) | SYMBOL_QMARK))
		retval |= PKT_BRKIN;
	return retval;
}

static int encode_triple(void)
{
	if (as.flags & FLG_BECOMEIRS) {
		as.flags &= ~FLG_BECOMEIRS;
		as.txtriple[0] = SHIFT_NUMBER;
		as.txtriple[1] = SYMBOL_PLUS;
		as.txtriple[2] = SYMBOL_QMARK;
		return PKT_BRKIN;
	}
	if (as.flags & FLG_QRT) {
		as.flags &= ~FLG_QRT;
		as.txtriple[0] = AMTOR_ALPHA;
		as.txtriple[1] = AMTOR_ALPHA;
		as.txtriple[2] = AMTOR_ALPHA;
		return PKT_QRT;
	}
	as.txtriple[0] = get_tx_char();
	as.txtriple[1] = get_tx_char();
	as.txtriple[2] = get_tx_char();
	return 0;
}

/* --------------------------------------------------------------------- */

extern __inline__ void cycle_end(void)
{
	as.rxtime += 450000;
	as.txtime += 450000;
}

extern __inline__ int retry(void)
{
	if ((--as.retry) <= 0)
		return 1;
	return 0;
}

/* --------------------------------------------------------------------- */

static void disp_status(const char *modename)
{
// was STATUS, changed by Günther
	bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR %s\n%s rt:%d df:%d/%d\nCMD:%s%s%s\n", modename, 
		  as.is_master ? "MASTER" : "SLAVE", as.retry, as.txfreqdev, as.rxfreqdev,
		  (as.flags & FLG_QRT) ? " QRT" : "", (as.flags & FLG_BECOMEIRS) ? " IRS" : "", 
		  (as.flags & FLG_BECOMEISS) ? " ISS" : "");
}

/* --------------------------------------------------------------------- */

static void arq_statemachine(void)
{
	int eflg, i, rq;

	l1_fsk_clear_requests();
	kbd_clear_and_fill(NULL, 0);
	as.flags = 0;
	as.figmode = 0;
	as.lcflag = 0;
	as.last_ch = 0;
	
	if (as.is_master) {
		send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_MASTERCONNECT, ERR_NOERR);
		goto tx_cs1;
	}
	send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_SLAVECONNECT, ERR_NOERR);
	goto rx_cs1;
	
tx_cs1:
	as.retry = ap.retry;
	eflg = encode_triple();
	rq = 0;
	for (;;) {
		send_triple(rq);
		disp_status("ARQ TX CS1");
		cycle_end();
		i = receive_cs();
		if (i == 2) {
			if (eflg & PKT_QRT) {
				send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_NOERR);
				return;
			}
			goto tx_cs2;
		} else if (i == 3)
			goto tx_rx;
		else if (i != 1 && retry()) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
		rq = i != 1;
	}
	
	
tx_cs2:
	as.retry = ap.retry;
	eflg = encode_triple();
	rq = 0;
	for (;;) {
		send_triple(rq);
		disp_status("ARQ TX CS2");
		cycle_end();
		i = receive_cs();
		if (i == 1) {
			if (eflg & PKT_QRT) {
				send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_NOERR);
				return;
			}
			goto tx_cs1;
		} else if (i == 3)
			goto tx_rx;
		else if (i != 2 && retry()) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
		rq = i != 2;
	}
	
  
tx_rx:
	as.retry = ap.retry;
	as.flags &= ~FLG_BECOMEIRS;
	send_short_msg(HFAPP_MSG_STATE_AMTOR_IRS, ERR_NOERR);
	for (;;) {
		send_triple(2);
		disp_status("ARQ TX->RX");
		cycle_end();
		i = receive_cs();
		if (i == 4) {
			if (as.is_master)
				as.txtime += 140000;
			else
				as.rxtime -= 140000;
			goto rx_cs1;
		} else if (i != 3 && retry()) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}
	
	
rx_cs1:
	as.retry = ap.retry;
	for (;;) {
		send_cs(1);
		disp_status("ARQ RX CS1");
		cycle_end();
		i = receive_triple();
		if (i & PKT_QRT) 
			goto rx_qrt_cs2;
		if ((i & PKT_BRKIN) || (i && (as.flags & FLG_BECOMEISS)))
			goto rx_tx_1;
		if (i & PKT_OK)
			goto rx_cs2;
		if (!i  && retry()) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}
		
rx_cs2:
	as.retry = ap.retry;
	for (;;) {
		send_cs(2);
		disp_status("ARQ RX CS2");
		cycle_end();
		i = receive_triple();
		if (i & PKT_QRT)
			goto rx_qrt_cs1;
		if ((i & PKT_BRKIN) || (i && (as.flags & FLG_BECOMEISS)))
			goto rx_tx_1;
		if (i & PKT_OK)
			goto rx_cs1;
		if (!i  && retry()) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}
	
rx_qrt_cs1:
	as.retry = 5;
	send_cs(1);
	for (;;) {
		disp_status("ARQ RX QRT CS1");
		cycle_end();
		i = receive_triple();
		if (i & PKT_QRT) {
			send_cs(1);
			as.retry = 5;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}
	
	
rx_qrt_cs2:
	as.retry = 5;
	send_cs(2);
	for (;;) {
		disp_status("ARQ RX QRT CS1");
		cycle_end();
		i = receive_triple();
		if (i & PKT_QRT) {
			send_cs(2);
			as.retry = 5;
		} else if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}
	

rx_tx_1:
	as.retry = ap.retry;
	as.flags &= ~FLG_BECOMEISS;
	send_short_msg(HFAPP_MSG_STATE_AMTOR_ISS, ERR_NOERR);
	for (;;) {
		send_cs(3);
		disp_status("ARQ RX->TX 1");
		cycle_end();
		i = receive_triple();
		if (i & PKT_OVER)
			goto rx_tx_2;
		if (!i  && retry()) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}

	
rx_tx_2:
	if (as.is_master) {
		send_triple(1);
		as.txtime -= 140000;
		disp_status("ARQ RX->TX 2 (MASTER)");
		cycle_end();
		i = receive_cs();
		if (i == 1) 
			goto tx_cs1;
	} else {
		as.rxtime += 140000;
	}
	as.retry = ap.retry;
	send_short_msg(HFAPP_MSG_STATE_AMTOR_ISS, ERR_NOERR);
	for (;;) {
		send_triple(1);
		disp_status("ARQ RX->TX 2");
		cycle_end();
		i = receive_cs();
		if (i == 1) 
			goto tx_cs1;
		else if (i == 3)
			goto tx_rx;
		if (retry()) {
			send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
			return;
		}
	}
}

/* --------------------------------------------------------------------- */

void amtor_monitor_init(l1_time_t tm)
{
	int i;

	as.mon_mute = tm - 440000;
	for (i = 0; i < MONNUM; i++)
		as.mon[i].state = -1;
}

/* --------------------------------------------------------------------- */

int amtor_monitor_arq(l1_time_t tm, unsigned char pk01, unsigned char pk02, 
		      unsigned char pk03, unsigned char pk11, unsigned char pk12,
		      unsigned char pk13, l1_soft_t *samples, int freqdev)
{
	short t01, t02, t03, t11, t12, t13;
	long dev;
	int i, j, k;

	if (ap.rxinvert) {
		pk01 ^= 0x7f;
		pk02 ^= 0x7f;
		pk03 ^= 0x7f;
		pk11 ^= 0x7f;
		pk12 ^= 0x7f;
		pk13 ^= 0x7f;
	}
	t01 = amtor_check_table[(pk01 &= 0x7f)];
	t02 = amtor_check_table[(pk02 &= 0x7f)];
	t03 = amtor_check_table[(pk03 &= 0x7f)];
	t11 = amtor_check_table[(pk11 &= 0x7f)];
	t12 = amtor_check_table[(pk12 &= 0x7f)];
	t13 = amtor_check_table[(pk13 &= 0x7f)];	
	if (t01 == ap.mycall[0] && t02 == AMTOR_RQ && t03 == ap.mycall[1] &&
	    t11 == ap.mycall[2] && t12 == ap.mycall[3] && t13 == AMTOR_RQ) {
		dev = soft_time_dev(samples-20*STANDBY_OVERSAMPLING_100, 21, STANDBY_OVERSAMPLING_100, 
				    1000000/100/STANDBY_OVERSAMPLING_100)+
			soft_time_dev(samples-20*STANDBY_OVERSAMPLING_100-450*STANDBY_OVERSAMPLING_100/10, 21, 
				      STANDBY_OVERSAMPLING_100, 1000000/100/STANDBY_OVERSAMPLING_100);
		dev /= 2;
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: call %c%c%c%c  dT=%ld\n", 
			  amtor_chars[ap.mycall[0]], amtor_chars[ap.mycall[1]], 
			  amtor_chars[ap.mycall[2]], amtor_chars[ap.mycall[3]], dev);
		as.is_master = 0;
		as.rxtime = tm-20*1000000/100+dev;
		as.txtime = as.rxtime + 210000 + ap.txdelay + 10000;
		as.rxfreqdev = as.txfreqdev = freqdev;
		l1_fsk_clear_requests();
		arq_statemachine();
		return 1;
	}
	if (freqdev)
		return 0;
	if (amtor_check_table[0x7f ^ pk01] == ap.mycall[0] &&
	    amtor_check_table[0x7f ^ pk02] == AMTOR_RQ &&
	    amtor_check_table[0x7f ^ pk03] == ap.mycall[1] &&
	    amtor_check_table[0x7f ^ pk11] == ap.mycall[2] &&
	    amtor_check_table[0x7f ^ pk12] == ap.mycall[3] &&
	    amtor_check_table[0x7f ^ pk13] == AMTOR_RQ) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: inverted call %c%c%c%c\n", 
			  amtor_chars[ap.mycall[0]], amtor_chars[ap.mycall[1]], 
			  amtor_chars[ap.mycall[2]], amtor_chars[ap.mycall[3]]);
		return 0;
	}
	if (/*t01 < 0 || t02 < 0 || t03 < 0 ||*/ t11 < 0 || t12 < 0 || t13 < 0)
		return 0;
	if ((unsigned)(tm - as.mon_mute) > 9000)
		monitor_triple("STDBY: RX: ", t11, t12, t13);
	as.mon_mute = tm;
	dev = soft_time_dev(samples-20*STANDBY_OVERSAMPLING_100, 21, STANDBY_OVERSAMPLING_100, 
			    1000000/100/STANDBY_OVERSAMPLING_100);
	tm += dev;
	for (i = k = 0, j = 10; i < MONNUM; i++) {
		if (j >= as.mon[i].state) {
			j = as.mon[i].state;
			k = i;
		}
		if (as.mon[i].state < 0)
			continue;
		if ((signed)(tm - as.mon[i].tm) > 15*450000) {
			j = -1;
			k = i;
			as.mon[i].state = -1;
			continue;
		}
		while ((signed)(tm - as.mon[i].tm) > 100000 && as.mon[i].state >= 0) {
			as.mon[i].tm += 450000;
			as.mon[i].state--;
		}
		if (j >= as.mon[i].state) {
			j = as.mon[i].state;
			k = i;
		}
		if (as.mon[i].state < 0)
			continue;
		if (labs((signed)(tm + 450000 - as.mon[i].tm)) < 5000)
			return 0;
		if (labs((signed)(tm - as.mon[i].tm)) < 5000) {
			if (as.mon[i].state < 8)
				as.mon[i].state++;
			if (as.mon[i].state > 3 && (t11 < 32 || t11 == AMTOR_BETA) && 
			    (t12 < 32 || t12 == AMTOR_BETA) && (t13 < 32 || t13 == AMTOR_BETA))
				output_triple(t11, t12, t13);
			as.mon[i].tm = tm + 450000;
			/*
			bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: STANDBY: pos %d state %d time %08lu\n", 
			*/
			/* i (guenther) took this out because monitor 
			makes my slow machine crash with this output */
			bufprintf(HFAPP_MSG_DATA_MONITOR, ".");
			return 0;
		}
	}
	as.mon[k].tm = tm + 450000;
	as.mon[k].state = 1;
	for (i = 0; i < MONNUM; i++)
		if (as.mon[i].state >= 0)
			bufprintf(HFAPP_MSG_DATA_MONITOR, 
			    /* "AMTOR: STANDBY: pos %d state %d time %08lu\n", 
			      i, as.mon[i].state, as.mon[i].tm); */
			      ".");
	return 0;
}

/* --------------------------------------------------------------------- */

int amtor_monitor_fec(l1_time_t tm, unsigned char rx0, unsigned char dx0,
		      unsigned char rx1, unsigned char dx1,
		      unsigned char rx2, unsigned char dx2,
		      unsigned char rx3, unsigned char dx3,
		      unsigned char rx4, unsigned char dx4,
		      unsigned char rx5, unsigned char dx5, l1_soft_t *samples, int freqdev)
{
	unsigned char ps1 = amtor_encode_table[AMTOR_PS1];
	unsigned char ps2 = amtor_encode_table[AMTOR_PS2];
	unsigned char invps1 = ps1 ^ 0x7f;
	unsigned char invps2 = ps2 ^ 0x7f;

	if (ap.rxinvert) {
		rx0 ^= 0x7f;
		dx0 ^= 0x7f;
		rx1 ^= 0x7f;
		dx1 ^= 0x7f;
		rx2 ^= 0x7f;
		dx2 ^= 0x7f;
		rx3 ^= 0x7f;
		dx3 ^= 0x7f;
		rx4 ^= 0x7f;
		dx4 ^= 0x7f;
		rx5 ^= 0x7f;
		dx5 ^= 0x7f;
	}
	rx0 &= 0x7f;
	dx0 &= 0x7f;
	rx1 &= 0x7f;
	dx1 &= 0x7f;
	rx2 &= 0x7f;
	dx2 &= 0x7f;
	rx3 &= 0x7f;
	dx3 &= 0x7f;
	rx4 &= 0x7f;
	dx4 &= 0x7f;
	rx5 &= 0x7f;
	dx5 &= 0x7f;
	if (rx0 == ps1 && rx1 == ps1 && rx2 == ps1 && rx3 == ps1 &&
	    dx2 == ps2 && dx3 == ps2 && dx4 == ps2 && dx5 == ps2) {
		as.last_ch = (dx1 << 8) | dx0;
		as.rxtime = tm + 1000000/100 + 
			soft_time_dev(samples - (12 * 7 - 1) * STANDBY_OVERSAMPLING_100, 12*7, STANDBY_OVERSAMPLING_100,
				      1000000/100/STANDBY_OVERSAMPLING_100);
		as.is_master = 0;
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: FEC phasing\n");
		as.rxfreqdev = as.txfreqdev = freqdev;
		fec_rx();
		return 1;
	}
	if (freqdev == 0 && rx0 == invps1 && rx1 == invps1 && rx2 == invps1 && rx3 == invps1 &&
	    dx2 == invps2 && dx3 == invps2 && dx4 == invps2 && dx5 == invps2) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: inverted FEC phasing\n");
		return 0;
	}
	return 0;
}

/* --------------------------------------------------------------------- */

static void fec_tx(int sel, const char *mode)
{
	int inv = ap.txinvert;
	unsigned long id;
	int cnt, i;
	unsigned int bits = amtor_encode_table[AMTOR_PS2] | (amtor_encode_table[AMTOR_PS1] << 7);
	unsigned char *bp;
	unsigned short txch;
	
	l1_fsk_clear_requests();
	kbd_clear_and_fill(NULL, 0);
	as.flags = 0;
	as.figmode = 0;
	as.lcflag = 0;
	as.txbuf[0] = as.txbuf[2] = as.txbuf[4] = as.txbuf[6] = bits & 0xff;
	as.txbuf[1] = as.txbuf[3] = as.txbuf[5] = as.txbuf[7] = bits >> 8;
	as.txtime = l1_get_current_time() + 100000;
	as.rxfreqdev = as.txfreqdev = 0;
	as.last_ch = amtor_encode_table[AMTOR_PS1];
	as.last_ch |= (as.last_ch << 8);
	for (id = 0; id < 4; id++) {
		l1_fsk_tx_request(as.txtime, 1000000/100, as.txfreqdev, inv, id, 14, as.txbuf+2*id);
		as.txtime += 140000;
	}
	bufprintf(HFAPP_MSG_DATA_STATUS, "%s\nSYNC", mode);
	send_short_msg(HFAPP_MSG_STATE_AMTOR_FEC_CONNECT, ERR_NOERR);
	for (cnt = 0; cnt < 10; cnt++) {
		id = l1_fsk_wait_request();
		assert(id < 4);
		l1_fsk_tx_request(as.txtime, 1000000/100, as.txfreqdev, inv, id, 14, as.txbuf+2*id);
		as.txtime += 140000;
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: FEXTX: PS1/PS2\n");
	}
	if (sel) {
		bufprintf(HFAPP_MSG_DATA_STATUS, "%s\nSELFEC CALL\n", mode);
		inv = !inv;
		for (cnt = 0; cnt < 5; cnt++) {
			for (i = 0; i < 5; i++) {				
				id = l1_fsk_wait_request();
				assert(id < 4);
				bp = as.txbuf+2*id;
				txch = (i < 4) ? ap.selfeccall[i] : AMTOR_BETA;
				as.last_ch = (as.last_ch << 8) | amtor_encode_table[txch];
				bits = (as.last_ch & 0x7f) | ((as.last_ch >> 9) & 0x3f80);
				bp[0] = bits & 0xff;
				bp[1] = bits >> 8;
				l1_fsk_tx_request(as.txtime, 1000000/100, as.txfreqdev, inv, id, 14, bp);
				as.txtime += 140000;
				bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: FEXTX: %s\n", (i < 4) ? monitor_names[txch] : "BETA");
			}
		}
	}
	bufprintf(HFAPP_MSG_DATA_STATUS, "%s\nDATA\n", mode);
	while (!(as.flags & FLG_QRT)) {
		id = l1_fsk_wait_request();
		assert(id < 4);
		bp = as.txbuf+2*id;
		as.last_ch <<= 8;
		txch = get_tx_char();
		if (txch == AMTOR_BETA && !sel) {
			as.last_ch |= amtor_encode_table[AMTOR_PS1];
			bits = amtor_encode_table[AMTOR_PS2];
			bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: FEXTX: PS1/PS2\n");
		} else {
			bits = amtor_encode_table[txch];
			as.last_ch |= bits;
			bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: FEXTX: %s\n", monitor_names[txch]);
		}
		bits |= ((as.last_ch >> 9) & 0x3f80);
		bp[0] = bits & 0xff;
		bp[1] = bits >> 8;
		l1_fsk_tx_request(as.txtime, 1000000/100, as.txfreqdev, inv, id, 14, bp);
		as.txtime += 140000;
	}
	bufprintf(HFAPP_MSG_DATA_STATUS, "%s\nQRT\n", mode);
	for (i = 0; i < 2; i++) {
		id = l1_fsk_wait_request();
		assert(id < 4);
		bp = as.txbuf+2*id;
		as.last_ch = (as.last_ch << 8) | amtor_encode_table[AMTOR_ALPHA];
		bits = (as.last_ch & 0x7f) | ((as.last_ch >> 9) & 0x3f80);
		bp[0] = bits & 0xff;
		bp[1] = bits >> 8;
		l1_fsk_tx_request(as.txtime, 1000000/100, as.txfreqdev, inv, id, 14, bp);
		as.txtime += 140000;
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: FEXTX: ALPHA\n");
	}
	for (i = 0; i < 4; i++) {
		id = l1_fsk_wait_request();
		assert(id < 4);
	}
	send_short_msg(HFAPP_MSG_STATE_AMTOR_FEC_DISCONNECT, ERR_NOERR);
	return;
}

/* --------------------------------------------------------------------- */

static int master_verify_cs(int callflg)
{
	unsigned char csbuf;
	unsigned char cs1 = amtor_encode_table[AMTOR_CS1];
	unsigned int bits;
	long dev;

	if (callflg)
		bits = amtor_encode_table[ap.destcall[2]] |
			(amtor_encode_table[ap.destcall[3]] << 7) |
			(amtor_encode_table[AMTOR_RQ] << 14);
	else
		bits = amtor_encode_table[ap.destcall[0]] |
			(amtor_encode_table[AMTOR_RQ] << 7) |
			(amtor_encode_table[ap.destcall[1]] << 14);
	as.txbuf[0] = bits & 0xff;
	as.txbuf[1] = (bits >> 8) & 0xff;
	as.txbuf[2] = (bits >> 16) & 0xff;
	l1_fsk_tx_request(as.txtime, 1000000/100, as.txfreqdev, ap.txinvert, 0x101, 21, as.txbuf);
	l1_fsk_tx_request(as.txtime+450000-ap.txdelay, ap.txdelay, as.txfreqdev, 0, 0x100, 1, as.txbuf);
	l1_fsk_rx_request(as.rxtime, 1000000/100/RXOVERSAMPLING, as.rxfreqdev, 100, 0x200,
			  7*RXOVERSAMPLING, as.rxbuf);
	if (callflg)
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TX: %s_%s_%s\n", 
			  monitor_names[ap.destcall[2]], monitor_names[ap.destcall[3]], monitor_names[AMTOR_RQ]);
	else
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TX: %s_%s_%s\n", 
			  monitor_names[ap.destcall[0]], monitor_names[AMTOR_RQ], monitor_names[ap.destcall[1]]);
	while (l1_fsk_wait_request() != 0x200);
	soft_to_hard(as.rxbuf, &csbuf, 7, RXOVERSAMPLING, ap.rxinvert);
	dev = soft_time_dev(as.rxbuf, 7, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
	if ((csbuf & 0x7f) == cs1) {
		bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: RX: CS1  dT=%ld\n", dev);
		return 1;
	}
	bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: RX: CS?  bits=%02x\n", csbuf & 0x7f);
	return 0;
}

/* --------------------------------------------------------------------- */

static int master_call(void)
{
	unsigned int nsamps = (450000-210000-ap.txdelay)/(1000000/100)*RXOVERSAMPLING;
	int i;
	l1_soft_t *s;
	unsigned char csbuf;
	unsigned char cs1 = amtor_encode_table[AMTOR_CS1];
	unsigned int bits;
	int callflg = 0;

	as.txtime = l1_get_current_time() + 200000;
	as.rxfreqdev = as.txfreqdev = 0;
	as.retry = ap.retry;
	as.is_master = 1;
	l1_fsk_tx_request(as.txtime-ap.txdelay, ap.txdelay, as.txfreqdev, 0, 0x100, 1, as.txbuf);
	for (;;) {
		if (callflg)
			bits = amtor_encode_table[ap.destcall[2]] |
				(amtor_encode_table[ap.destcall[3]] << 7) |
				(amtor_encode_table[AMTOR_RQ] << 14);
		else
			bits = amtor_encode_table[ap.destcall[0]] |
				(amtor_encode_table[AMTOR_RQ] << 7) |
				(amtor_encode_table[ap.destcall[1]] << 14);
		as.txbuf[0] = bits & 0xff;
		as.txbuf[1] = (bits >> 8) & 0xff;
		as.txbuf[2] = (bits >> 16) & 0xff;
		l1_fsk_tx_request(as.txtime, 1000000/100, as.txfreqdev, ap.txinvert, 0x101, 21, as.txbuf);
		l1_fsk_tx_request(as.txtime+450000-ap.txdelay, ap.txdelay, as.txfreqdev, 0, 0x100, 1, as.txbuf);
		as.rxtime = as.txtime+210000;
		l1_fsk_rx_request(as.rxtime, 1000000/100/RXOVERSAMPLING, as.rxfreqdev, 100, 0x200,
				  nsamps, as.rxbuf);
		if (callflg)
			bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TX: %s_%s_%s\n", 
				  monitor_names[ap.destcall[2]], monitor_names[ap.destcall[3]], monitor_names[AMTOR_RQ]);
		else
			bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: TX: %s_%s_%s\n", 
				  monitor_names[ap.destcall[0]], monitor_names[AMTOR_RQ], monitor_names[ap.destcall[1]]);
		disp_status("ARQ MASTER CALL");
		while (l1_fsk_wait_request() != 0x200);
		for (s = as.rxbuf, i = 0; i < nsamps-RXOVERSAMPLING*(7-1); 
		     i++, s++, as.rxtime += 1000000/100/RXOVERSAMPLING) {
			soft_to_hard(s, &csbuf, 7, RXOVERSAMPLING, ap.rxinvert);
			if ((csbuf & 0x7f) == cs1) {
				as.rxtime += 450000 + soft_time_dev(s, 7, RXOVERSAMPLING, 1000000/100/RXOVERSAMPLING);
				as.txtime += 450000;
				bufprintf(HFAPP_MSG_DATA_MONITOR, "AMTOR: RX: CS1  dT=%ld\n", as.rxtime-as.txtime-210000);
				callflg = !callflg;
				if (master_verify_cs(callflg)) {
					as.txtime += 450000;
					return 1;
				}
			}
		}
		if ((--as.retry) <= 0)
			return -1;
		as.txtime += 450000;
		callflg = !callflg;
	}
}

/* --------------------------------------------------------------------- */

void *mode_amtor_arq(void *dummy)
{
	int i;

	l1_fsk_clear_requests();
	errprintf(SEV_INFO, "mode: amtor arq call\n");
	i = master_call();
	if (i >= 0)
		arq_statemachine();
	else
	    send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_MASTERCONNECT, ERR_TIMEOUT);
	    send_short_msg(HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT, ERR_TIMEOUT);
	return NULL;
}

void *mode_amtor_colfec(void *dummy)
{
	errprintf(SEV_INFO, "mode: amtor colfec\n");
	fec_tx(0, "AMTOR COLLECTIVE FEC TRANSMIT");
	return NULL;
}

void *mode_amtor_selfec(void *dummy)
{
	errprintf(SEV_INFO, "mode: amtor selfec\n");
	fec_tx(1, "AMTOR SELECTIVE FEC TRANSMIT");
	return NULL;
}

/* --------------------------------------------------------------------- */

