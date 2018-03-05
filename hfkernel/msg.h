/*****************************************************************************/

/*
 *      msg.h  --  Messages hfkernel <--> hfterm.
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
 *
 */

/*****************************************************************************/
      
#ifndef _MSG_H
#define _MSG_H

extern void *io_process(void *dummy);
extern void *l1_process(void *dummy);
extern void process_sample_msg(unsigned int u);
extern void check_sample_msg_ready(void);

extern pthread_t thr_l2;

/* --------------------------------------------------------------------- */

/*
 * Bittypes
 */

#ifndef HAVE_BITTYPES

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
typedef int int8_t __attribute__((__mode__(__QI__)));
typedef unsigned int u_int8_t __attribute__((__mode__(__QI__)));
typedef int int16_t __attribute__((__mode__(__HI__)));
typedef unsigned int u_int16_t __attribute__((__mode__(__HI__)));
typedef int int32_t __attribute__((__mode__(__SI__)));
typedef unsigned int u_int32_t __attribute__((__mode__(__SI__)));
typedef int int64_t __attribute__((__mode__(__DI__)));
typedef unsigned int u_int64_t __attribute__((__mode__(__DI__)));
#else
typedef char /* deduced */ int8_t __attribute__((__mode__(__QI__)));
typedef unsigned char /* deduced */ u_int8_t __attribute__((__mode__(__QI__)));
typedef short /* deduced */ int16_t __attribute__((__mode__(__HI__)));
typedef unsigned short /* deduced */ u_int16_t __attribute__((__mode__(__HI__)));
typedef long /* deduced */ int32_t __attribute__((__mode__(__SI__)));
typedef unsigned long /* deduced */ u_int32_t __attribute__((__mode__(__SI__)));
typedef long long /* deduced */ int64_t __attribute__((__mode__(__DI__)));
typedef unsigned long long /* deduced */ u_int64_t __attribute__((__mode__(__DI__)));
#endif

#endif /* !HAVE_BITTYPES */

/* --------------------------------------------------------------------- */
/*
 * the size of the messages is aligned to 32bit quantities
 * to facilitate processing on machines not able to do unaligned accesses
 */

#define HFAPP_MSG_ALIGN(x) (((x)+3)&(~3))
#define HFAPP_MSG_ALIGNED(x) ((((unsigned int)(x))&3)==0)

/* --------------------------------------------------------------------- */

struct hfapp_msg_hdr {
	u_int32_t type;
	u_int32_t len;
	u_int32_t err;
};

struct hfapp_msg {
	struct hfapp_msg_hdr hdr;
	union {
		unsigned char b[256];
		u_int32_t u;

		struct hfapp_msg_general_params {
			u_int16_t beaconpause;
		} generalpar;
		
		struct hfapp_msg_fsk_params {
			u_int16_t freq[2];
		} fpar;

		struct hfapp_msg_cw_params {
			u_int16_t wpm;
			u_int16_t tone;
			unsigned char farnsworth;
			unsigned char dtr;
		} cwpar;

		struct hfapp_msg_rtty_params {
//			u_int16_t baud;
			float baud;
			unsigned char rxinvert;
			unsigned char txinvert;
		} rpar;

		struct hfapp_msg_amtor_params {
			unsigned char destcall[4];
			unsigned char selfeccall[4];
			unsigned char mycall[4];
			u_int16_t txdelay;
			u_int16_t retry;
			unsigned char rxinvert;
			unsigned char txinvert;
		} apar;

		struct hfapp_msg_gtor_params {
			unsigned char destcall[10];
			unsigned char mycall[10];
			u_int16_t txdelay;
			u_int16_t retry;
		} gpar;

		struct hfapp_msg_pactor_params {
			unsigned char destcall[8];
			unsigned char mycall[8];
			u_int16_t txdelay;
			u_int16_t retry;
			unsigned char longpath;
			u_int16_t crcpreset[4];
		} ppar;

		struct hfapp_msg_mt63_params {
			u_int16_t bandwidth; 
			u_int16_t integration;
			unsigned char cwcall[10];
			u_int16_t doubleinterleave;
		} mt63par;

