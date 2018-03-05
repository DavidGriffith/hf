/*****************************************************************************/

/*
 *      main.c  --  simple HF terminal program.
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
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <ctype.h>

#include "hfapp.h"
#include "hft.h"

#include "interface.h"
#include "support.h"
#include "spectrum.h"

/* --------------------------------------------------------------------- */

#ifndef FREQ_SPACE
#define FREQ_SPACE 1275
#endif

#ifndef FREQ_MARK
#define FREQ_MARK  1475
#endif

#ifndef MYCALL
#define MYCALL ""
#endif

#ifndef PCT_CRC_0
#define PCT_CRC_0 "FFFF"
#endif

#ifndef PCT_CRC_1
#define PCT_CRC_1 "FFFF"
#endif

#ifndef PCT_CRC_2
#define PCT_CRC_2 "FFFF"
#endif

#ifndef PCT_CRC_3
#define PCT_CRC_3 "FFFF"
#endif

/* --------------------------------------------------------------------- */

static GtkWidget *wspec;
static GtkWidget *wpar;
static GtkWidget *wmain;
static GtkWidget *wabout;
static GtkWidget *wmonitor;

int scope_on = 0;
static int fd_krnl = -1;

static char *name_kernel = "hfapp";

static struct par {
	struct {
		unsigned int freq[2];
	} fsk;

	struct {
		unsigned int baud;
		unsigned int rxinvert;
		unsigned int txinvert;
	} rtty;

	struct {
		unsigned char destcall[4];
		unsigned char selfeccall[4];
		unsigned char mycall[4];
		unsigned int txdelay;
		unsigned int retry;
		unsigned int rxinvert;
		unsigned int txinvert;
	} amtor;

	struct {
		unsigned char destcall[8];
		unsigned char mycall[8];
		unsigned int txdelay;
		unsigned int retry;
		unsigned char longpath;
		unsigned int crcpreset[4];
	} pactor;

	struct {
		unsigned char destcall[10];
		unsigned char mycall[10];
		unsigned int txdelay;
		unsigned int retry;
	} gtor;
} params;

/* --------------------------------------------------------------------- */
/*
 * Logging functions
 */

void errprintf(int severity, const char *fmt, ...)
{
        va_list args;

        va_start(args, fmt);
	fprintf(stderr, "hfterm[%lu]: ", (unsigned long)getpid());
	vfprintf(stderr, fmt, args);
        va_end(args);
        if (severity <= SEV_FATAL)
                exit(1);
}

/* --------------------------------------------------------------------- */

void errstr(int severity, const char *st)
{
        errprintf(severity, "error: %s: %s\n", st, strerror(errno));
}

/* --------------------------------------------------------------------- */

void display_status(const char *data)
{
	GtkText *txt;
	unsigned int len;

	if (!data || !(len = strlen(data)))
		return;
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textstatus"));
	gtk_text_freeze(txt);
	gtk_text_set_point(txt, 0);
	gtk_editable_delete_text(GTK_EDITABLE(txt), 0, ~0);
	gtk_text_insert(txt, NULL, NULL, NULL, data, len);
	gtk_text_thaw(txt);
}

/* --------------------------------------------------------------------- */

static void set_fsk_freq(unsigned int mark, unsigned int space)
{
	struct hfapp_msg msg;
	char buf[16];
	GtkEntry *entry;
	Spectrum *spec;

	msg.hdr.type = htonl(HFAPP_MSG_SET_FSKPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.fpar));
	if (space < 500)
		space = 500;
	if (space > 3500)
		space = 3500;
	if (mark < 500)
		mark = 500;
	if (mark > 3500)
		mark = 3500;
	params.fsk.freq[0] = space;
	params.fsk.freq[1] = mark;
	msg.data.fpar.freq[0] = htons(params.fsk.freq[0]);
	msg.data.fpar.freq[1] = htons(params.fsk.freq[1]);
	msg_send(&msg);
	snprintf(buf, sizeof(buf), "%d", params.fsk.freq[0]);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskspacefreq"));
	gtk_entry_set_text(entry, buf);
	snprintf(buf, sizeof(buf), "%d", params.fsk.freq[1]);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskmarkfreq"));
	gtk_entry_set_text(entry, buf);
	snprintf(buf, sizeof(buf), "%d Hz", params.fsk.freq[0]);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wspec), "specfreqspace"));
	gtk_entry_set_text(entry, buf);
	snprintf(buf, sizeof(buf), "%d Hz", params.fsk.freq[1]);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wspec), "specfreqmark"));
	gtk_entry_set_text(entry, buf);
	spec = SPECTRUM(gtk_object_get_data(GTK_OBJECT(wspec), "spec"));
	spectrum_setmarker(spec, space, mark, -1);
}

