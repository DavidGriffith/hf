/*****************************************************************************/

/*
 *      msg.c  --  HF Kernel message handling.
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
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hft.h"

#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
/*#include "byteorder.h"*/
#include <netinet/in.h>  /* ntoh? and hton? */

/* --------------------------------------------------------------------- */

void msg_send(struct hfapp_msg *msg)
{
	unsigned int len;
	unsigned char *bp = (unsigned char *)msg;

	len = ntohl(msg->hdr.len) + sizeof(msg->hdr);
	write_kernel(bp, HFAPP_MSG_ALIGN(len));
}

/* --------------------------------------------------------------------- */

static void process_msg(struct hfapp_msg *msg)
{
	unsigned int type = ntohl(msg->hdr.type);
	unsigned int len = ntohl(msg->hdr.len);
	unsigned int err = ntohl(msg->hdr.err);
	unsigned char obuf[256];
	
	switch(type) {
	case HFAPP_MSG_ACK_SAMPLES:
		if (err != ERR_NOERR) 
			errprintf(SEV_WARNING, "HFAPP_MSG_ACK_SAMPLES: error %d\n", err);
		else
			spec_samples((short *)msg->data.b, len / 2);
		return;

	case HFAPP_MSG_DATA_SEND:
		if (len <= 0)
			return;
		write_output(msg->data.b, len);
		return;
			
	case HFAPP_MSG_DATA_RECEIVE:
		if (len <= 0)
			return;
		write_input(msg->data.b, len);
		return;

	case HFAPP_MSG_DATA_MONITOR:
		if (len <= 0)
			return;
		write_monitor(msg->data.b, len);
		return;

	case HFAPP_MSG_DATA_STATUS:
		if (len <= 0)
			return;
		if (len >= sizeof(obuf))
			len = sizeof(obuf)-1;
		memcpy(obuf, msg->data.b, len);
		obuf[len] = 0;
		display_status(obuf);
		return;

	case HFAPP_MSG_KEEPALIVE:
		return;

	case HFAPP_MSG_STATE_PACTOR_ARQ_MASTERCONNECT:
		errprintf(SEV_INFO, "Pactor: ARQ master connect  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_PACTOR_ARQ_SLAVECONNECT:
		errprintf(SEV_INFO, "Pactor: ARQ slave connect  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT:
		errprintf(SEV_INFO, "Pactor: ARQ disconnect  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_PACTOR_FEC_CONNECT:
		errprintf(SEV_INFO, "Pactor: FEC transmit  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_PACTOR_ISS:
		errprintf(SEV_INFO, "Pactor: ISS  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_PACTOR_IRS:
		errprintf(SEV_INFO, "Pactor: IRS  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_PACTOR_SPEED100:
		errprintf(SEV_INFO, "Pactor: Speed 100 Baud  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_PACTOR_SPEED200:
		errprintf(SEV_INFO, "Pactor: Speed 200 Baud  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_AMTOR_ARQ_MASTERCONNECT:
		errprintf(SEV_INFO, "Amtor: ARQ Master connect  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_AMTOR_ARQ_SLAVECONNECT:
		errprintf(SEV_INFO, "Amtor: ARQ Slave connect  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT:
		errprintf(SEV_INFO, "Amtor: ARQ disconnected  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_AMTOR_FEC_CONNECT:
		errprintf(SEV_INFO, "Amtor: FEC transmit  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_AMTOR_SELFEC_RX:
		errprintf(SEV_INFO, "Amtor: selective FEC receive  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_AMTOR_COLFEC_RX:
		errprintf(SEV_INFO, "Amtor: collective FEC receive  Error: %d\n", err);
		return;

	case HFAPP_MSG_STATE_AMTOR_FEC_DISCONNECT:
		errprintf(SEV_INFO, "Amtor: FEC receive terminated  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_AMTOR_ISS:
		errprintf(SEV_INFO, "Amtor: ISS  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_AMTOR_IRS:
		errprintf(SEV_INFO, "Amtor: IRS  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_RTTY_TX:
		errprintf(SEV_INFO, "RTTY: Start Transmit  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_RTTY_RX:
		errprintf(SEV_INFO, "RTTY: Start Receive  Error: %d\n", err);
		return;

	case HFAPP_MSG_STATE_GTOR_ARQ_MASTERCONNECT:
		errprintf(SEV_INFO, "Gtor: ARQ master connect  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_GTOR_ARQ_SLAVECONNECT:
		errprintf(SEV_INFO, "Gtor: ARQ slave connect  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT:
		errprintf(SEV_INFO, "Gtor: ARQ disconnect  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_GTOR_ISS:
		errprintf(SEV_INFO, "Gtor: ISS  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_GTOR_IRS:
		errprintf(SEV_INFO, "Gtor: IRS  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_GTOR_SPEED100:
		errprintf(SEV_INFO, "Gtor: Speed 100 Baud  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_GTOR_SPEED200:
		errprintf(SEV_INFO, "Gtor: Speed 200 Baud  Error: %d\n", err);
		return;
		
	case HFAPP_MSG_STATE_GTOR_SPEED300:
		errprintf(SEV_INFO, "Gtor: Speed 300 Baud  Error: %d\n", err);
		return;
		
	default:
		errprintf(SEV_WARNING, "unknown message type: %u length: %u\n", type, len);
	}
}

/* --------------------------------------------------------------------- */

void msg_process(int fd)
{
	static unsigned char rxbuf[4096];
	static unsigned int rxbuf_size  = 0;
	struct hfapp_msg *msg;
	int i;
	unsigned char *bp;
	unsigned int truelen;

	i = read(fd, rxbuf+rxbuf_size, sizeof(rxbuf)-rxbuf_size);
	if (i < 0) {
		if (errno != EAGAIN)
			errstr(SEV_FATAL, "read");
		return;
	}
	rxbuf_size += i;
	msg = (struct hfapp_msg *)(bp = rxbuf);
	while (rxbuf_size >= sizeof(msg->hdr)) {
		if (!HFAPP_MSG_ALIGNED(msg))
			errprintf(SEV_FATAL, "msg_process: buffer not aligned\n");
		truelen = HFAPP_MSG_ALIGN(sizeof(msg->hdr)+ntohl(msg->hdr.len));
		if (truelen > rxbuf_size)
			break;
		process_msg(msg);
		rxbuf_size -= truelen;
		msg = (struct hfapp_msg *)(bp += truelen);
	}
	if (rxbuf_size >= sizeof(rxbuf))
		errprintf(SEV_FATAL, "incoming message size too big\n");
	memmove(rxbuf, bp, rxbuf_size);
}

/* --------------------------------------------------------------------- */



