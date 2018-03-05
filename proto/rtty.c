/*****************************************************************************/

/*
 *      rtty.c  --  RTTY "uart" implementation.
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

#include "l1.h"
#include "os.h"
#include "hfapp.h"
#include "util.h"
#include "rtty.h"

#include <assert.h>
#include <stdio.h>

/* --------------------------------------------------------------------- */

#define SHIFT_NUMBER   27
#define SHIFT_LETTER   31

#define IDLE_SHIFT_FREQ 10

/* --------------------------------------------------------------------- */

#define INVAL "\001"
#define BELL  "\007" 
#define WRU   "$"

#define LETTERS INVAL "E\nA SIU\rDRJNFCKTZLWHYPQOBG" INVAL "MXV" INVAL
#define NUMBERS_DL INVAL"3\n- '87\r" WRU "4ß,Ä:(5+)2Ü6019?Ö" INVAL "./=" INVAL
#define NUMBERS_GB INVAL"3\n- '87\r" WRU "4" BELL ",%:(5+)2#6019?@" INVAL "./=" INVAL
#define NUMBERS_USA INVAL "3\n- " BELL "87\r" WRU "4',!:(5\")2*6019?&" INVAL "./;" INVAL

static const char rtty_chars[65] = LETTERS NUMBERS_DL;

/* --------------------------------------------------------------------- */

static struct {
	unsigned int baud;
	int rxinvert, txinvert;
} rp = { 45, 0, 0 };

static struct {
	unsigned int figmode;
	int idle_cnt;
} rs;

/* --------------------------------------------------------------------- */

#define NUMBUF            4
#define BUFLEN          128
#define RXOVERSAMPLING    5

static l1_time_t buftimes[NUMBUF];
static l1_soft_t buf[2*NUMBUF][BUFLEN];

#define NUMTXCHARS 4

static l1_time_t txtimes[NUMTXCHARS];
static unsigned char txchars[NUMTXCHARS];

/* --------------------------------------------------------------------- */

void rtty_set_params(unsigned int baud, int rxinvert, int txinvert)
{
	errprintf(SEV_INFO, "rtty params: baud %u invert%s%s\n",
		  baud, rxinvert ? " RX" : "", txinvert ? " TX" : "");
	if (baud >= 10 && baud <= 1000)
		rp.baud = baud;
	rp.rxinvert = !!rxinvert;
	rp.txinvert = !!txinvert;
}

/* --------------------------------------------------------------------- */

void rtty_reset_uppercase(void)
{
	rs.figmode = 0;
}

void rtty_reset_figurecase(void)
{
	rs.figmode = 0x20;
}

/* --------------------------------------------------------------------- */

void *mode_rtty_rx(void *dummy)
{
	int i, j;
	l1_soft_t *s;
	l1_time_t tm_inc = 1000000/rp.baud/RXOVERSAMPLING;
	int deaf = 0;
	unsigned char rxchar;

       	l1_clear_requests();
	errprintf(SEV_INFO, "mode: rtty rx\n");
	bufprintf(HFAPP_MSG_DATA_STATUS, "RTTY RX %u BAUD LETTERS", rp.baud);
	rs.figmode = 0;
	memset(buf, 0, sizeof(buf));
	buftimes[0] = l1_get_current_time() + 100000;
	for (i = 1; i < NUMBUF; i++)
		buftimes[i] = buftimes[i-1] + BUFLEN * tm_inc;
	for (i = 0; i < NUMBUF; i++)
		l1_fsk_rx_request(buftimes[i], tm_inc, 0, rp.baud, i, BUFLEN, buf[i]);
	send_short_msg(HFAPP_MSG_STATE_RTTY_RX, ERR_NOERR);
	for (;;) {
		i = l1_wait_request();
		errprintf(SEV_INFO, "rttyrx: req %d ready\n", i);
		assert(i >= 0);
		assert(i < NUMBUF);
		memcpy(s = buf[i+NUMBUF], buf[i], sizeof(buf[i]));
		for (j = 0; j < BUFLEN; j++, s++) {
 printf("%c", '0' + (*s >= 0));
			if (deaf > 0) {
				deaf--;
				continue;
			}
			if ((!rp.rxinvert && s[-RXOVERSAMPLING*6-1-RXOVERSAMPLING/2] >= 0 && 
			     s[-RXOVERSAMPLING*6-RXOVERSAMPLING/2] < 0) || 
			    (rp.rxinvert && s[-RXOVERSAMPLING*6-1-RXOVERSAMPLING/2] < 0 && 
			     s[-RXOVERSAMPLING*6-RXOVERSAMPLING/2] >= 0)) {
				soft_to_hard(s-RXOVERSAMPLING*5, &rxchar, 6, RXOVERSAMPLING, rp.rxinvert);
				if (!(rxchar & 0x20))
					continue;
				deaf = 6*RXOVERSAMPLING;
				rxchar &= 0x1f;
				bufprintf(HFAPP_MSG_DATA_MONITOR, "%02x\n", (int)rxchar);
				if (rxchar == SHIFT_NUMBER)
					rs.figmode = 0x20;
				else if (rxchar == SHIFT_LETTER)
					rs.figmode = 0;
				else
					bufprintf(HFAPP_MSG_DATA_RECEIVE, "%c", rtty_chars[rxchar + rs.figmode]);
				bufprintf(HFAPP_MSG_DATA_STATUS, "RTTY RX %u BAUD %s", rp.baud, rs.figmode ? "FIGURES" : "LETTERS");
			}
		}
  printf("\n");
  fflush(stdout);
		buftimes[i] += NUMBUF * BUFLEN * tm_inc;
		l1_fsk_rx_request(buftimes[i], tm_inc, 0, rp.baud, i, BUFLEN, buf[i]);
	}
}