/* ---------------------------------------------------------------------- */

#define DBSPAN 80
#define SRATE 8000

#if 0

static int cb_canvas(FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d)
{
        XWindowAttributes winattrs;     
        GC gc;
        XGCValues gcv;
	XButtonEvent *bev = (XButtonEvent *)ev;
	XMotionEvent *mev = (XMotionEvent *)ev;
	unsigned int freq;
	unsigned char buf[16];

	switch (ev->type) {
	case Expose:  /* GraphicsExpose??*/
		XGetWindowAttributes(fl_get_display(), fl_get_canvas_id(fd_spec->scdisp), &winattrs);
		gcv.line_width = 1;
		gcv.line_style = LineSolid;
		gcv.fill_style = FillSolid;
		gc = XCreateGC(fl_get_display(), pixmap, GCLineWidth | GCLineStyle | GCFillStyle, &gcv);
		XSetState(fl_get_display(), gc, col_background, col_background, GXcopy, AllPlanes);
		XCopyArea(fl_get_display(), pixmap, fl_get_canvas_id(fd_spec->scdisp), gr_context, 
			  0, 0, winattrs.width, winattrs.height, 0, 0);
		return 0;

	case ButtonPress:
		freq = (bev->x * SRATE + SPEC_WIDTH) / (2*SPEC_WIDTH);
		if (bev->button == 1)
			set_fsk_freq(freq+freq_shift, freq);
		else if (bev->button == 3)
			set_fsk_freq(freq, freq-freq_shift);
		snprintf(buf, sizeof(buf), "%d Hz", freq);
		fl_set_object_label(fd_spec->freq_pointer, buf);	
		return 0;

	case MotionNotify:
		freq = (mev->x * SRATE + SPEC_WIDTH) / (2*SPEC_WIDTH);
		snprintf(buf, sizeof(buf), "%d Hz", freq);
		fl_set_object_label(fd_spec->freq_pointer, buf);
		return 0;

	default:
		fprintf(stderr, "Canvas: unknown event %d\n", ev->type);
		return 0;
	}
}

#endif

/* --------------------------------------------------------------------- */

#if 0
static void scope_window(int on) 
{
	struct hfapp_msg msg;
	if (on) {
		fl_show_form(fd_spec->spec, FL_PLACE_CENTER, FL_FULLBORDER, 
			     "HF Terminal Spectrum Display");
		if (!(pixmap = XCreatePixmap(fl_get_display(), 
					     fl_get_canvas_id(fd_spec->scdisp), 
					     SPEC_WIDTH, SPEC_HEIGHT, 
					     fl_get_canvas_depth(fd_spec->scdisp))))
			errprintf(SEV_FATAL, "unable to open offscreen pixmap\n");
		fl_add_canvas_handler(fd_spec->scdisp, Expose, cb_canvas, NULL);
		fl_add_canvas_handler(fd_spec->scdisp, MotionNotify, cb_canvas, NULL);
		fl_add_canvas_handler(fd_spec->scdisp, ButtonPress, cb_canvas, NULL);
		gr_context = XCreateGC(fl_get_display(), fl_state[fl_vmode].trailblazer, 0, 0);
		col_zeroline = fl_get_flcolor(FL_RED);
		col_background = fl_get_flcolor(FL_WHITE);
		col_trace = fl_get_flcolor(FL_BLACK);
		col_demodbars = fl_get_flcolor(FL_BLUE);
		
		msg.hdr.type = htonl(HFAPP_MSG_REQ_SAMPLES);
		msg.hdr.len = htonl(sizeof(msg.data.u));
		msg.hdr.err = htonl(ERR_NOERR);
		msg.data.u = htonl(SPEC_WIDTH*2);
		msg_send(&msg);
		scope_on = 1;
		return;
	}
	fl_remove_canvas_handler(fd_spec->scdisp, Expose, cb_canvas);
	fl_remove_canvas_handler(fd_spec->scdisp, MotionNotify, cb_canvas);
	fl_remove_canvas_handler(fd_spec->scdisp, ButtonPress, cb_canvas);
	fl_hide_form(fd_spec->spec);
	XFreeGC(fl_get_display(), gr_context);
	XFreePixmap(fl_get_display(), pixmap);
	scope_on = 0;
}
#endif

/* --------------------------------------------------------------------- */

