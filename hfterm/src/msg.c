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

#define MAILBOXPAUSE 1 // pause between mailbox rx and tx,
		      // the unit is Keepalive-message-intervals
int beacon_pause = 0;

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
	static int EOF_count = 0, beacon_wait_count = 0, mailbox_wait_count = 0;
	unsigned char obuf[256];
	
	
	switch(type) {
	case HFAPP_MSG_ACK_SAMPLES:
		if (err != ERR_NOERR) 
			errprintf(SEV_WARNING, "HFAPP_MSG_ACK_SAMPLES: error %d\n", err);
		else
			spec_samples((short *)msg->data.b, len / 2);
		return;

	case HFAPP_MSG_DATA_SEND:
		if (len <= 0) {
		    //display_status("send data len = %d, < 0", len);		    
		    return;
		}
//		test with: 
//		display_status("send data len len = %d", len);		    
//		showed: is always 1 in rtty
		write_output(msg->data.b, len);
//		}
		EOF_count = 0;
		return;
			
	case HFAPP_MSG_DATA_SEND_EOF:
/* 
 * The RTTY tx function (in hfkernel/fsk/rtty.c) waits 5 EOF`s 
 * (means: no data to transmit any more in buffer)
 * until it sends the EOF message.
 * So for RTTY the first EOF here would be o.k. to trigger autorx().
 * In Pactor, I think to have found out, there comes one EOF 
 * in the beginning before transmission,
 * so I let the EOF_count become 2 before switching to autorx().
 */
 		EOF_count++ ;
//		display_status("empty tx data block %d!", EOF_count);
//		printf("empty tx data block %d!\n", EOF_count);
		if (EOF_count >= 2) {   
//		    display_status("suppose: end of transmit data!");
		    EOF_count = 0;
		    if (autorx_on) {
		    	autorx();
		    }
		    if (beacon_on) {
		    	beacon_pause = 1;
		    	autorx();
		    }
		}
		return;
			 
	case HFAPP_MSG_SEND_TEST:
		display_status("TEST");
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
/* 
 *
 * here fallback autorx for "waiting and hanging qrt"
 *
 * here i also do the timing for the beacon pause 
 * and for the mailbox answering delay.
 * I want to be able to receive between beacon / mailbox transmissions.
 */		
            if (qrt_pending) {
		qrt_pending = 0;
		autorx();
		display_status
		    ("msg.c: qrt pending -> autorx."); 
	    }
	    
 	    if (beacon_on == 1 && beacon_pause == 1) {	
 		beacon_wait_count++ ;
		display_status("beacon countdown: %d", 
		    ((params.general.beaconpause + 5) / 5) - beacon_wait_count);
		if (beacon_wait_count >= ((params.general.beaconpause + 5) / 5)) {
//		    display_status
//		    ("beacon countdown from %d to 0, will transmit beacon.", 
//		    beacon_wait_count);
		    beacon_wait_count = 0;
		    beacon_pause = 0;
		    beacon_send();
		}
	    }
	    else beacon_wait_count = 0;

	    if (mailbox_on && mailbox_output_waiting) {
		mailbox_wait_count++ ;
//		display_status("mailbox pause countdown: %d", 
//			MAILBOXPAUSE - mailbox_wait_count);
		if (mailbox_wait_count >= MAILBOXPAUSE) {
//		    display_status
//		      	("msg.c: mailbox countdown from %d to 0, "
//			"will transmit mailbox output.", 
//		    	mailbox_wait_count);
		    mailbox_wait_count = 0;
		    mailbox_output_waiting = 0;
		    mailbox_output();
		    return;
		}
	    } 
	    else if ((mailbox_on) && (mailbox_connected)) {
//	    see if other side (f6fbb program) closed port
		mailbox_test_port();
	    }
	    return;
       
        case HFAPP_MSG_STATE_STANDBY:
                lastrxmsg = HFAPP_MSG_STATE_STANDBY;
		arq = 0;
                beacon_restore(); 
		way = RX;
		return;
     
        case HFAPP_MSG_STATE_PACTOR_STANDBY:
                lastrxmsg = HFAPP_MSG_STATE_PACTOR_STANDBY;
                beacon_restore(); 
		way = RX;
		arq = 0;
                return;
     
	case HFAPP_MSG_STATE_PACTOR_ARQ_MASTERCONNECT:
		beacon_suspend();
		arq = 1;
//		errprintf(SEV_INFO, "Pactor: ARQ master connect  Error: %d\n", err);
		lasttxmsg = HFAPP_MSG_STATE_PACTOR_ARQ_MASTERCONNECT;
		display_status("PACTOR ARQ MASTER CONNECT");
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_PACTOR_ARQ_SLAVECONNECT:
		beacon_suspend(); 
		arq = 1;
//		errprintf(SEV_INFO, "Pactor: ARQ slave connect  Error: %d\n", err);
		lastrxmsg = HFAPP_MSG_STATE_PACTOR_ARQ_SLAVECONNECT;
		display_status("PACTOR ARQ SLAVE CONNECT");
		way = RX;
		return;
		
	case HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT:
//		errprintf(SEV_INFO, "Pactor: ARQ disconnect  Error: %d\n", err);
		lasttxmsg = HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT;
		display_status("PACTOR ARQ DISCONNECT");
		arq = 0;
		if (mailbox_on) mailbox_loggedout();
		beacon_restore(); 
		way = TX; // if this is not so, autorx will not work
		autorx_on = 1;
		autorx();
		return;
		
	case HFAPP_MSG_STATE_PACTOR_FEC_CONNECT:
//		errprintf(SEV_INFO, "Pactor: FEC transmit  Error: %d\n", err);
		lasttxmsg =  HFAPP_MSG_STATE_PACTOR_FEC_CONNECT;
		/* bad name, just means, i am transmitting p-fec. */
		//display_status("PACTOR FEC TRANSMIT");
		arq = 0;
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_PACTOR_ISS:
		lasttxmsg = HFAPP_MSG_STATE_PACTOR_ISS;
//		errprintf(SEV_INFO, "Pactor: ISS  Error: %d\n", err);
//		display_status("kernel msg: %d", type);
		//display_status("PACTOR ISS");
		arq = 1;
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_PACTOR_IRS:
		lastrxmsg = HFAPP_MSG_STATE_PACTOR_IRS;
//		errprintf(SEV_INFO, "Pactor: IRS  Error: %d\n", err);
//		display_status("kernel msg: %d", type);
		//display_status("PACTOR IRS");
		arq = 1;
		way = RX;
		return;
		
	case HFAPP_MSG_STATE_PACTOR_SPEED100:
//		errprintf(SEV_INFO, "Pactor: Speed 100 Baud  Error: %d\n", err);
		arq = 1;
		display_status("PACTOR SPEED 100");
		return;
		
	case HFAPP_MSG_STATE_PACTOR_SPEED200:
//		errprintf(SEV_INFO, "Pactor: Speed 200 Baud  Error: %d\n", err);
		arq = 1;
		display_status("PACTOR SPEED 200");
		return;
		
	case HFAPP_MSG_STATE_AMTOR_ARQ_MASTERCONNECT:
		beacon_suspend();
		arq = 1;
		errprintf(SEV_INFO, "Amtor: ARQ Master connect  Error: %d\n", err);
		lasttxmsg = HFAPP_MSG_STATE_AMTOR_ARQ_MASTERCONNECT;
		display_status("AMTOR ARQ MASTER CONNECT");
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_AMTOR_ARQ_SLAVECONNECT:
		beacon_suspend();
		arq = 1;
//		errprintf(SEV_INFO, "Amtor: ARQ Slave connect  Error: %d\n", err);
		lastrxmsg = HFAPP_MSG_STATE_AMTOR_ARQ_SLAVECONNECT;
//		if (mailbox_on) mailbox_check();
		way = RX;
		display_status("AMTOR ARQ SLAVE CONNECT");
		return;
		
	case HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT:
//		errprintf(SEV_INFO, "Amtor: ARQ disconnected  Error: %d\n", err);
		arq = 0;
		lasttxmsg = HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT;
		display_status("AMTOR ARQ DISCONNECT");
		way = TX; // if this is not so, autorx will not work
		if (mailbox_on) mailbox_loggedout();
		beacon_restore(); 
		autorx_on = 1;
		autorx();
		return;
		
	case HFAPP_MSG_STATE_AMTOR_FEC_CONNECT:
//		errprintf(SEV_INFO, "Amtor: FEC transmit  Error: %d\n", err);
		arq = 0;
		lasttxmsg = HFAPP_MSG_STATE_AMTOR_FEC_CONNECT;
		//display_status("AMTOR FEC TRANSMIT");
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_AMTOR_SELFEC_RX:
//		errprintf(SEV_INFO, "Amtor: selective FEC receive  Error: %d\n", err);
		arq = 0;
		beacon_suspend(); 
		lastrxmsg = HFAPP_MSG_STATE_AMTOR_SELFEC_RX;
		//display_status("AMTOR FEC RECEIVE");
		way = RX;
		return;
		
	case HFAPP_MSG_STATE_AMTOR_COLFEC_RX:
//		errprintf(SEV_INFO, "Amtor: collective FEC receive  Error: %d\n", err);
		arq = 0;
		beacon_suspend(); 
		lastrxmsg = HFAPP_MSG_STATE_AMTOR_COLFEC_RX;
		//display_status("AMTOR COLFEC RECEIVE");
		way = RX;
		return;

	case HFAPP_MSG_STATE_AMTOR_FEC_DISCONNECT:
//		errprintf(SEV_INFO, "Amtor: FEC receive terminated  Error: %d\n", err);
		arq = 0;
		lasttxmsg = HFAPP_MSG_STATE_AMTOR_FEC_DISCONNECT;
		display_status("AMTOR FEC DISCONNECT");
		if (mailbox_on) mailbox_loggedout();
		beacon_restore(); 
		way = TX; // if this is not so, autorx will not work
		autorx_on = 1;
		autorx();
		return;
		
	case HFAPP_MSG_STATE_AMTOR_ISS:
//		errprintf(SEV_INFO, "Amtor: ISS  Error: %d\n", err);
		lasttxmsg = HFAPP_MSG_STATE_AMTOR_ISS;
		arq = 1;
		//display_status("AMTOR ISS");
//		display_status("kernel msg: %d", type);
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_AMTOR_IRS:
//		errprintf(SEV_INFO, "Amtor: IRS  Error: %d\n", err);
		lastrxmsg = HFAPP_MSG_STATE_AMTOR_IRS;
		arq = 1;
		//display_status("AMTOR IRS");
		way = RX;
		return;
		
	case HFAPP_MSG_STATE_GTOR_ARQ_MASTERCONNECT:
//		errprintf(SEV_INFO, "Gtor: ARQ master connect  Error: %d\n", err);
		beacon_suspend();
		arq = 1;
		lasttxmsg = HFAPP_MSG_STATE_GTOR_ARQ_MASTERCONNECT;
		display_status("GTOR ARQ MASTER CONNECT");
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_GTOR_ARQ_SLAVECONNECT:
		beacon_suspend();
		arq = 1;
//		errprintf(SEV_INFO, "Gtor: ARQ slave connect  Error: %d\n", err);
		lastrxmsg = HFAPP_MSG_STATE_GTOR_ARQ_SLAVECONNECT;
		display_status("GTOR ARQ SLAVE CONNECT");
		way = RX;
		return;
		
	case HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT:
//		errprintf(SEV_INFO, "Gtor: ARQ disconnect  Error: %d\n", err);
		arq = 0;
		lasttxmsg = HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT;
		display_status("GTOR ARQ DISCONNECT");
		if (mailbox_on) mailbox_loggedout();
		beacon_restore(); 
		way = TX; // if this is not so, autorx will not work
		autorx_on = 1;
		autorx();
		return;
		
	case HFAPP_MSG_STATE_GTOR_ISS:
//		errprintf(SEV_INFO, "Gtor: ISS  Error: %d\n", err);
		lasttxmsg = HFAPP_MSG_STATE_GTOR_ISS;
		arq = 1;
		//display_status("GTOR ISS");
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_GTOR_IRS:
//		errprintf(SEV_INFO, "Gtor: IRS  Error: %d\n", err);
		lastrxmsg = HFAPP_MSG_STATE_GTOR_IRS;
		arq = 1;
		//display_status("GTOR IRS");
		way = RX;
		return;
		
	case HFAPP_MSG_STATE_GTOR_SPEED100:
//		errprintf(SEV_INFO, "Gtor: Speed 100 Baud  Error: %d\n", err);
		display_status("GTOR SPEED 100");
		arq = 1;
		return;
		
	case HFAPP_MSG_STATE_GTOR_SPEED200:
//		errprintf(SEV_INFO, "Gtor: Speed 200 Baud  Error: %d\n", err);
		display_status("GTOR SPEED 200");
		arq = 1;
		return;
		
	case HFAPP_MSG_STATE_GTOR_SPEED300:
//		errprintf(SEV_INFO, "Gtor: Speed 300 Baud  Error: %d\n", err);
		display_status("GTOR SPEED 300");
		arq = 1;
		return;

	case HFAPP_MSG_STATE_CW_TX:
//		errprintf(SEV_INFO, "RTTY: Start Transmit  Error: %d\n", err);
		arq = 0;
		lasttxmsg = HFAPP_MSG_STATE_CW_TX;
		EOF_count = 0;
		qrt_pending = 0;
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_RTTY_TX:
//		errprintf(SEV_INFO, "RTTY: Start Transmit  Error: %d\n", err);
		arq = 0;
		lasttxmsg = HFAPP_MSG_STATE_RTTY_TX;
		EOF_count = 0;
		qrt_pending = 0;
		way = TX;
		return;
		
	case HFAPP_MSG_STATE_RTTY_RX:
//		errprintf(SEV_INFO, "RTTY: Start Receive  Error: %d\n", err);
		arq = 0;
		lastrxmsg = HFAPP_MSG_STATE_RTTY_RX;
		qrt_pending = 0;
		way = RX;
		return;

	case HFAPP_MSG_STATE_MT63_RX:
//		errprintf(SEV_INFO, "MT63: Start Receive  Error: %d\n", err);
		arq = 0;
		lastrxmsg = HFAPP_MSG_STATE_MT63_RX;
		qrt_pending = 0;
		way = RX;
		return;

	case HFAPP_MSG_STATE_MT63_TX:
//		errprintf(SEV_INFO, "MT63: Start Transmit  Error: %d\n", err);
		arq = 0;
		lasttxmsg = HFAPP_MSG_STATE_MT63_TX;
		qrt_pending = 0;
		way = TX;
		return;

    	default:
		errprintf(SEV_WARNING, "unknown message type: %u length: %u\n", type, len);
	}
}

