/*****************************************************************************/
/*
 *      hft.h  --  Simple HF terminal. Main header file. 
 *
 *      Copyright (C) 1996  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *        Swiss Federal Institute of Technology (ETH), Electronics Lab
 *
 *	modified 2001-2 by Axel Krystof DF3JRK
 * 	modified 2003 by Gnther Montag DL4MGE
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
      
#ifndef _HFT_H
#define _HFT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslog.h>
#include <errno.h>

#include "fskl1.h"
#include "msg.h"
#include "callbacks.h"
#include "gui.h"
#include "support.h"
#include "spectrum.h"
#include "log.h"
#include "rxtx.h"
#include "mailbox.h"

/* --------------------------------------------------------------------- */

#ifndef HAVE_VSNPRINTF
#include <stdarg.h>
extern int snprintf(char *buf, size_t len, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
extern int vsnprintf(char *buf, size_t len, const char *fmt, va_list ap);
#endif

/* --------------------------------------------------------------------- */

#define SEV_FATAL    LOG_CRIT
#define SEV_ERROR    LOG_ERR
#define SEV_WARNING  LOG_WARNING
#define SEV_NOTICE   LOG_NOTICE
#define SEV_INFO     LOG_INFO

extern void errprintf(int severity, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
extern void errstr(int severity, const char *st);

/* --------------------------------------------------------------------- */

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

/* --------------------------------------------------------------------- */

#define DBSPAN 80
#define SRATE 8000
#define MAXMON 800

/* --------------------------------------------------------------------- */

#ifndef DEBUG
/*DEBUG*/
#define DEBUG printf("%s: function %s still running at line %d...\n", \
__FILE__, __FUNCTION__,  __LINE__);

#define D DEBUG
#endif /* DEBUG */

/* --------------------------------------------------------------------- */

//static 
GPollFD msgpfd;

/* Funktionen aus main.c -------------------------------------------- */

void textinsert(char *filename, GtkText *text);
extern void write_kernel(unsigned char *data, int datalen);
extern void msg_process(int fd);
extern void term_process(int trm, int fd);
extern void msg_send(struct hfapp_msg *msg);
extern void write_input(unsigned char *data, int datalen);
extern void write_output(unsigned char *data, int datalen);
void start_write_mailboxtest();
extern void write_monitor(unsigned char *data, int datalen);
void edit_newline(void);
void edit_backspace(void);
void edit_addchar(char v);
//gboolean poll_prepare(gpointer source_data, GTimeVal *current_time, gint *timeout, gpointer user_data)static gboolean poll_prepare(gpointer source_data, GTimeVal *current_time, gint *timeout, gpointer user_data)static gboolean poll_prepare(gpointer source_data, GTimeVal *current_time, gint *timeout, gpointer user_data);
//gboolean poll_check(gpointer source_data, GTimeVal *current_time, gpointer user_data);
void errprintf(int severity, const char *fmt, ...);
void errstr(int severity, const char *st);
extern void display_status(const char *fmt, ...);
//int cb_canvas(FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d);
void param_set_defaults(void);
void param_get(void);
void param_set(void);
void param_kernel(void);
void param_read();
void param_store(void);
void timequery();

void init(void);
void finit(void);

/* aus maingui.c ------------------------------------------------------ */
GtkWidget *wrxfileselection;
GtkWidget *wspec;
GtkWidget *wpar;
GtkWidget *wmain;
GtkWidget *wabout;
GtkWidget *whilfe; 		/* HILFEDATEI */
GtkWidget *Wfixtext;		/* FIXTEXTE */
GtkWidget *Wsearchlogentr;	/* LOGBUCH DURCHSUCHEN */
GtkWidget *wlistalllog; 	/* LISTE ALLE LOGBUCH EINTRAEGE */
GtkWidget *wqsoeditor; 		/* ZEIGE LOGBUCH EINTRAEGE */
GtkWidget *whinweis; 		/* HINWEISFENSTER */
GtkWidget *wmap; 		/* WELTKARTE */
GtkWidget *wmonitor;

struct par {
	struct {
		unsigned char call[16];
		unsigned char op[128];
	        unsigned char qth[128];
		unsigned char loc[128];
		unsigned char rig[128];
		unsigned char pwr[128];
		unsigned char ant[128];
		unsigned char www[128];
	} brag;

	struct {
		float snd_corr;
		float tv_corr;
		float cpu_mhz;
		unsigned char soundcard[16];
		unsigned char serial[16];
		unsigned char kerneloptions[256];
		unsigned int beaconpause;
		unsigned int squelchpercent;
	} general;

	struct {
		unsigned int freq[2];
	} fsk;

	struct {
		unsigned int wpm;
		unsigned int tone;
		unsigned int farnsworth;
		unsigned int dtr;
	} cw;

	struct {
		float baud;
		unsigned int rxinvert;
		unsigned int txinvert;
	} rtty;

	struct {
		unsigned char destcall[5];
		unsigned char selfeccall[5];
		unsigned char mycall[5];
		unsigned int txdelay;
		unsigned int retry;
		unsigned int rxinvert;
		unsigned int txinvert;
	} amtor;

	struct {
		unsigned char destcall[10];
		unsigned char mycall[10];
		unsigned int txdelay;
		unsigned int retry;
	} gtor;

	struct {
		unsigned char destcall[8];
		unsigned char mycall[8];
		unsigned int txdelay;
		unsigned int retry;
		unsigned char longpath;
		unsigned int crcpreset[4];
	} pactor;

	struct {
	        unsigned int bandwidth;
	        unsigned int integration;
	        unsigned char cwcall[10];
	        unsigned int doubleinterleave;
	} mt63;
	
	struct {
		char host[32];
		unsigned int port;
	} mailbox;

};
extern struct par params;

extern const char* fixmagic;
extern int scope_on, mon_on, log_on, params_on, brag_on, fix_on;
extern int rxfile_ready;
extern char gmt[32];
extern GdkFont *radiofont;

#endif /* _HFT_H */