void scope_draw(float *data)
{
#if 0
        int cnt;
        GC gc;
        XGCValues gcv;
	int yc = 0, ycz;

	if (!scope_on)
		return;
#if 0
        XWindowAttributes winattrs;
        XGetWindowAttributes(fl_get_display(), fl_get_canvas_id(fd_spec->scdisp), &winattrs);
	winattrs.width, winattrs.height;
#endif 
        gcv.line_width = 1;
        gcv.line_style = LineSolid;
        gc = XCreateGC(fl_get_display(), pixmap, GCLineWidth | GCLineStyle, &gcv);
        XSetState(fl_get_display(), gc, col_background, col_background, GXcopy, AllPlanes);
	XFillRectangle(fl_get_display(), pixmap, gc, 0, 0, SPEC_WIDTH, SPEC_HEIGHT);
	/*
	 * draw grid
	 */
        XSetForeground(fl_get_display(), gc, col_zeroline);
	for (cnt = 0; cnt < SPEC_HEIGHT; cnt += (10 * SPEC_HEIGHT + DBSPAN / 2) / DBSPAN)
		XDrawLine(fl_get_display(), pixmap, gc, 0, cnt, SPEC_WIDTH-1, cnt);
	for (cnt = 0; cnt < SPEC_WIDTH; cnt += (500 * (2*SPEC_WIDTH) + SRATE / 2) / SRATE)
		XDrawLine(fl_get_display(), pixmap, gc, cnt, 0, cnt, SPEC_HEIGHT-1);
	/*
	 * draw demodulator frequency bars
	 */
        XSetForeground(fl_get_display(), gc, col_demodbars);
	cnt = (fsk_freq[0] * (2*SPEC_WIDTH) + SRATE / 2) / SRATE;
	XDrawLine(fl_get_display(), pixmap, gc, cnt, 0, cnt, SPEC_HEIGHT-1);
	cnt = (fsk_freq[1] * (2*SPEC_WIDTH) + SRATE / 2) / SRATE;
	XDrawLine(fl_get_display(), pixmap, gc, cnt, 0, cnt, SPEC_HEIGHT-1);	
	/*
	 * draw curve
	 */
        XSetForeground(fl_get_display(), gc, col_trace);
        for (cnt = 0; cnt < SPEC_WIDTH; cnt++) {
		ycz = yc;
		yc = -data[cnt] / DBSPAN * SPEC_HEIGHT;
		if (yc < 0) 
			yc = 0;
		if (yc >= SPEC_HEIGHT)
			yc = SPEC_HEIGHT-1;
		if (cnt > 0)
			XDrawLine(fl_get_display(), pixmap, gc, cnt-1, ycz, cnt, yc);
	}
        XCopyArea(fl_get_display(), pixmap, fl_get_canvas_id(fd_spec->scdisp), gr_context, 0, 0, 
                  SPEC_WIDTH, SPEC_HEIGHT, 0, 0);
        XFreeGC(fl_get_display(), gc);
        XSync(fl_get_display(), 0);
#endif
}

/* --------------------------------------------------------------------- */