/* --------------------------------------------------------------------------------- */

void msg_process(int fd)
{
	static unsigned char rxbuf[4096];
	static unsigned int rxbuf_size  = 0;
	struct hfapp_msg *msg;
	int i;
	unsigned char *bp;
	unsigned int truelen;
	static int read_check_count = 0;
	static int read_wait_count = 0;
	int read_count_max = 1000;
	
	i = read(fd, rxbuf+rxbuf_size, sizeof(rxbuf)-rxbuf_size);
	if (i < 0) {
		if (errno == EAGAIN) {
		    display_status("error in reading message from hfkernel. Trying again...");
		    read_check_count++;
		    if (read_check_count > read_count_max) {
			display_status
			    ("error count is %d in reading message from hfkernel.", read_check_count);
			errstr(SEV_FATAL, "read: EAGAIN");
	    	    }
		    return;
		}
		if (errno != EAGAIN) {
		    display_status("error in reading message from hfkernel. Giving up.");
			errstr(SEV_FATAL, "read: !=EAGAIN");
		    return;
		}
	}
	if (i == 0) {
		read_check_count++;
		if (read_check_count > read_count_max) {
		    display_status
		        ("reading message from hfkernel: %d zero-reads.  Will wait a bit.", read_check_count);
			usleep(30000);
			read_wait_count++;
			if (read_wait_count > 30) {
			    errstr(SEV_FATAL, "hfterm: Did hfkernel die?  zero-read loop");
			}
		}
		return;
	}
	
	read_check_count = 0;
	read_wait_count = 0;
	rxbuf_size += i;
	msg = (struct hfapp_msg *)(bp = rxbuf);
	while (rxbuf_size >= sizeof(msg->hdr)) {
		if (!HFAPP_MSG_ALIGNED(msg)) {
		    display_status("error in reading message from hfkernel: Buffer not aligned.");
		    errprintf(SEV_FATAL, "msg_process: buffer not aligned\n");
		}
		truelen = HFAPP_MSG_ALIGN(sizeof(msg->hdr)+ntohl(msg->hdr.len));
		if (truelen > rxbuf_size)
			break;
		process_msg(msg);
		rxbuf_size -= truelen;
		msg = (struct hfapp_msg *)(bp += truelen);
	}
	if (rxbuf_size >= sizeof(rxbuf)) {
	    display_status("error in reading message from hfkernel; Too big.");
	    errprintf(SEV_FATAL, "incoming message size too big\n");
	}
	memmove(rxbuf, bp, rxbuf_size);
}

/* --------------------------------------------------------------------- */



