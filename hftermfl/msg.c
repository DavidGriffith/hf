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

/*extern*/static __inline__ float hamming(float x)
{
        return 0.54-0.46*cos(2*M_PI*x);
}

/* ---------------------------------------------------------------------- */

/*
 * This fft routine is from ~gabriel/src/filters/fft/fft.c;
 * I am unsure of the original source.  The file contains no
 * copyright notice or description.
 * The declaration is changed to the prototype form but the
 * function body is unchanged.  (J. T. Buck)
 */

#define SWAP(a, b) tempr=(a); (a)=(b); (b)=tempr

/*
 * Replace data by its discrete Fourier transform, if isign is
 * input as 1, or by its inverse discrete Fourier transform, if 
 * "isign" is input as -1.  "data'"is a complex array of length "nn",
 * input as a real array data[0..2*nn-1]. "nn" MUST be an integer
 * power of 2 (this is not checked for!?)
 */

static void fft_rif(float *data, int nn, int isign) 
{
        int n;
        int mmax;
        int m, j, istep, i;
        float wtemp, wr, wpr, wpi, wi, theta;
        float tempr, tempi;

        data--;
        n = nn << 1;
        j = 1;

        for (i = 1; i < n; i += 2) {
                if(j > i) {
                        SWAP(data[j], data[i]);
                        SWAP(data[j+1], data[i+1]);
                }
                m= n >> 1;
                while (m >= 2 && j >m) {
                        j -= m;
                        m >>= 1;
                }
                j += m;
        }
        mmax = 2;
        while (n > mmax) {
                istep = 2*mmax;
                theta = -6.28318530717959/(isign*mmax);
                wtemp = sin(0.5*theta);
                wpr = -2.0*wtemp*wtemp;
                wpi = sin(theta);
                wr = 1.0;
                wi = 0.0;
                for (m = 1; m < mmax; m += 2) {
                        for (i = m; i < n; i += istep) {
                                j = i + mmax;
                                tempr = wr*data[j] - wi*data[j+1];
                                tempi = wr*data[j+1] + wi*data[j];
                                data[j] = data[i] - tempr;
                                data[j+1] = data[i+1] - tempi;
                                data[i] += tempr;
                                data[i+1] += tempi;
                        }
                        wr = (wtemp=wr)*wpr - wi*wpi+wr;
                        wi = wi*wpr + wtemp*wpi + wi;
                }
                mmax = istep;
        }
}
        
#undef SWAP

/* --------------------------------------------------------------------- */

/*extern*/static __inline__ float fsqr(float x) 
{ 
	return x*x;
}

/* --------------------------------------------------------------------- */