static void param_get(void)
{
	GtkEntry *entry;
	GtkToggleButton *tog;

	/* FSK parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskspacefreq"));
	params.fsk.freq[0] = strtoul(gtk_entry_get_text(entry), NULL, 0);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskmarkfreq"));
	params.fsk.freq[1] = strtoul(gtk_entry_get_text(entry), NULL, 0);
	/* RTTY parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "rttybaudrate"));
	params.rtty.baud = strtoul(gtk_entry_get_text(entry), NULL, 0);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "rttyinvert"));
	params.rtty.rxinvert = gtk_toggle_button_get_active(tog);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "rttyrxtxinvert"));
	params.rtty.txinvert = gtk_toggle_button_get_active(tog);
	if (params.rtty.rxinvert)
		params.rtty.txinvert = !params.rtty.txinvert;
	/* Amtor parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtordestcall"));
	strncpy(params.amtor.destcall, gtk_entry_get_text(entry), sizeof(params.amtor.destcall));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtormycall"));
	strncpy(params.amtor.mycall, gtk_entry_get_text(entry), sizeof(params.amtor.mycall));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtorselfeccall"));
	strncpy(params.amtor.selfeccall, gtk_entry_get_text(entry), sizeof(params.amtor.selfeccall));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtortxdelay"));
	params.amtor.txdelay = strtoul(gtk_entry_get_text(entry), NULL, 0);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtorretry"));
	params.amtor.retry = strtoul(gtk_entry_get_text(entry), NULL, 0);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "amtorinvert"));
	params.amtor.rxinvert = gtk_toggle_button_get_active(tog);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "amtorrxtxinvert"));
	params.amtor.txinvert = gtk_toggle_button_get_active(tog);
	if (params.amtor.rxinvert)
		params.amtor.txinvert = !params.amtor.txinvert;
	/* Pactor parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcall"));
	strncpy(params.pactor.destcall, gtk_entry_get_text(entry), sizeof(params.pactor.destcall));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactormycall"));
	strncpy(params.pactor.mycall, gtk_entry_get_text(entry), sizeof(params.pactor.mycall));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorretry"));
	params.pactor.retry = strtoul(gtk_entry_get_text(entry), NULL, 0);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactortxdelay"));
	params.pactor.txdelay = strtoul(gtk_entry_get_text(entry), NULL, 0);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "pactorlongpath"));
	params.pactor.longpath = gtk_toggle_button_get_active(tog);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcrc0"));
	params.pactor.crcpreset[0] = strtoul(gtk_entry_get_text(entry), NULL, 16);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcrc1"));
	params.pactor.crcpreset[1] = strtoul(gtk_entry_get_text(entry), NULL, 16);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcrc2"));
	params.pactor.crcpreset[2] = strtoul(gtk_entry_get_text(entry), NULL, 16);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcrc3"));
	params.pactor.crcpreset[3] = strtoul(gtk_entry_get_text(entry), NULL, 16);
	/* GTOR parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtordestcall"));
	strncpy(params.gtor.destcall, gtk_entry_get_text(entry), sizeof(params.gtor.destcall));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtormycall"));
	strncpy(params.gtor.mycall, gtk_entry_get_text(entry), sizeof(params.gtor.mycall));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtorretry"));
	params.gtor.retry = strtoul(gtk_entry_get_text(entry), NULL, 0);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtortxdelay"));
	params.gtor.txdelay = strtoul(gtk_entry_get_text(entry), NULL, 0);
}
	
static void param_set(void)
{
	GtkEntry *entry;
	GtkToggleButton *tog;
	char buf[16];

	/* FSK parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskspacefreq"));
	snprintf(buf, sizeof(buf), "%u", params.fsk.freq[0]);
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskmarkfreq"));
	snprintf(buf, sizeof(buf), "%u", params.fsk.freq[1]);
	gtk_entry_set_text(entry, buf);
	/* RTTY parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "rttybaudrate"));
	snprintf(buf, sizeof(buf), "%u", params.rtty.baud);
	gtk_entry_set_text(entry, buf);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "rttyinvert"));
	gtk_toggle_button_set_active(tog, params.rtty.rxinvert);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "rttyrxtxinvert"));
	gtk_toggle_button_set_active(tog, params.rtty.txinvert ^ params.rtty.rxinvert);
	/* Amtor parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtordestcall"));
	strncpy(buf, params.amtor.destcall, sizeof(params.amtor.destcall));
	buf[sizeof(params.amtor.destcall)] = 0;
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtormycall"));
	strncpy(buf, params.amtor.mycall, sizeof(params.amtor.mycall));
	buf[sizeof(params.amtor.mycall)] = 0;
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtorselfeccall"));
	strncpy(buf, params.amtor.selfeccall, sizeof(params.amtor.selfeccall));
	buf[sizeof(params.amtor.selfeccall)] = 0;
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtortxdelay"));
	snprintf(buf, sizeof(buf), "%u", params.amtor.txdelay);
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "amtorretry"));
	snprintf(buf, sizeof(buf), "%u", params.amtor.txdelay);
	gtk_entry_set_text(entry, buf);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "amtorinvert"));
	gtk_toggle_button_set_active(tog, params.amtor.rxinvert);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "amtorrxtxinvert"));
	gtk_toggle_button_set_active(tog, params.amtor.txinvert ^ params.amtor.rxinvert);
	/* Pactor parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcall"));
	strncpy(buf, params.pactor.destcall, sizeof(params.pactor.destcall));
	buf[sizeof(params.pactor.destcall)] = 0;
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactormycall"));
	strncpy(buf, params.pactor.mycall, sizeof(params.pactor.mycall));
	buf[sizeof(params.pactor.mycall)] = 0;
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorretry"));
	snprintf(buf, sizeof(buf), "%u", params.pactor.retry);
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactortxdelay"));
	snprintf(buf, sizeof(buf), "%u", params.pactor.txdelay);
	gtk_entry_set_text(entry, buf);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "pactorlongpath"));
	gtk_toggle_button_set_active(tog, params.pactor.longpath);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcrc0"));
	snprintf(buf, sizeof(buf), "%04X", params.pactor.crcpreset[0]);
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcrc1"));
	snprintf(buf, sizeof(buf), "%04X", params.pactor.crcpreset[1]);
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcrc2"));
	snprintf(buf, sizeof(buf), "%04X", params.pactor.crcpreset[2]);
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcrc3"));
	snprintf(buf, sizeof(buf), "%04X", params.pactor.crcpreset[3]);
	gtk_entry_set_text(entry, buf);
	/* GTOR parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtordestcall"));
	strncpy(buf, params.gtor.destcall, sizeof(params.gtor.destcall));
	buf[sizeof(params.gtor.destcall)] = 0;
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtormycall"));
	strncpy(buf, params.gtor.mycall, sizeof(params.gtor.mycall));
	buf[sizeof(params.gtor.mycall)] = 0;
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtorretry"));
	snprintf(buf, sizeof(buf), "%u", params.gtor.retry);
	gtk_entry_set_text(entry, buf);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtortxdelay"));
	snprintf(buf, sizeof(buf), "%u", params.gtor.txdelay);
	gtk_entry_set_text(entry, buf);
}
	
static void param_kernel(void)
{
	struct hfapp_msg msg;

	/* FSK parameters */
	set_fsk_freq(params.fsk.freq[1], params.fsk.freq[0]);
	/* RTTY parameters */
	msg.hdr.type = htonl(HFAPP_MSG_SET_RTTYPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.rpar));
	msg.data.rpar.baud = htons(params.rtty.baud);
	msg.data.rpar.rxinvert = params.rtty.rxinvert;
	msg.data.rpar.txinvert = params.rtty.txinvert;
	msg_send(&msg);
	/* Amtor parameters */
	msg.hdr.type = htonl(HFAPP_MSG_SET_AMTORPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.apar));
	strncpy(msg.data.apar.destcall, params.amtor.destcall, sizeof(msg.data.apar.destcall));
	strncpy(msg.data.apar.selfeccall, params.amtor.selfeccall, sizeof(msg.data.apar.selfeccall));
	strncpy(msg.data.apar.mycall, params.amtor.mycall, sizeof(msg.data.apar.mycall));
	msg.data.apar.txdelay = htons(params.amtor.txdelay);
	msg.data.apar.retry = htons(params.amtor.retry);
	msg.data.apar.rxinvert = params.amtor.rxinvert ;
	msg.data.apar.txinvert = params.amtor.txinvert;
	msg_send(&msg);
	/* Pactor parameters */
	msg.hdr.type = htonl(HFAPP_MSG_SET_PACTORPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.ppar));
	strncpy(msg.data.ppar.destcall, params.pactor.destcall, sizeof(msg.data.ppar.destcall));
	strncpy(msg.data.ppar.mycall, params.pactor.mycall, sizeof(msg.data.ppar.mycall));
	msg.data.ppar.txdelay = htons(params.pactor.txdelay);
	msg.data.ppar.retry = htons(params.pactor.retry);
	msg.data.ppar.longpath = params.pactor.longpath;
	msg.data.ppar.crcpreset[0] = htons(params.pactor.crcpreset[0]);
	msg.data.ppar.crcpreset[1] = htons(params.pactor.crcpreset[1]);
	msg.data.ppar.crcpreset[2] = htons(params.pactor.crcpreset[2]);
	msg.data.ppar.crcpreset[3] = htons(params.pactor.crcpreset[3]);
	msg_send(&msg);
	/* GTOR parameters */
	msg.hdr.type = htonl(HFAPP_MSG_SET_GTORPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.gpar));
	strncpy(msg.data.gpar.destcall, params.gtor.destcall, sizeof(msg.data.gpar.destcall));
	strncpy(msg.data.gpar.mycall, params.gtor.mycall, sizeof(msg.data.gpar.mycall));
	msg.data.gpar.txdelay = htons(params.gtor.txdelay);
	msg.data.gpar.retry = htons(params.gtor.retry);
	msg_send(&msg);
}