		struct hfapp_msg_mixer_params {
			int32_t src;
			int32_t igain;
			int32_t ogain;
		} mpar;
	} data;
};

/* --------------------------------------------------------------------- */

#define FSK 1
#define MT63 2

/* --------------------------------------------------------------------- */

#define HFAPP_MSG_DATA_TRANSMIT      1
#define HFAPP_MSG_DATA_TRANSMIT_BIN  2
#define HFAPP_MSG_DATA_SEND          3
#define HFAPP_MSG_DATA_RECEIVE       4
#define HFAPP_MSG_DATA_MONITOR       5
#define HFAPP_MSG_DATA_STATUS        6

#define HFAPP_MSG_DATA_SEND_EOF     20
#define HFAPP_MSG_SEND_TEST 	    25

#define HFAPP_MSG_SET_GENERALPAR    40

#define HFAPP_MSG_SET_CWPAR 	    50
#define HFAPP_MSG_MODE_CW_RX 	    51
#define HFAPP_MSG_MODE_CW_TX 	    52
#define HFAPP_MSG_CW_ELBUG_DIT 	    55
#define HFAPP_MSG_CW_ELBUG_DAH 	    56
#define HFAPP_MSG_CW_ELBUG_TUNE	    57
#define HFAPP_MSG_CW_ELBUG_PAUSE    58
#define HFAPP_MSG_STATE_CW_RX 	    60
#define HFAPP_MSG_STATE_CW_TX 	    61

#define HFAPP_MSG_REQ_SAMPLES      100
#define HFAPP_MSG_ACK_SAMPLES      101

#define HFAPP_MSG_SET_FSKPAR       150
#define HFAPP_MSG_SET_MIXERPAR     160

#define HFAPP_MSG_KEEPALIVE        200

#define HFAPP_MSG_MODE_QRT         300
#define HFAPP_MSG_MODE_IRS         301
#define HFAPP_MSG_MODE_ISS         302
#define HFAPP_MSG_MODE_SPEEDUP     303
#define HFAPP_MSG_STATE_QRT        310

#define HFAPP_MSG_CASE_UPPER       350
#define HFAPP_MSG_CASE_LOWER       351
#define HFAPP_MSG_CASE_FIGURE      352

#define HFAPP_MSG_START_STANDBY    	400
#define HFAPP_MSG_START_PACTOR_STANDBY  401

#define HFAPP_MSG_STATE_STANDBY    	450
#define HFAPP_MSG_STATE_PACTOR_STANDBY  451

#define HFAPP_MSG_START_PACTOR_ARQ 500
#define HFAPP_MSG_START_PACTOR_FEQ 501
#define HFAPP_MSG_SET_PACTORPAR    510

#define HFAPP_MSG_STATE_PACTOR_ARQ_MASTERCONNECT  550
#define HFAPP_MSG_STATE_PACTOR_ARQ_SLAVECONNECT   551
#define HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT     552
#define HFAPP_MSG_STATE_PACTOR_FEC_CONNECT        553
#define HFAPP_MSG_STATE_PACTOR_ISS                555
#define HFAPP_MSG_STATE_PACTOR_IRS                556
#define HFAPP_MSG_STATE_PACTOR_SPEED100           557
#define HFAPP_MSG_STATE_PACTOR_SPEED200           558

#define HFAPP_MSG_START_AMTOR_ARQ     600
#define HFAPP_MSG_START_AMTOR_COLFEQ  601
#define HFAPP_MSG_START_AMTOR_SELFEQ  602
#define HFAPP_MSG_SET_AMTORPAR        610

#define HFAPP_MSG_STATE_AMTOR_ARQ_MASTERCONNECT  650
#define HFAPP_MSG_STATE_AMTOR_ARQ_SLAVECONNECT   651
#define HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT     652
#define HFAPP_MSG_STATE_AMTOR_FEC_CONNECT        653
#define HFAPP_MSG_STATE_AMTOR_FEC_DISCONNECT     654
#define HFAPP_MSG_STATE_AMTOR_SELFEC_RX          655
#define HFAPP_MSG_STATE_AMTOR_COLFEC_RX          656
#define HFAPP_MSG_STATE_AMTOR_ISS                657
#define HFAPP_MSG_STATE_AMTOR_IRS                658

