/*****************************************************************************/

/*
 *      hfapp.h  --  HF kernel application interface.
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
 *  This is the Linux realtime sound output driver
 */

/*****************************************************************************/
      
#ifndef _HFAPP_H
#define _HFAPP_H

/* --------------------------------------------------------------------- */

#include <sys/types.h>

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
		
		struct hfapp_msg_fsk_params {
			u_int16_t freq[2];
		} fpar;

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

		struct hfapp_msg_amtor_params {
			unsigned char destcall[4];
			unsigned char selfeccall[4];
			unsigned char mycall[4];
			u_int16_t txdelay;
			u_int16_t retry;
			unsigned char rxinvert;
			unsigned char txinvert;
		} apar;

		struct hfapp_msg_rtty_params {
			u_int16_t baud;
			unsigned char rxinvert;
			unsigned char txinvert;
		} rpar;

		struct hfapp_msg_mixer_params {
			int32_t src;
			int32_t igain;
			int32_t ogain;
		} mpar;

	} data;
};

/* --------------------------------------------------------------------- */

#define HFAPP_MSG_DATA_TRANSMIT      1
#define HFAPP_MSG_DATA_TRANSMIT_BIN  2
#define HFAPP_MSG_DATA_SEND          3
#define HFAPP_MSG_DATA_RECEIVE       4
#define HFAPP_MSG_DATA_MONITOR       5
#define HFAPP_MSG_DATA_STATUS        6

#define HFAPP_MSG_REQ_SAMPLES      100
#define HFAPP_MSG_ACK_SAMPLES      101

#define HFAPP_MSG_SET_FSKPAR       150
#define HFAPP_MSG_SET_MIXERPAR     160

#define HFAPP_MSG_KEEPALIVE        200

#define HFAPP_MSG_MODE_QRT         300
#define HFAPP_MSG_MODE_IRS         301
#define HFAPP_MSG_MODE_ISS         302
#define HFAPP_MSG_MODE_SPEEDUP     303

#define HFAPP_MSG_CASE_UPPER       350
#define HFAPP_MSG_CASE_LOWER       351
#define HFAPP_MSG_CASE_FIGURE      352

#define HFAPP_MSG_START_STANDBY    400

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


/* --------------------------------------------------------------------- */

#define ERR_NOERR                 0
#define ERR_TOOBIG                1
#define ERR_RESOURCE              2
#define ERR_TIMEOUT               3

/* --------------------------------------------------------------------- */
#endif /* _HFAPP_H */