/* --------------------------------------------------------------------- */

void write_input(unsigned char *data, int datalen)
{
	GtkText *txt;

	if (!data && datalen <= 0)
		return;
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textmain"));
	gtk_text_insert(txt, NULL, NULL, NULL, data, datalen);
}

void write_output(unsigned char *data, int datalen)
{
	GtkText *txt;

	if (!data && datalen <= 0)
		return;
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textmain"));
	gtk_text_insert(txt, NULL, &(GTK_WIDGET(txt)->style->fg[GTK_WIDGET_STATE(GTK_WIDGET(txt))]), NULL, data, datalen);
}

void write_monitor(unsigned char *data, int datalen)
{
	GtkText *txt;

	if (!data && datalen <= 0)
		return;
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmonitor), "textmonitor"));
	if (!GTK_WIDGET_DRAWABLE(GTK_WIDGET(txt)))
		return;
	gtk_text_insert(txt, NULL, NULL, NULL, data, datalen);
}

/* --------------------------------------------------------------------- */

void write_kernel(unsigned char *data, int datalen)
{
	int i;

	if (fd_krnl < 0) {
		errprintf(SEV_WARNING, "write_kernel: fd closed\n");
		return;
	}
	while (datalen > 0) {
		i = write(fd_krnl, data, datalen);
		if (i < 0) {
			if (errno != EAGAIN)
				errstr(SEV_FATAL, "write");
		} else {
			data += i;
			datalen -= i;
		}
	}
}

