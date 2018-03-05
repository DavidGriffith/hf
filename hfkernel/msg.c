/*****************************************************************************/

/*
 *      msg.c  --  Linux message I/O. Not really linux specific.
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

#include <sys/types.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>  /* ntoh? and hton? */

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"
#include "standby.h"
#include "gtor.h"
#include "pactor.h"
#include "amtor.h"
#include "rtty.h"
#include "cw.h"
#include "mt63hflink.h"
/*#include "byteorder.h"*/

/* --------------------------------------------------------------------- */
/*
 * Buffering
 */
//static unsigned char txbuf[16384];
static unsigned char txbuf[32768];
static unsigned int txb_rd = 0, txb_wr = 0;
static pthread_mutex_t txb_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * "keyboard" data buffer
 */
#define KBDBUFSZ 1024
static unsigned short kbd_buf[KBDBUFSZ];
static pthread_mutex_t kbd_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned int kbd_wr = 0;
static unsigned int kbd_rd = 0;
static unsigned int kbd_rdahead = 0;

/*
 * app socket
 */
static int fd_app = -1;

/* --------------------------------------------------------------------- */

static struct {
	struct hfapp_msg_hdr hdr;
	short buf[2048];
} samples_buf;

/*
 *  to make mode group known
 *  to prevent hangs at switching from MT63 to FSK, I hope..
 */
int modefamily;
#define FSK 1
#define MT63 2
#define CW 3
int beaconpause = 5;

/* --------------------------------------------------------------------- */
/*
 * ================ Keyboard buffer functions =============================
 */