#define HFAPP_MSG_START_RTTY_TX       700
#define HFAPP_MSG_START_RTTY_RX       701
#define HFAPP_MSG_SET_RTTYPAR         710

#define HFAPP_MSG_STATE_RTTY_TX       750
#define HFAPP_MSG_STATE_RTTY_RX       751

#define HFAPP_MSG_START_GTOR_ARQ      800
#define HFAPP_MSG_SET_GTORPAR         810

#define HFAPP_MSG_STATE_GTOR_ARQ_MASTERCONNECT  850
#define HFAPP_MSG_STATE_GTOR_ARQ_SLAVECONNECT   851
#define HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT     852
#define HFAPP_MSG_STATE_GTOR_ISS                855
#define HFAPP_MSG_STATE_GTOR_IRS                856
#define HFAPP_MSG_STATE_GTOR_SPEED100           857
#define HFAPP_MSG_STATE_GTOR_SPEED200           858
#define HFAPP_MSG_STATE_GTOR_SPEED300           859

#define HFAPP_MSG_START_MT63_RX			900
#define HFAPP_MSG_START_MT63_TX			901
#define HFAPP_MSG_START_MT63_500_RX		902
#define HFAPP_MSG_START_MT63_500_TX		903
#define HFAPP_MSG_START_MT63_1000_RX		904
#define HFAPP_MSG_START_MT63_1000_TX		905
#define HFAPP_MSG_START_MT63_2000_RX		906
#define HFAPP_MSG_START_MT63_2000_TX		907

#define HFAPP_MSG_SET_MT63PAR		        910

#define HFAPP_MSG_STATE_MT63_RX			950
#define HFAPP_MSG_STATE_MT63_TX			951
#define HFAPP_MSG_STATE_MT63_500_RX		952
#define HFAPP_MSG_STATE_MT63_500_TX		953
#define HFAPP_MSG_STATE_MT63_1000_RX		954
#define HFAPP_MSG_STATE_MT63_1000_TX		955
#define HFAPP_MSG_STATE_MT63_2000_RX		956
#define HFAPP_MSG_STATE_MT63_2000_TX		957

/* --------------------------------------------------------------------- */

#define ERR_NOERR                 0
#define ERR_TOOBIG                1
#define ERR_RESOURCE              2
#define ERR_TIMEOUT               3

/* --------------------------------------------------------------------- */

#define SEV_FATAL    LOG_CRIT
#define SEV_ERROR    LOG_ERR
#define SEV_WARNING  LOG_WARNING
#define SEV_NOTICE   LOG_NOTICE
#define SEV_INFO     LOG_INFO


/* --------------------------------------------------------------------- */

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

/* --------------------------------------------------------------------- */

#define KBD_CHAR  0xff
#define KBD_BIN   0x100
#define KBD_EOF   0xffff

/* --------------------------------------------------------------------- */

extern void errprintf(int severity, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
extern void errstr(int severity, const char *st);

extern void start_l2_thread();
extern unsigned short kbd_get(void);
extern void kbd_ack(void);
extern void kbd_negack(void);
extern void kbd_back(void);
extern void kbd_clear_and_fill(const unsigned char *data, unsigned int length);
extern void bufprintf(unsigned int which, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
extern void bufwrite(unsigned int which, const unsigned char *data, unsigned int datalen);
extern void send_short_msg(u_int32_t type, u_int32_t err);
extern void send_msg(struct hfapp_msg *msg);

/* --------------------------------------------------------------------- */

#ifndef HAVE_VSNPRINTF

#include <stdarg.h>

extern int snprintf(char *buf, size_t len, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
extern int vsnprintf(char *buf, size_t len, const char *fmt, va_list ap);

#endif

/* --------------------------------------------------------------------- */

extern int modefamily;
extern int mt63_bandwidth;

/* --------------------------------------------------------------------- */

#endif /* _MSG_H */