/* --------------------------------------------------------------------- */

static void edit_newline(void)
{
        struct hfapp_msg msg;
	GtkEntry *entry;

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wmain), "textedit"));
	gtk_entry_set_text(entry, "");
	msg.data.b[0] = '\r';
	msg.data.b[1] = '\n';
        msg.hdr.type = htonl(HFAPP_MSG_DATA_TRANSMIT);
        msg.hdr.len = htonl(2);
        msg.hdr.err = htonl(ERR_NOERR);
        msg_send(&msg);
}

static void edit_backspace(void)
{
        struct hfapp_msg msg;
	GtkEditable *entry;
	gint pos;

	entry = GTK_EDITABLE(gtk_object_get_data(GTK_OBJECT(wmain), "textedit"));
	pos = gtk_editable_get_position(entry);
	if (pos > 0) {
		gtk_editable_set_position(entry, pos-1);
		gtk_editable_delete_text(entry, pos, pos);
	}
	msg.data.b[0] = '\b';
        msg.hdr.type = htonl(HFAPP_MSG_DATA_TRANSMIT);
        msg.hdr.len = htonl(1);
        msg.hdr.err = htonl(ERR_NOERR);
        msg_send(&msg);
}

static void edit_addchar(char v)
{
        struct hfapp_msg msg;
	GtkEntry *entry;
	char buf[2];

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wmain), "textedit"));
	buf[0] = v;
	buf[1] = 0;
	gtk_entry_append_text(entry, buf);
	msg.data.b[0] = v;
        msg.hdr.type = htonl(HFAPP_MSG_DATA_TRANSMIT);
        msg.hdr.len = htonl(1);
        msg.hdr.err = htonl(ERR_NOERR);
        msg_send(&msg);
}

/* --------------------------------------------------------------------- */

void spec_samples(short *data, unsigned int len)
{
	struct hfapp_msg msg;
	short tmp[SPECTRUM_NUMSAMPLES];
	unsigned int i;
	Spectrum *spec;

	if (!scope_on)
		return;
	if (len < SPECTRUM_NUMSAMPLES)
		errprintf(SEV_WARNING, "HFAPP_MSG_ACK_SAMPLES: message too short\n");
	else {
		for (i = 0; i < SPECTRUM_NUMSAMPLES; i++)
			tmp[i] = ntohs(data[i]);
		spec = SPECTRUM(gtk_object_get_data(GTK_OBJECT(wspec), "spec"));
		spectrum_setdata(spec, tmp);
	}
	msg.hdr.type = htonl(HFAPP_MSG_REQ_SAMPLES);
	msg.hdr.len = htonl(sizeof(msg.data.u));
	msg.hdr.err = htonl(ERR_NOERR);
	msg.data.u = htonl(SPECTRUM_NUMSAMPLES);
	msg_send(&msg);
}

/* --------------------------------------------------------------------- */

static unsigned int get_freq_shift(void)
{
	GtkToggleButton *tog;

	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wspec), "shift170"));
	if (gtk_toggle_button_get_active(tog))
		return 170;
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wspec), "shift200"));
	if (gtk_toggle_button_get_active(tog))
		return 200;
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wspec), "shift425"));
	if (gtk_toggle_button_get_active(tog))
		return 425;
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wspec), "shift800"));
	if (gtk_toggle_button_get_active(tog))
		return 800;
	return 0;
}