static void do_fftdisp(short *data)
{
	static int init = 0;
	static float window[2*SPEC_WIDTH];
	float fdata[2*2*SPEC_WIDTH];
	short s;
	int i;

#if 1
	if (!init) {
		init = 1;
		for (i = 0; i < 2*SPEC_WIDTH; i++)
			window[i] = 1.0/32767.0/(2*SPEC_WIDTH)*hamming(((float)i)/(2.0*SPEC_WIDTH-1.0));
	}
	for (i = 0; i < 2*SPEC_WIDTH; i++) {
		s = ntohs(data[i]);
		fdata[2*i] = s * window[i];
		fdata[2*i+1] = 0;
	}
	fft_rif(fdata, 2*SPEC_WIDTH, 1);
	for (i = 0; i < SPEC_WIDTH; i++)
		fdata[i] = 10.0 * log10(fsqr(fdata[2*i])+fsqr(fdata[2*i+1]));
#else
	for (i = 0; i < SPEC_WIDTH; i++) {
		s = ntohs(data[i]);		
		fdata[i] = -40*(1.0+s/32767.0);
	}
#endif
	scope_draw(fdata);
}

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
	struct hfapp_msg smsg;
	unsigned char obuf[256];
	unsigned char *iptr, *optr;
	
	switch(type) {
	case HFAPP_MSG_ACK_SAMPLES:
		if (!scope_on)
			return;
		if (err != ERR_NOERR) 
			errprintf(SEV_WARNING, "HFAPP_MSG_ACK_SAMPLES: error %d\n", err);
		else if (len < 2*2*SPEC_WIDTH)
			errprintf(SEV_WARNING, "HFAPP_MSG_ACK_SAMPLES: message too short\n");
		else 
			do_fftdisp((short *)msg->data.b);
		smsg.hdr.type = htonl(HFAPP_MSG_REQ_SAMPLES);
		smsg.hdr.len = htonl(sizeof(smsg.data.u));
		smsg.hdr.err = htonl(ERR_NOERR);
		smsg.data.u = htonl(SPEC_WIDTH*2);
		msg_send(&smsg);
		return;

	case HFAPP_MSG_DATA_SEND:
		if (len <= 0)
			return;
		optr = obuf + sprintf(obuf, "\033[32m");
		iptr = msg->data.b;
		for (; len > 0 && optr < obuf + sizeof(obuf) - 5; iptr++, len--) {
			if (*iptr == '\b') {
				*optr++ = '\b';
				*optr++ = ' ';
				*optr++ = '\b';
			} else if (*iptr == '\r') {
				*optr++ = '\r';
				*optr++ = '\n';
			} else if (*iptr != '\n')
				*optr++ = *iptr;
		}
		write_term(1, obuf, optr - obuf);
		return;
			
	case HFAPP_MSG_DATA_RECEIVE:
		if (len <= 0)
			return;
		optr = obuf + sprintf(obuf, "\033[30m");
		iptr = msg->data.b;
		for (; len > 0 && optr < obuf + sizeof(obuf) - 5; iptr++, len--) {
			if (*iptr == '\b') {
				*optr++ = '\b';
				*optr++ = ' ';
				*optr++ = '\b';
			} else if (*iptr == '\r') {
				*optr++ = '\r';
				*optr++ = '\n';
			} else if (*iptr != '\n')
				*optr++ = *iptr;
		}
		write_term(1, obuf, optr - obuf);
		return;

	case HFAPP_MSG_DATA_MONITOR:
		if (len <= 0)
			return;
		optr = obuf;
		iptr = msg->data.b;
		for (; len > 0 && optr < obuf + sizeof(obuf) - 5; iptr++, len--) {
			if (*iptr == '\n') {
				*optr++ = '\r';
				*optr++ = '\n';
			} else if (*iptr != '\r')
				*optr++ = *iptr;
		}
		write_term(2, obuf, optr - obuf);
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

void term_process(int trm, int fd)
{
	int i;
	struct hfapp_msg msg;
	unsigned char ibuf[64];
	unsigned char obuf[256];
	unsigned char *iptr = ibuf, *optr = obuf, *mptr = msg.data.b;
	
	i = read(fd, ibuf, sizeof(ibuf));
	if (i < 0) {
		if (errno != EAGAIN)
			errstr(SEV_FATAL, "read");
		return;
	}
	if (trm != 0)
		return;
	for (; i > 0 && optr < obuf + sizeof(obuf) - 8 && 
		     mptr < msg.data.b + sizeof(msg.data.b) - 8; i--, iptr++) {
		if (*iptr == '\b') {
			*mptr++ = '\b';
			*optr++ = '\b';
			*optr++ = ' ';
			*optr++ = '\b';
		} else if (*iptr == '\r') {
			*mptr++ = '\r';
			*mptr++ = '\n';
			*optr++ = '\r';
			*optr++ = '\n';
		} else if (*iptr != '\n') {
			*mptr++ = *iptr;
			*optr++ = *iptr;
		}
	}
	write_term(trm, obuf, optr - obuf);
	msg.hdr.type = htonl(HFAPP_MSG_DATA_TRANSMIT);
	msg.hdr.len = htonl(mptr - msg.data.b);
	msg.hdr.err = htonl(ERR_NOERR);
	msg_send(&msg);
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