/* --------------------------------------------------------------------- */

static unsigned char rtty_getcharbits(void)
{
	unsigned char *bp;
	unsigned short ch;
	int i;

	for (;;) {
		kbd_negack();
		if ((ch = kbd_get()) == KBD_EOF) {
			if ((--rs.idle_cnt) <= 0) {
				rs.idle_cnt = IDLE_SHIFT_FREQ;
				return (rs.figmode ? SHIFT_NUMBER << 1 : SHIFT_LETTER << 1) | 0xc0;
			}
			return 0xff;
		}
		ch &= KBD_CHAR;
		if (ch >= 'a' && ch <= 'z')
			ch -= 'a'-'A';
		else if (ch == (((unsigned)'ä') & 0xff))
			ch = 'Ä';
		else if (ch == (((unsigned)'ö') & 0xff))
			ch = 'Ö';
		else if (ch == (((unsigned)'ü') & 0xff))
			ch = 'Ü';
		if (!(bp = memchr(rtty_chars, ch, 64))) {
			kbd_ack();
			continue;
		}
		i = bp - (unsigned char *)rtty_chars;
		if ((i & 0x20) != rs.figmode) {
			rs.figmode = (i & 0x20);
			return (rs.figmode ? SHIFT_NUMBER << 1 : SHIFT_LETTER << 1) | 0xc0;
		}
		kbd_ack();
		return (i << 1) | 0xc0;
	}
}

/* --------------------------------------------------------------------- */

void *mode_rtty_tx(void *dummy)
{
	l1_time_t tm_inc = 1000000/rp.baud;
	l1_time_t tm_chinc = 7500000/rp.baud;
	int i;

       	l1_clear_requests();
	errprintf(SEV_INFO, "mode: rtty rx\n");
	bufprintf(HFAPP_MSG_DATA_STATUS, "RTTY TX %u BAUD LETTERS", rp.baud);
	rs.idle_cnt = IDLE_SHIFT_FREQ;
	rs.figmode = 0;
	memset(txchars, 0xff, sizeof(txchars));
	txtimes[0] = l1_get_current_time() + 100000;
	for (i = 1; i < NUMTXCHARS; i++)
		txtimes[i] = txtimes[i-1] + tm_chinc;
	for (i = 0; i < NUMTXCHARS; i++)
		l1_fsk_tx_request(txtimes[i], tm_inc, 0, rp.txinvert, i, 8, txchars+i);
	send_short_msg(HFAPP_MSG_STATE_RTTY_TX, ERR_NOERR);
	for (;;) {
		i = l1_wait_request();
		errprintf(SEV_INFO, "rttytx: req %d ready\n", i);
		assert(i >= 0);
		assert(i < NUMTXCHARS);
		txchars[i] = rtty_getcharbits();
		if (txchars[i] != 0xff)
			bufprintf(HFAPP_MSG_DATA_MONITOR, "%02x\n", (txchars[i] >> 1) & 0x1f);
		txtimes[i] += NUMTXCHARS*tm_chinc;
		l1_fsk_tx_request(txtimes[i], tm_inc, 0, rp.txinvert, i, 8, txchars+i);
		if (i == 0)
			bufprintf(HFAPP_MSG_DATA_STATUS, "RTTY TX %u BAUD %s", rp.baud, rs.figmode ? "FIGURES" : "LETTERS");
	}
}

/* --------------------------------------------------------------------- */