gboolean on_spec_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	unsigned int freq;
	char buf[16];
	GtkEntry *entry;
	Spectrum *spec;

	freq = ((SRATE/2) * event->x + SPECTRUM_WIDTH/2) / SPECTRUM_WIDTH;
	snprintf(buf, sizeof(buf), "%d Hz", freq);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wspec), "specfreqpointer"));
	gtk_entry_set_text(entry, buf);
	spec = SPECTRUM(gtk_object_get_data(GTK_OBJECT(wspec), "spec"));
	spectrum_setmarker(spec, -1, -1, freq);
	printf("on_scope_motion_event: x %g y %g\n", event->x, event->y);
	return FALSE;
}


gboolean on_spec_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	unsigned int freq, shift;

	freq = ((SRATE/2) * event->x + SPECTRUM_WIDTH/2) / SPECTRUM_WIDTH;
	shift = get_freq_shift();
	switch (event->button) {
	case 1:
		set_fsk_freq(freq, freq+shift);
		break;

	case 2:
		set_fsk_freq(freq-shift/2, freq+shift/2);
		break;

	case 3:
		set_fsk_freq(freq-shift, freq);
		break;
	}
	printf("on_scope_button_press_event: x %g y %g  button 0x%04x\n", event->x, event->y, event->button);
	return FALSE;
}

void on_parok_clicked(GtkButton *button, gpointer user_data)
{
	param_get();
	param_set();
	param_kernel();
	gtk_widget_hide(wpar);
}


void on_parcancel_clicked(GtkButton *button, gpointer user_data)
{
	param_set();
	gtk_widget_hide(wpar);
}


void on_quit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_main_quit();
}


void on_become_irs_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_IRS);
	msg_send(&msg);
}


void on_become_iss_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_ISS);
	msg_send(&msg);
}


void on_qrt_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_QRT);
	msg_send(&msg);
}


void on_speedup_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_SPEEDUP);
	msg_send(&msg);
}

void on_uppercase_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_CASE_UPPER);
	msg_send(&msg);
}


void on_lowercase_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_CASE_LOWER);
	msg_send(&msg);
}


void on_figurecase_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_CASE_FIGURE);
	msg_send(&msg);
}


void on_standby_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_STANDBY);
	msg_send(&msg);
}

void on_pactor_arq_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_PACTOR_ARQ);
	msg_send(&msg);
}


void on_pactor_fec_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_PACTOR_FEQ);
	msg_send(&msg);
}


void on_gtor_arq1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_GTOR_ARQ);
	msg_send(&msg);
}


void on_amtor_arq_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_AMTOR_ARQ);
	msg_send(&msg);
}


void on_amtor_collective_fec_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_AMTOR_COLFEQ);
	msg_send(&msg);
}


void on_amtor_selective_fec_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_AMTOR_SELFEQ);
	msg_send(&msg);
}


void on_rtty_receive_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_RTTY_RX);
	msg_send(&msg);
}


void on_rtty_transmit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_RTTY_TX);
	msg_send(&msg);
}

void on_frequency_spectrum_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.type = htonl(HFAPP_MSG_REQ_SAMPLES);
	msg.hdr.len = htonl(sizeof(msg.data.u));
	msg.hdr.err = htonl(ERR_NOERR);
	msg.data.u = htonl(SPECTRUM_NUMSAMPLES);
	msg_send(&msg);
	scope_on = 1;
	gtk_widget_show(wspec);
}

void on_parameters_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(wpar);
}


void on_monitor_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(wmonitor);
}

void on_about_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(wabout);
}


gboolean on_text_keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	char c = event->keyval;

	printf("on_text_keypress_event: 0x%x %c  state %u\n", event->keyval, (c >= ' ' && c <= 0x7f) ? c : '.', event->state);

	if (event->state & ~(GDK_SHIFT_MASK | GDK_LOCK_MASK))
		return FALSE;
	if (event->keyval == GDK_BackSpace) {
		edit_backspace();
		return TRUE;
	}
	if (event->keyval == GDK_Return) {
		edit_newline();
		return TRUE;
	}
	if (event->keyval >= 32 && event->keyval < 128) {
		edit_addchar(event->keyval);
		return TRUE;
	}
	return FALSE;
}


gboolean on_text_keyrelease_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	char c = event->keyval;

	printf("on_text_keyrelease_event: 0x%x %c  state %u\n", event->keyval, (c >= ' ' && c <= 0x7f) ? c : '.', event->state);
	return TRUE;
}

gboolean on_wspec_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_hide(widget);
	scope_on = 0;
	return TRUE;
}

gboolean on_wmain_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_main_quit();
	return FALSE;
}


gboolean on_wmain_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_main_quit();
	return FALSE;
}

void on_aboutok_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(wabout);
}