unsigned short kbd_get(void)
{
	unsigned short ret = KBD_EOF;

	if (pthread_mutex_lock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	if (kbd_rdahead != kbd_wr) {
		ret = kbd_buf[kbd_rdahead];
		kbd_rdahead = (kbd_rdahead+1) % KBDBUFSZ;
	}
	if (pthread_mutex_unlock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
	/*printf ("kbd_get: char %c is decimal %d and hex %x",
	    (char)ret, ret, ret);
	*/
	return ret;
}

void kbd_back(void)
{
	if (pthread_mutex_lock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	if (kbd_rdahead != kbd_rd)
		kbd_rdahead = (KBDBUFSZ-1+kbd_rdahead) % KBDBUFSZ;
	if (pthread_mutex_unlock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
}

void kbd_ack(void)
{
	unsigned char buf[KBDBUFSZ];
	int bc = 0;

	if (pthread_mutex_lock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	while (kbd_rd != kbd_rdahead) {
		buf[bc++] = kbd_buf[kbd_rd];
		kbd_rd = (kbd_rd + 1) % KBDBUFSZ;	
	}
	if (pthread_mutex_unlock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
	if (bc > 0)
		bufwrite(HFAPP_MSG_DATA_SEND, buf, bc);
	else	
		send_short_msg(HFAPP_MSG_DATA_SEND_EOF, ERR_NOERR);
}

void kbd_negack(void)
{
	if (pthread_mutex_lock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	kbd_rdahead = kbd_rd;
	if (pthread_mutex_unlock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
}

static void kbd_put(unsigned short ch)
{
	unsigned int i;

	if (ch == KBD_EOF)
		return;
	if (pthread_mutex_lock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	if ((ch & KBD_CHAR) == '\b' && !(ch & KBD_BIN)) {
		if (kbd_wr != kbd_rdahead && kbd_wr != kbd_rd) {
			i = (KBDBUFSZ-1+kbd_wr) % KBDBUFSZ;
			if ((ch & KBD_CHAR) != '\b' && !(ch & KBD_BIN)) {
				kbd_wr = i;
				if (pthread_mutex_unlock(&kbd_mutex))
					errstr(SEV_FATAL, "pthread_mutex_unlock");
				return;
			}
		}
	}
	i = (kbd_wr+1) % KBDBUFSZ;
	if (i != kbd_rd || i != kbd_rdahead) {
		kbd_buf[kbd_wr] = ch;
		kbd_wr = i;
	}		   
	if (pthread_mutex_unlock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
}

void kbd_clear_and_fill(const unsigned char *data, unsigned int length)
{
	if (pthread_mutex_lock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	if (length >= KBDBUFSZ)
		length = KBDBUFSZ-1;
	kbd_wr = kbd_rd = kbd_rdahead = 0;
	for (; length > 0; length--)
		kbd_buf[kbd_wr++] = *data++;
	if (pthread_mutex_unlock(&kbd_mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
}

/* --------------------------------------------------------------------- */
/*
 * ================ Buffering functions ===================================
 */

void send_msg(struct hfapp_msg *msg)
{
	unsigned int len, free, u;
	unsigned char *bp = (unsigned char *)msg;

	if (pthread_mutex_lock(&txb_mutex))
		errstr(SEV_FATAL, "pthread_mutex_lock");
	if (fd_app < 0) {
		txb_wr = txb_rd = 0;
		if (pthread_mutex_unlock(&txb_mutex))
			errstr(SEV_FATAL, "pthread_mutex_unlock");
		return;
	}
	len = HFAPP_MSG_ALIGN(ntohl(msg->hdr.len) + sizeof(msg->hdr));
	free = (sizeof(txbuf) - 1 - txb_wr + txb_rd) % sizeof(txbuf);
	if (len > free) {
		if (pthread_mutex_unlock(&txb_mutex))
			errstr(SEV_FATAL, "pthread_mutex_unlock");
		errprintf(SEV_FATAL, "message buffer overflow, msg len %u\n", len);
	}
	u = sizeof(txbuf)-txb_wr;
	if (len >= u) {
		memcpy(txbuf+txb_wr, bp, u);
		txb_wr = 0;
		bp += u;
		len -= u;
	}
	if (len > 0) {
		memcpy(txbuf+txb_wr, bp, len);
		txb_wr += len;
	}
	if (pthread_mutex_unlock(&txb_mutex))
		errstr(SEV_FATAL, "pthread_mutex_unlock");
}

/* --------------------------------------------------------------------- */

void send_short_msg(u_int32_t type, u_int32_t err)
{
	struct hfapp_msg_hdr msg_hdr;
				
	msg_hdr.type = htonl(type);
	msg_hdr.len = htonl(0);
	msg_hdr.err = htonl(err);
	send_msg((struct hfapp_msg *)&msg_hdr);
}

/* --------------------------------------------------------------------- */

void bufwrite(unsigned int which, const unsigned char *data, unsigned int datalen)
{
	struct hfapp_msg msg;
	unsigned int i;

	msg.hdr.type = htonl(which);
	msg.hdr.err = ERR_NOERR;
	while (datalen > 0) {
		i = datalen;
		if (i > sizeof(msg.data.b))
			i = sizeof(msg.data.b);
		memcpy(msg.data.b, data, i);
		data += i;
		datalen -= i;
		msg.hdr.len = htonl(i);
		send_msg(&msg);
	}
}

/* --------------------------------------------------------------------- */

void bufprintf(unsigned int which, const char *fmt, ...) 
{
	va_list args;
	struct hfapp_msg msg;
	int i;

	va_start(args, fmt);
	msg.hdr.type = htonl(which);
	i = vsnprintf(msg.data.b, sizeof(msg.data.b), fmt, args);
	if (i < 0)
		errstr(SEV_FATAL, "vsnprintf");
	msg.hdr.len = htonl(i);
	msg.hdr.err = ERR_NOERR;
	send_msg(&msg);
//	debug only, prints lot of mess...
//	printf("%s", fmt);
}

/* --------------------------------------------------------------------- */

/* compiler warn: "static decl follows non-static" */
/*static*/ 
//modi
void start_l2_thread(void *(*routine)(void *))
{
#ifndef NOREALTIME
	static struct sched_param schp;
#endif /* NOREALTIME */
	pthread_attr_t thr_attr;
	static int crashblock = 0;
	int res = 0;
	
//	printf("start_l2_thread: modefamily is %d\n", modefamily);

	if (pthread_attr_init(&thr_attr))
		errstr(SEV_FATAL, "pthread_attr_init");

	if (crashblock) {
	    bufprintf(HFAPP_MSG_DATA_STATUS, 
		"start of new mode in progress.\n"
		"-> no mode-changing commands accepted for now.\n");
	    return;
	}
	crashblock = 1;
	
	res =(pthread_cancel(thr_l2));
	//res =(pthread_kill(thr_l2, NULL));
	//kill does not work
	
	if (res){
	    if (res == ESRCH) {
	    	errstr(SEV_INFO, "pthread_cancel: no l2 thread was running");
		res = 0;
	    } else {
		errstr(SEV_WARNING, "pthread_cancel l2 thread: unknown error");
	    }
	}
	
	if (!res) {
	    if (pthread_join(thr_l2, NULL))
		errstr(SEV_WARNING, "pthread_join l2 thread");
	}
	
	// switch fsk sound loop on / off 
	
	if ((fsk_l1_loop_running) && (modefamily == FSK)) {
//	    printf("mode family remains FSK, so FSK sound loop keeps running...\n");
	}
	
	if ((! fsk_l1_loop_running) && (modefamily == FSK)) {
	    l1_init();
//	    printf("mode family changed to FSK, I started l1 FSK sound loop.\n");
	}
	
	if ( ( fsk_l1_loop_running) && (modefamily != FSK)) {
	    l1_stop();
//	    printf("mode family changed, I stopped l1 FSK sound loop.\n");
	}
	if ( (! fsk_l1_loop_running) && (modefamily != FSK)) {
//	    printf("mode family remains other than FSK.\n");
	}

#ifndef NOREALTIME
 	memset(&schp, 0, sizeof(schp));
	if (pthread_attr_setschedpolicy(&thr_attr, SCHED_RR))
		errstr(SEV_FATAL, "pthread_attr_setschedpolicy");
	schp.sched_priority = sched_get_priority_min(SCHED_RR)+1;
	if (pthread_attr_setschedparam(&thr_attr, &schp))
		errstr(SEV_FATAL, "pthread_attr_setschedparam");
#endif /* NOREALTIME */
        
	if (pthread_create(&thr_l2, &thr_attr, routine, NULL))
		errstr(SEV_FATAL, "pthread_create");
	
	if (pthread_attr_destroy(&thr_attr))
		errstr(SEV_WARNING, "pthread_attr_destroy");
	
	crashblock = 0;

}

// Tom's original
static void ori_start_l2_thread(void *(*routine)(void *))
{
#ifndef NOREALTIME
	static struct sched_param schp;
#endif /* NOREALTIME */
	pthread_attr_t thr_attr;

	if (pthread_cancel(thr_l2))
		errstr(SEV_FATAL, "pthread_cancel");
	if (pthread_join(thr_l2, NULL))
		errstr(SEV_FATAL, "pthread_join");
	if (pthread_attr_init(&thr_attr))
		errstr(SEV_FATAL, "pthread_attr_init");
#ifndef NOREALTIME
 	memset(&schp, 0, sizeof(schp));
	if (pthread_attr_setschedpolicy(&thr_attr, SCHED_RR))
		errstr(SEV_FATAL, "pthread_attr_setschedpolicy");
	schp.sched_priority = sched_get_priority_min(SCHED_RR)+1;
	if (pthread_attr_setschedparam(&thr_attr, &schp))
		errstr(SEV_FATAL, "pthread_attr_setschedparam");
#endif /* NOREALTIME */
	if (pthread_create(&thr_l2, &thr_attr, routine, NULL))
		errstr(SEV_FATAL, "pthread_create");
	if (pthread_attr_destroy(&thr_attr))
		errstr(SEV_WARNING, "pthread_attr_destroy");
}

/* --------------------------------------------------------------------- */

/* --------------------------------------------------------------------- */

static void process_msg(struct hfapp_msg *msg)
{
	unsigned int type = ntohl(msg->hdr.type);
	unsigned int len = ntohl(msg->hdr.len);
	/*unsigned int err = ntohl(msg->hdr.err);*/
	unsigned char *bp;
	unsigned int u;

	switch(type) {

	case HFAPP_MSG_SET_GENERALPAR:
		if (len < sizeof(msg->data.generalpar))
			return;
		beaconpause = msg->data.generalpar.beaconpause; 
		return;

	case HFAPP_MSG_REQ_SAMPLES:
		if (len < sizeof(msg->data.u))
			return;
		u = ntohl(msg->data.u);
		//printf("hfkernel: sample request for spectrum by hfterm.\n");

		if (u >= sizeof(samples_buf.buf)/sizeof(samples_buf.buf[0])) {
			samples_buf.hdr.type = htonl(HFAPP_MSG_ACK_SAMPLES);
			samples_buf.hdr.len = htonl(0);
			samples_buf.hdr.err = htonl(ERR_TOOBIG);
			send_msg((struct hfapp_msg *)&samples_buf);
			return;
		}
#if 0
		if (samples_count) {
			samples_buf.hdr.type = htonl(HFAPP_MSG_ACK_SAMPLES);
			samples_buf.hdr.len = htonl(0);
			samples_buf.hdr.err = htonl(ERR_RESOURCE);
			send_msg((struct hfapp_msg *)&samples_buf);
			return;
		}
#endif
		l1_start_sample(samples_buf.buf, u);
		samples_buf.hdr.type = htonl(HFAPP_MSG_ACK_SAMPLES);
		samples_buf.hdr.len = htonl(u*2);
		samples_buf.hdr.err = htonl(ERR_NOERR);
		return;

	case HFAPP_MSG_DATA_TRANSMIT:
		for (bp = msg->data.b; len > 0; len--, bp++)
			kbd_put((*bp) & KBD_CHAR);
		return;
		
	case HFAPP_MSG_DATA_TRANSMIT_BIN:  
		for (bp = msg->data.b; len > 0; len--, bp++)
			kbd_put(((*bp) & KBD_CHAR) | KBD_BIN);
		return;

	case HFAPP_MSG_SET_CWPAR:
		if (len < sizeof(msg->data.cwpar))
			return;
		cw_set_params(
		    ntohs(msg->data.cwpar.wpm), 
		    ntohs(msg->data.cwpar.tone),
		    msg->data.cwpar.farnsworth,
		    msg->data.cwpar.dtr);
		return;

	case HFAPP_MSG_MODE_CW_RX:
		modefamily = CW;
//		start_l2_thread(mode_cw_rx);
		return;	
		
	case HFAPP_MSG_MODE_CW_TX:
		modefamily = CW;
		start_l2_thread(mode_cw_tx);
		return;	
		
	case HFAPP_MSG_CW_ELBUG_DIT:
		lastcwmsg = type;
		return;
		
	case HFAPP_MSG_CW_ELBUG_DAH:
		lastcwmsg = type;
		return;

	case HFAPP_MSG_CW_ELBUG_TUNE:
		lastcwmsg = type;
		return;

	case HFAPP_MSG_CW_ELBUG_PAUSE:
		lastcwmsg = type;
		return;

	case HFAPP_MSG_START_STANDBY:
		modefamily = FSK;
		start_l2_thread(mode_standby);
		return;

	case HFAPP_MSG_START_PACTOR_STANDBY:
		modefamily = FSK;
		start_l2_thread(mode_pactor_standby);
		return;

	case HFAPP_MSG_START_PACTOR_ARQ:
		modefamily = FSK;
		start_l2_thread(mode_pactor_arq);
		return;

	case HFAPP_MSG_START_PACTOR_FEQ:
		modefamily = FSK;
		start_l2_thread(mode_pactor_fec);
		return;

	case HFAPP_MSG_SET_PACTORPAR:
		if (len < sizeof(msg->data.ppar))
			return;
		pactor_set_params(msg->data.ppar.destcall, 
		    msg->data.ppar.mycall,
		    ntohs(msg->data.ppar.txdelay), 
		    ntohs(msg->data.ppar.retry),
		    msg->data.ppar.longpath, 
		    ntohs(msg->data.ppar.crcpreset[0]), 
		    ntohs(msg->data.ppar.crcpreset[1]), 
		    ntohs(msg->data.ppar.crcpreset[2]), 
		    ntohs(msg->data.ppar.crcpreset[3]));
		return;

	case HFAPP_MSG_START_GTOR_ARQ:
		modefamily = FSK;
		start_l2_thread(mode_gtor_arq);
		return;

	case HFAPP_MSG_SET_GTORPAR:
		if (len < sizeof(msg->data.gpar))
			return;
		gtor_set_params(msg->data.gpar.destcall, 
		    msg->data.gpar.mycall,
		    ntohs(msg->data.gpar.txdelay), 
		    ntohs(msg->data.gpar.retry));
		return;

	case HFAPP_MSG_KEEPALIVE:
		return;

	case HFAPP_MSG_MODE_QRT:   
		gtor_mode_qrt();
		pactor_mode_qrt();
		amtor_mode_qrt();
		return;

	case HFAPP_MSG_MODE_IRS:
		gtor_mode_irs();
		pactor_mode_irs();
		amtor_mode_irs();
		return;

	case HFAPP_MSG_MODE_ISS:
		gtor_mode_iss();
		pactor_mode_iss();
		amtor_mode_iss();
		return;

	case HFAPP_MSG_MODE_SPEEDUP:
		gtor_mode_speedup();
		pactor_mode_speedup();
		return;

	case HFAPP_MSG_START_AMTOR_ARQ:
		modefamily = FSK;
		start_l2_thread(mode_amtor_arq);
		return;

	case HFAPP_MSG_START_AMTOR_COLFEQ:
		modefamily = FSK;
		start_l2_thread(mode_amtor_colfec);
		return;

	case HFAPP_MSG_START_AMTOR_SELFEQ:
		modefamily = FSK;
		start_l2_thread(mode_amtor_selfec);
		return;

	case HFAPP_MSG_SET_AMTORPAR:
		if (len < sizeof(msg->data.apar))
			return;
		amtor_set_params(msg->data.apar.destcall, 
		    msg->data.apar.selfeccall, 
		    msg->data.apar.mycall,
		    htons(msg->data.apar.txdelay), 
		    htons(msg->data.apar.retry), 
		    msg->data.apar.rxinvert, 
		    msg->data.apar.txinvert);
		return;

	case HFAPP_MSG_START_RTTY_TX:
		modefamily = FSK;
		start_l2_thread(mode_rtty_tx);
		return;

	case HFAPP_MSG_START_RTTY_RX:
		modefamily = FSK;
		start_l2_thread(mode_rtty_rx);
		return;

	case HFAPP_MSG_SET_RTTYPAR:
		if (len < sizeof(msg->data.rpar))
			return;
//		rtty_set_params(htons(msg->data.rpar.baud), 
		rtty_set_params (msg->data.rpar.baud, 
		    msg->data.rpar.rxinvert, 
		    msg->data.rpar.txinvert);
		return;

	case HFAPP_MSG_CASE_UPPER:
		amtor_reset_uppercase();
		rtty_reset_uppercase();
		return;

	case HFAPP_MSG_CASE_LOWER:
		amtor_reset_lowercase();
		rtty_reset_uppercase();
		return;

	case HFAPP_MSG_CASE_FIGURE:
		amtor_reset_figurecase();
		rtty_reset_figurecase();
		return;

	case HFAPP_MSG_SET_FSKPAR:
		if (len < sizeof(msg->data.fpar))
			return;
		l1_set_fsk_params(
		    htons(msg->data.fpar.freq[0]), 
		    htons(msg->data.fpar.freq[1]));
		return;
		
	case HFAPP_MSG_SET_MIXERPAR:
		if (len < sizeof(msg->data.mpar))
			return;
		l1_set_mixer(
		    htonl(msg->data.mpar.src), 
		    htonl(msg->data.mpar.igain), 
		    htonl(msg->data.mpar.ogain));
		return;
		
	case HFAPP_MSG_START_MT63_TX:
		modefamily = MT63;
		start_l2_thread(mode_mt63_tx);
		return;

	case HFAPP_MSG_START_MT63_RX:
		modefamily = MT63;
		start_l2_thread(mode_mt63_rx);
		return;

	case HFAPP_MSG_SET_MT63PAR:
		if (len < sizeof(msg->data.mt63par))
			return;
		mt63_set_params(
		    htons(msg->data.mt63par.bandwidth), 
		    htons(msg->data.mt63par.integration), 
		    msg->data.mt63par.cwcall,
		    msg->data.mt63par.doubleinterleave);
		return;
	default:
		errprintf(SEV_WARNING, "unknown message type: %u length: %u\n",
			  type, len);
	}
}

/* --------------------------------------------------------------------- */

void *io_process(void *dummy)
{
	fd_set wmask, rmask, emask;
	int i;
	unsigned char rxbuf[4096];
	unsigned int rxbuf_size  = 0, truesize;
	int maxfd;
	int sock;
	struct hfapp_msg *msg;
	struct sockaddr_un saddr;
	struct timeval tm;
	unsigned char *bp;
	time_t keepalive = 0, newt;

	if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		errstr(SEV_FATAL, "socket");
	saddr.sun_family = AF_UNIX;
	strncpy(saddr.sun_path, name_comm, sizeof(saddr.sun_path));
	unlink(saddr.sun_path);
	if (bind(sock, (struct sockaddr *)&saddr, sizeof(saddr)))
		errstr(SEV_FATAL, "bind");
	if (listen(sock, 1))
		errstr(SEV_FATAL, "listen");
	chmod(saddr.sun_path, comm_perm);
	/*fchmod(sock, comm_perm);*/
	for (;;) {
		FD_ZERO(&rmask);
		FD_ZERO(&wmask);
		FD_ZERO(&emask);
		if (fd_app >= 0) {
			FD_SET(fd_app, &rmask);
			FD_SET(fd_app, &emask);
			if (pthread_mutex_lock(&txb_mutex))
				errstr(SEV_FATAL, "pthread_mutex_lock");
			if (txb_rd != txb_wr)
				FD_SET(fd_app, &wmask);
			if (pthread_mutex_unlock(&txb_mutex))
				errstr(SEV_FATAL, "pthread_mutex_unlock");
			maxfd = fd_app+1;
		} else {
			FD_SET(sock, &rmask);
			maxfd = sock+1;
		}
		tm.tv_sec = 0;
		tm.tv_usec = 100000;
		i = select(maxfd, &rmask, &wmask, &emask, &tm);
		if (i < 0)
			errstr(SEV_FATAL, "select");
		if (FD_ISSET(sock, &rmask)) {
			i = sizeof(saddr);
			if ((fd_app = accept(sock, (struct sockaddr *)(&saddr), &i)) < 0)
				errstr(SEV_FATAL, "accept");
			errprintf(SEV_INFO, "connect from %s fam %d\n", 
				  saddr.sun_path, saddr.sun_family);
			if (pthread_mutex_lock(&txb_mutex))
				errstr(SEV_FATAL, "pthread_mutex_lock");
			txb_rd = txb_wr = 0;
			if (pthread_mutex_unlock(&txb_mutex))
				errstr(SEV_FATAL, "pthread_mutex_unlock");
			rxbuf_size  = 0;
			continue;
		}
		if (fd_app < 0) 
			continue;
		if (FD_ISSET(fd_app, &emask)) {
			errprintf(SEV_INFO, "connection closed\n");
			close(fd_app);
			fd_app = -1;
			continue;
		}
		if (FD_ISSET(fd_app, &wmask)) {
			if (pthread_mutex_lock(&txb_mutex))
				errstr(SEV_FATAL, "pthread_mutex_lock");
			if (txb_rd <= txb_wr) 
				i = write(fd_app, txbuf+txb_rd, txb_wr-txb_rd);
			else 
				i = write(fd_app, txbuf+txb_rd, sizeof(txbuf)-txb_rd);
			if (i < 0) {
				if (errno == EPIPE) {
					if (pthread_mutex_unlock(&txb_mutex))
						errstr(SEV_FATAL, "pthread_mutex_unlock");
					errprintf(SEV_INFO, "connection closed (EPIPE)\n");
					close(fd_app);
					fd_app = -1;
					continue;
				}
				if (errno != EAGAIN)
					errstr(SEV_FATAL, "write");
			} else {
				txb_rd = (txb_rd + i) % sizeof(txbuf);
			}
			if (pthread_mutex_unlock(&txb_mutex))
				errstr(SEV_FATAL, "pthread_mutex_unlock");
		}
		if (FD_ISSET(fd_app, &rmask)) {
			i = read(fd_app, rxbuf+rxbuf_size, sizeof(rxbuf)-rxbuf_size);
			if (i < 0) {
				if (errno == EPIPE) {
					errprintf(SEV_INFO, "connection closed (EPIPE on read)\n");
					close(fd_app);
					fd_app = -1;
					continue;
				}
				if (errno != EAGAIN)
					errstr(SEV_FATAL, "read");
				continue;
			}
			rxbuf_size += i;
			msg = (struct hfapp_msg *)(bp = rxbuf);
			while (rxbuf_size >= sizeof(msg->hdr)) {
				if (!HFAPP_MSG_ALIGNED(msg))
					errprintf(SEV_FATAL, "io_process: buffer not aligned\n");
				truesize = HFAPP_MSG_ALIGN(sizeof(msg->hdr)+ntohl(msg->hdr.len));
				if (truesize > rxbuf_size)
					break;
				process_msg(msg);
				rxbuf_size -= truesize;
				msg = (struct hfapp_msg *)(bp += truesize);
			}
			if (rxbuf_size >= sizeof(rxbuf))
				errprintf(SEV_FATAL, "incoming message size too big\n");
			memmove(rxbuf, bp, rxbuf_size);
		}
		/*
		 * do periodic checks
		 */
		if (l1_sample_finished()) { 
			for (i = ntohl(samples_buf.hdr.len)/2-1; i >= 0; i--)
				samples_buf.buf[i] = htons(samples_buf.buf[i]);
			send_msg((struct hfapp_msg *)&samples_buf);
		}
		/* send keepalive message to socket */
		if (fd_app >= 0) {
			if ((unsigned)(time(&newt) - keepalive) >= 5) {
				send_short_msg(HFAPP_MSG_KEEPALIVE, ERR_NOERR);
				keepalive = newt;
			}
		}
	}
}

/* --------------------------------------------------------------------- */