/* --------------------------------------------------------------------- */

static GPollFD msgpfd;

static gboolean poll_prepare(gpointer source_data, GTimeVal *current_time, gint *timeout, gpointer user_data)
{
	*timeout = -1;
        return FALSE;
}

static gboolean poll_check(gpointer source_data, GTimeVal *current_time, gpointer user_data)
{
	if (msgpfd.revents & G_IO_IN)
		return TRUE;
        return FALSE;
}

static gboolean poll_dispatch(gpointer source_data, GTimeVal *current_time, gpointer user_data)
{
	if (msgpfd.revents & G_IO_IN)
		msg_process(msgpfd.fd);
        return TRUE;
}

static GSourceFuncs poll_funcs =
{
        poll_prepare,
        poll_check,
        poll_dispatch,
        NULL
};

static void init(void)
{
	static const char *mycall = MYCALL;
	struct sockaddr_un saddr;
	char buf[16];
	char *bp1;
	const char *bp2;

	if ((fd_krnl = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		errstr(SEV_FATAL, "socket");
	saddr.sun_family = AF_UNIX;
	strncpy(saddr.sun_path, name_kernel, sizeof(saddr.sun_path));
	if (connect(fd_krnl, (struct sockaddr *)&saddr, sizeof(saddr))) {
		errstr(SEV_WARNING, "connect");
		close(fd_krnl);
		msgpfd.fd = fd_krnl = -1;
	} else {
		g_source_add(G_PRIORITY_HIGH, FALSE, &poll_funcs, NULL, NULL, NULL);
		msgpfd.fd = fd_krnl;
		msgpfd.events = G_IO_IN;
		msgpfd.revents = 0;
		g_main_add_poll(&msgpfd, G_PRIORITY_HIGH);
	}
	
	memset(&params, 0, sizeof(params));
	strncpy(params.gtor.mycall, mycall, sizeof(params.gtor.mycall));
	params.gtor.txdelay = 30;
	params.gtor.retry = 30;
	strncpy(params.pactor.mycall, mycall, sizeof(params.pactor.mycall));
	params.pactor.txdelay = 30;
	params.pactor.retry = 30;
	params.pactor.crcpreset[0] = strtoul(PCT_CRC_0, NULL, 16);
	params.pactor.crcpreset[1] = strtoul(PCT_CRC_1, NULL, 16);
	params.pactor.crcpreset[2] = strtoul(PCT_CRC_2, NULL, 16);
	params.pactor.crcpreset[3] = strtoul(PCT_CRC_3, NULL, 16);

	bp2 = mycall-1+strlen(mycall);
	buf[sizeof(buf)-1] = 0;
	bp1 = buf+4;
	*bp1 = 0;
	while (bp2 >= mycall) {
		if (isalpha(*bp2))
			*--bp1 = *bp2;
		bp2--;
		if (bp1 <= buf)
			bp1 = buf+1;
	}
	strncpy(params.amtor.mycall, buf, sizeof(params.amtor.mycall));
	strncpy(params.amtor.selfeccall, buf, sizeof(params.amtor.selfeccall));
	params.amtor.txdelay = 30;
	params.amtor.retry = 30;
	params.rtty.baud = 45;
	set_fsk_freq(FREQ_MARK, FREQ_SPACE);
}

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
        int c, err = 0;

	bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain(PACKAGE);

	gtk_set_locale();
	gtk_init(&argc, &argv);
        while ((c = getopt(argc, argv, "k:")) != -1) 
                switch (c) {
                case 'k':
                        name_kernel = optarg;
                        break;

                default:
                        err++;
                        break;
                }
        if (err) {
                fprintf(stderr, "usage: hfterm [-k <hfkernel>]\n");
                exit(1);
        }
#if 0
	add_pixmap_directory(PACKAGE_DATA_DIR "/pixmaps");
	add_pixmap_directory(PACKAGE_SOURCE_DIR "/pixmaps");
#endif
	/*
	 * The following code was added by Glade to create one of each component
	 * (except popup menus), just so that you see something after building
	 * the project. Delete any components that you don't want shown initially.
	 */
	wspec = create_wspec();
	wpar = create_wpar();
	wabout = create_wabout();
	wmonitor = create_wmonitor();
	wmain = create_wmain();
 	init();
	param_set();
	param_kernel();
	gtk_widget_show(wmain);
	display_status("HFTerm\n(C) 1999 by Thomas Sailer, HB9JNX/AE4WA\n");

	gtk_main();
	return 0;
}

/* --------------------------------------------------------------------- */




