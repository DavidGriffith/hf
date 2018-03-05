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
 *  This is the Linux realtime sound output driver
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "forms.h"
#include "hfterm.h"
#include "hfapp.h"
#include "hft.h"
/*#include "byteorder.h"*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <grp.h>
#include <termios.h>
#include <ctype.h>
#include <netinet/in.h>  /* ntoh? and hton? */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

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

/*
 * fds and pids: 
 * 0 = input
 * 1 = output
 * 2 = monitor
 */
static int termfds[3] = { -1, -1, -1 };
static pid_t termpids[3];

static int fd_krnl;

static char *name_kernel = "hfapp";

static FD_menubar *fd_menubar;
static FD_spec *fd_spec;
static FD_gtor_params *fd_gtor_params;
static FD_pactor_params *fd_pactor_params;
static FD_amtor_params *fd_amtor_params;
static FD_rtty_params *fd_rtty_params;
static FD_fsk_params *fd_fsk_params;
static FD_mixer_params *fd_mixer_params;

static Pixmap pixmap;
static unsigned long col_zeroline;
static unsigned long col_background;
static unsigned long col_trace;
static unsigned long col_demodbars;
static GC gr_context;
int scope_on = 0;

static unsigned int fsk_freq[2];
static unsigned int freq_shift = 200;

/* --------------------------------------------------------------------- */
#ifdef SOLARIS

#include <stdarg.h>

int snprintf(char *buf, size_t len, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
int vsnprintf(char *buf, size_t len, const char *fmt, va_list ap);

int snprintf(char *buf, size_t len, const char *fmt, ...) 
{
        va_list args;
	int ret;
	
	if (len <= 0)
		return 0;
        va_start(args, fmt);
	ret = vsprintf(buf, fmt, args);
        va_end(args);
	if (ret+1 > len)
		errprintf(SEV_FATAL, "snprintf: buffer overrun, buffer size %d, "
			  "required %d\n", len, ret);
	return ret;
}

int vsnprintf(char *buf, size_t len, const char *fmt, va_list ap)
{
	int ret;

	if (len <= 0)
		return 0;
	ret = vsprintf(buf, fmt, ap);
	if (ret+1 > len)
		errprintf(SEV_FATAL, "vsnprintf: buffer overrun, buffer size %d, "
			  "required %d\n", len, ret);
	return ret;
}

#endif /* SOLARIS */

/* --------------------------------------------------------------------- */

static void cleanup(void)
{
	int j, i, stat;
	int done;

	signal(SIGCHLD, SIG_IGN);
	if (fd_krnl > 0) {
		shutdown(fd_krnl, 2);
		close(fd_krnl);
		fd_krnl = -1;
	}
	for (j = 0; j < 4; j++) {
		done = 1;
		for (i = 0; i < sizeof(termfds)/sizeof(termfds[0]); i++) {
			if (termfds[i] >= 0) {
				if (kill(termpids[i], SIGABRT))
                                        if (errno == ESRCH) {
						close(termfds[i]);
						termfds[i] = -1;
					}
			}
			done = done && (termfds[i] < 0);
		}
		if (done)
			break;
		if (j)
			sleep(1);
		done = 1;
		for (i = 0; i < sizeof(termfds)/sizeof(termfds[0]); i++) {
			if (termfds[i] >= 0) {
				if (waitpid(termpids[i], &stat, WNOHANG) ==
				    termpids[i]) {
					close(termfds[i]);
					termfds[i] = -1;
				}
			}
			done = done && (termfds[i] < 0);
		}	
		if (done)
			break;
	}
	for (i = 0; i < sizeof(termfds)/sizeof(termfds[0]); i++)
		if (termfds[i] >= 0) 
			close(termfds[i]);
}

/* --------------------------------------------------------------------- */

static void sig_cleanup(int sig)
{
	errprintf(SEV_WARNING, "Caught signal %d\n", sig);
	cleanup();
}

/* --------------------------------------------------------------------- */
/*
 * Logging functions
 */

void errprintf(int severity, const char *fmt, ...)
{
        va_list args;

        va_start(args, fmt);
	fprintf(stderr, "hfterm[%u]: ", getpid());
	vfprintf(stderr, fmt, args);
        va_end(args);
        if (severity <= SEV_FATAL) {
		cleanup();
                exit(1);
	}
}

/* --------------------------------------------------------------------- */

void errstr(int severity, const char *st)
{
        errprintf(severity, "error: %s: %s\n", st, strerror(errno));
}

/* --------------------------------------------------------------------- */

void display_status(const char *data)
{
	fl_clear_browser(fd_menubar->status);
	if (!data) 
		return;
	fl_addto_browser(fd_menubar->status, data);
}

/* --------------------------------------------------------------------- */

static void set_fsk_freq(unsigned int mark, unsigned int space)
{
	struct hfapp_msg msg;
	char buf[16];

	msg.hdr.type = htonl(HFAPP_MSG_SET_FSKPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.fpar));
	if (space >= 500 && space <= 3500)
		fsk_freq[0] = space;
	if (mark >= 500 && mark <= 3500)
		fsk_freq[1] = mark;
	msg.data.fpar.freq[0] = htons(fsk_freq[0]);
	msg.data.fpar.freq[1] = htons(fsk_freq[1]);
	msg_send(&msg);
	snprintf(buf, sizeof(buf), "%d", fsk_freq[0]);
	fl_set_input(fd_fsk_params->freqspace, buf);
	snprintf(buf, sizeof(buf), "%d", fsk_freq[1]);
	fl_set_input(fd_fsk_params->freqmark, buf);
	snprintf(buf, sizeof(buf), "%d Hz", fsk_freq[0]);
	fl_set_object_label(fd_spec->freq_space, buf);
	snprintf(buf, sizeof(buf), "%d Hz", fsk_freq[1]);
	fl_set_object_label(fd_spec->freq_mark, buf);
}

/* ---------------------------------------------------------------------- */

#define DBSPAN 80
#define SRATE 8000

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

/* --------------------------------------------------------------------- */

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

/* --------------------------------------------------------------------- */

void scope_draw(float *data)
{
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
}

/* --------------------------------------------------------------------- */

static void find_pty(int *fdm, int *fds, char *name, int namesz)
{
        static const char ptych1[] = "pqrstuvwxyzabcde";
        static const char ptych2[] = "0123456789abcdef";
        char ptyname[] = "/dev/pty00";
        struct group *gr = getgrnam("tty");
        int i, j;

        for (i = 0; i < strlen(ptych1); i++) {
                ptyname[8] = ptych1[i];
                for (j = 0; j < strlen(ptych2); j++) {
                        ptyname[9] = ptych2[j];
                        if ((*fdm = open(ptyname, O_RDWR)) >= 0) {
                                chown(ptyname, getuid(), gr ? gr->gr_gid : -1);
                                chmod(ptyname, S_IRUSR|S_IWUSR|S_IWGRP);
                                ptyname[5] = 't';
                                if ((*fds = open(ptyname, O_RDWR)) >= 0) {
					if (name)
						strncpy(name, ptyname, namesz);
                                        return;
				}
                                close(*fdm);
                                ptyname[5] = 'p';
                        }
                }
        }
        errprintf(SEV_FATAL, "Unable to find free pty/tty pair\n");
}

/* ---------------------------------------------------------------------- */

static void fork_xterm(int *fd, pid_t *cpid, const char *title, 
		       const char *resname)
{
        int mfd, sfd;
        char optconn[16];
        int pid;
        struct termios tm;
        char buf[32];
	char name[16];
        int i;

	find_pty(&mfd, &sfd, name, sizeof(name));
        memset(&tm, 0, sizeof(tm));
        tm.c_iflag = 0;
        tm.c_oflag = 0;
        tm.c_cflag = B9600 | CS8 | CREAD;
        tm.c_lflag = 0;
        tm.c_cc[VMIN] = tm.c_cc[VTIME] = 0; 
	tcsetattr(sfd, TCSAFLUSH, &tm);
        switch (pid = fork()) {
        case -1:
                close(mfd);
                close(sfd);
		errstr(SEV_FATAL, "fork");

        case 0:
                close(mfd);
                sprintf(optconn, "-S%c%c%d", name[8], name[9], sfd);
                execlp("xterm", "xterm", optconn, "-j", "-s", "-sl", "512",
                       "-ut", "-T", title ? title : "Terminal", 
                       "-name", resname ? resname : "xterm", NULL);
                errstr(SEV_FATAL, "exec'ing xterm");

        default:
                break;
        }
        *fd = mfd;
        close(sfd);
	*cpid = pid;
        i = read(mfd, buf, sizeof(buf));
        if (i < 0) 
                errstr(SEV_FATAL, "fork_xterm");
}

/* --------------------------------------------------------------------- */
/*
 * user interface callbacks
 */

void cb_shift(FL_OBJECT *ob, long data)
{
	freq_shift = data;
}

/* --------------------------------------------------------------------- */

void cb_filemenu(FL_OBJECT *ob, long data)
{
	int item = fl_get_menu(ob);
	
	if (item <= 0)
		return;

	cleanup();
	exit(0);
}

/* --------------------------------------------------------------------- */

void cb_utilmenu(FL_OBJECT *ob, long data)
{
	int item = fl_get_menu(ob);
	unsigned int mode;

	if (item <= 0)
		return;
	switch (item) {
	case 1:
		mode = fl_get_menu_item_mode(fd_menubar->util, 1);
		scope_window(mode & FL_PUP_CHECK);
		errprintf(SEV_INFO, "menu item mode: %u  checked: %d\n", mode, !!(mode & FL_PUP_CHECK));
		return;
			
	case 2:
		fl_show_form(fd_fsk_params->fsk_params, FL_PLACE_CENTER, FL_FULLBORDER, "FSK Parameters");
		return;

	case 3:
		fl_show_form(fd_gtor_params->gtor_params, FL_PLACE_CENTER, FL_FULLBORDER, "GTOR Parameters");
		return;

	case 4:
		fl_show_form(fd_pactor_params->pactor_params, FL_PLACE_CENTER, FL_FULLBORDER, "Pactor Parameters");
		return;

	case 5:
		fl_show_form(fd_amtor_params->amtor_params, FL_PLACE_CENTER, FL_FULLBORDER, "Amtor Parameters");
		return;

	case 6:
		fl_show_form(fd_rtty_params->rtty_params, FL_PLACE_CENTER, FL_FULLBORDER, "RTTY Parameters");
		return;

	case 7:
		fl_show_form(fd_mixer_params->mixer_params, FL_PLACE_CENTER, FL_FULLBORDER, "Mixer Parameters");
		return;

	default:
		errprintf(SEV_INFO, "menu item %d clicked\n", item);
		return;
	}
}

/* --------------------------------------------------------------------- */

void cb_statemenu(FL_OBJECT *ob, long data)
{
	int item = fl_get_menu(ob);
	struct hfapp_msg msg;

	if (item <= 0)
		return;
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	switch (item) {
	case 1:
		msg.hdr.type = htonl(HFAPP_MSG_MODE_IRS);
		msg_send(&msg);
		return;

	case 2:
		msg.hdr.type = htonl(HFAPP_MSG_MODE_ISS);
		msg_send(&msg);
		return;

	case 3:
		msg.hdr.type = htonl(HFAPP_MSG_MODE_QRT);
		msg_send(&msg);
		return;

	case 4:
		msg.hdr.type = htonl(HFAPP_MSG_MODE_SPEEDUP);
		msg_send(&msg);
		return;

	case 5:
		msg.hdr.type = htonl(HFAPP_MSG_CASE_UPPER);
		msg_send(&msg);
		return;

	case 6:
		msg.hdr.type = htonl(HFAPP_MSG_CASE_LOWER);
		msg_send(&msg);
		return;

	case 7:
		msg.hdr.type = htonl(HFAPP_MSG_CASE_FIGURE);
		msg_send(&msg);
		return;

	default:
		errprintf(SEV_INFO, "menu item %d clicked\n", item);
		return;
	}
}

/* --------------------------------------------------------------------- */

void cb_modemenu(FL_OBJECT *ob, long data)
{
	int item = fl_get_menu(ob);
	struct hfapp_msg msg;

	if (item <= 0)
		return;
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	switch (item) {
	case 1:
		msg.hdr.type = htonl(HFAPP_MSG_START_STANDBY);
		msg_send(&msg);
		return;

	case 2:
		msg.hdr.type = htonl(HFAPP_MSG_START_PACTOR_ARQ);
		msg_send(&msg);
		return;

	case 3:
		msg.hdr.type = htonl(HFAPP_MSG_START_PACTOR_FEQ);
		msg_send(&msg);
		return;

	case 4:
		msg.hdr.type = htonl(HFAPP_MSG_START_GTOR_ARQ);
		msg_send(&msg);
		return;

	case 5:
		msg.hdr.type = htonl(HFAPP_MSG_START_AMTOR_ARQ);
		msg_send(&msg);
		return;

	case 6:
		msg.hdr.type = htonl(HFAPP_MSG_START_AMTOR_COLFEQ);
		msg_send(&msg);
		return;

	case 7:
		msg.hdr.type = htonl(HFAPP_MSG_START_AMTOR_SELFEQ);
		msg_send(&msg);
		return;

	case 8:
		msg.hdr.type = htonl(HFAPP_MSG_START_RTTY_RX);
		msg_send(&msg);
		return;

	case 9:
		msg.hdr.type = htonl(HFAPP_MSG_START_RTTY_TX);
		msg_send(&msg);
		return;

	default:
		errprintf(SEV_INFO, "menu item %d clicked\n", item);
		return;
	}
}

/* --------------------------------------------------------------------- */

void cb_mixerparams(FL_OBJECT *ob, long data)
{
	struct hfapp_msg msg;

	if (data == 0) {
		msg.hdr.type = htonl(HFAPP_MSG_SET_MIXERPAR);
		msg.hdr.err = htonl(ERR_NOERR);
		msg.hdr.len = htonl(sizeof(msg.data.ppar));
		msg.data.mpar.igain = htonl((unsigned int)(255 * fl_get_slider_value(fd_mixer_params->igain)));
		msg.data.mpar.ogain = htonl((unsigned int)(255 * fl_get_slider_value(fd_mixer_params->ogain)));
		if (fl_get_button(fd_mixer_params->insrc_mic))
			msg.data.mpar.src = htonl(1);
		else if (fl_get_button(fd_mixer_params->insrc_aux))
			msg.data.mpar.src = htonl(2);
		else 
			msg.data.mpar.src = htonl(0);
		msg_send(&msg);
	}
	fl_hide_form(fd_mixer_params->mixer_params);
}

/* --------------------------------------------------------------------- */

void cb_fskparams(FL_OBJECT *ob, long data)
{
	if (data == 0)
		set_fsk_freq(strtoul(fl_get_input(fd_fsk_params->freqmark), NULL, 0),
			     strtoul(fl_get_input(fd_fsk_params->freqspace), NULL, 0));
	else
		set_fsk_freq(fsk_freq[1], fsk_freq[0]);
	fl_hide_form(fd_fsk_params->fsk_params);
}

/* --------------------------------------------------------------------- */

void cb_pactorparams(FL_OBJECT *ob, long data)
{
	struct hfapp_msg msg;

	if (data == 0) {
		msg.hdr.type = htonl(HFAPP_MSG_SET_PACTORPAR);
		msg.hdr.err = htonl(ERR_NOERR);
		msg.hdr.len = htonl(sizeof(msg.data.ppar));
		strncpy(msg.data.ppar.destcall, fl_get_input(fd_pactor_params->destcall), sizeof(msg.data.ppar.destcall));
		strncpy(msg.data.ppar.mycall, fl_get_input(fd_pactor_params->mycall), sizeof(msg.data.ppar.mycall));
		msg.data.ppar.txdelay = htons(strtoul(fl_get_input(fd_pactor_params->txdelay), NULL, 0));
		msg.data.ppar.retry = htons(strtoul(fl_get_input(fd_pactor_params->retry), NULL, 0));
		msg.data.ppar.longpath = fl_get_button(fd_pactor_params->longpath);
		msg.data.ppar.crcpreset[0] = htons(strtoul(fl_get_input(fd_pactor_params->crc100chg), NULL, 16));
		msg.data.ppar.crcpreset[1] = htons(strtoul(fl_get_input(fd_pactor_params->crc100), NULL, 16));
		msg.data.ppar.crcpreset[2] = htons(strtoul(fl_get_input(fd_pactor_params->crc200chg), NULL, 16));
		msg.data.ppar.crcpreset[3] = htons(strtoul(fl_get_input(fd_pactor_params->crc200), NULL, 16));
		msg_send(&msg);
	}
	fl_hide_form(fd_pactor_params->pactor_params);
}

/* --------------------------------------------------------------------- */

void cb_gtorparams(FL_OBJECT *ob, long data)
{
	struct hfapp_msg msg;

	if (data == 0) {
		msg.hdr.type = htonl(HFAPP_MSG_SET_GTORPAR);
		msg.hdr.err = htonl(ERR_NOERR);
		msg.hdr.len = htonl(sizeof(msg.data.gpar));
		strncpy(msg.data.gpar.destcall, fl_get_input(fd_gtor_params->destcall), sizeof(msg.data.gpar.destcall));
		strncpy(msg.data.gpar.mycall, fl_get_input(fd_gtor_params->mycall), sizeof(msg.data.gpar.mycall));
		msg.data.gpar.txdelay = htons(strtoul(fl_get_input(fd_gtor_params->txdelay), NULL, 0));
		msg.data.gpar.retry = htons(strtoul(fl_get_input(fd_gtor_params->retry), NULL, 0));
		msg_send(&msg);
	}
	fl_hide_form(fd_gtor_params->gtor_params);
}

/* --------------------------------------------------------------------- */

void cb_amtorparams(FL_OBJECT *ob, long data)
{
	struct hfapp_msg msg;

	if (data == 0) {
		msg.hdr.type = htonl(HFAPP_MSG_SET_AMTORPAR);
		msg.hdr.err = htonl(ERR_NOERR);
		msg.hdr.len = htonl(sizeof(msg.data.apar));
		strncpy(msg.data.apar.destcall, fl_get_input(fd_amtor_params->destcall), sizeof(msg.data.apar.destcall));
		strncpy(msg.data.apar.selfeccall, fl_get_input(fd_amtor_params->selfeccall), sizeof(msg.data.apar.selfeccall));
		strncpy(msg.data.apar.mycall, fl_get_input(fd_amtor_params->mycall), sizeof(msg.data.apar.mycall));
		msg.data.apar.txdelay = htons(strtoul(fl_get_input(fd_amtor_params->txdelay), NULL, 0));
		msg.data.apar.retry = htons(strtoul(fl_get_input(fd_amtor_params->retry), NULL, 0));
		msg.data.apar.rxinvert = fl_get_button(fd_amtor_params->inv);
		msg.data.apar.txinvert = fl_get_button(fd_amtor_params->rxtxinv);
		if (msg.data.apar.rxinvert)
			msg.data.apar.txinvert = !msg.data.apar.txinvert;
		msg_send(&msg);
	}
	fl_hide_form(fd_amtor_params->amtor_params);
}

/* --------------------------------------------------------------------- */

void cb_rttyparams(FL_OBJECT *ob, long data)
{
	struct hfapp_msg msg;

	if (data == 0) {
		msg.hdr.type = htonl(HFAPP_MSG_SET_RTTYPAR);
		msg.hdr.err = htonl(ERR_NOERR);
		msg.hdr.len = htonl(sizeof(msg.data.rpar));
		msg.data.rpar.baud = htons(strtoul(fl_get_input(fd_rtty_params->baud), NULL, 0));
		msg.data.rpar.rxinvert = fl_get_button(fd_rtty_params->inv);
		msg.data.rpar.txinvert = fl_get_button(fd_rtty_params->rxtxinv);
		if (msg.data.rpar.rxinvert)
			msg.data.rpar.txinvert = !msg.data.rpar.txinvert;
		msg_send(&msg);
	}
	fl_hide_form(fd_rtty_params->rtty_params);
}

/* --------------------------------------------------------------------- */

void write_term(int term, unsigned char *data, int datalen)
{
	int i;

	if (term < 0 || term >= sizeof(termfds)/sizeof(termfds[0]))
		errprintf(SEV_FATAL, "write_term: term out of range\n");
	if (termfds[term] < 0) {
		errprintf(SEV_WARNING, "write_term: terminal %d closed\n", term);
		return;
	}
	while (datalen > 0) {
		i = write(termfds[term], data, datalen);
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

static void mainloop(void)
{
	fd_set rmask, wmask;
	int i, maxfd, fdx;
	struct timeval tm;

	for (;;) {
		maxfd = fdx = ConnectionNumber(fl_get_display());
		FD_ZERO(&rmask);
		FD_ZERO(&wmask);
		FD_SET(fdx, &rmask);
		if (fd_krnl >= 0) {
			FD_SET(fd_krnl, &rmask);
			if (fd_krnl > maxfd)
				maxfd = fd_krnl;
		}
		for (i = 0; i < sizeof(termfds)/sizeof(termfds[0]); i++) {
			if (termfds[i] >= 0) {
				FD_SET(termfds[i], &rmask);
				if (termfds[i] > maxfd)
					maxfd = termfds[i];
			}
		}
		tm.tv_sec = 0;
		tm.tv_usec = 100000;
		if ((i = select(maxfd+1, &rmask, &wmask, NULL, &tm)) < 0)
			errstr(SEV_FATAL, "select");
		/*if (FD_ISSET(fdx, &rmask))*/
			fl_check_forms();
		if (fd_krnl >= 0 && FD_ISSET(fd_krnl, &rmask)) 
			msg_process(fd_krnl);
		for (i = 0; i < sizeof(termfds)/sizeof(termfds[0]); i++)
			if (termfds[i] >= 0 && FD_ISSET(termfds[i], &rmask))
				term_process(i, termfds[i]);
	}
}

/* --------------------------------------------------------------------- */

static void init(void)
{
	static const char *titles[3] = { "HFTerm Input", "HFTerm Output",
					 "HFTerm Monitor" };
	int i;
	struct sockaddr_un saddr;
	char buf[16];
	char *bp1, *bp2;

	if ((fd_krnl = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		errstr(SEV_FATAL, "socket");
	saddr.sun_family = AF_UNIX;
	strncpy(saddr.sun_path, name_kernel, sizeof(saddr.sun_path));
	if (connect(fd_krnl, (struct sockaddr *)&saddr, sizeof(saddr))) {
		errstr(SEV_WARNING, "connect");
		close(fd_krnl);
		fd_krnl = -1;
	}
	if (atexit(cleanup))
		errprintf(SEV_FATAL, "cannot register atexit handler\n");
	signal(SIGCHLD, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTERM, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        for (i = 0; i < sizeof(termfds)/sizeof(termfds[0]); i++)
                fork_xterm(termfds+i, termpids+i, titles[i], NULL);
        signal(SIGCHLD, sig_cleanup);
        signal(SIGQUIT, sig_cleanup);
        signal(SIGTERM, sig_cleanup);
        signal(SIGHUP, sig_cleanup);
        signal(SIGINT, sig_cleanup);
	
	fd_menubar = create_form_menubar();
	fd_spec = create_form_spec();
	fd_gtor_params = create_form_gtor_params();
	fd_pactor_params = create_form_pactor_params();
	fd_amtor_params = create_form_amtor_params();
	fd_rtty_params = create_form_rtty_params();
	fd_fsk_params = create_form_fsk_params();
	fd_mixer_params = create_form_mixer_params();

	fl_show_form(fd_menubar->menubar, FL_PLACE_CENTER, FL_FULLBORDER, 
		     "HF Terminal");
	fl_set_menu(fd_menubar->file, "Quit");
	fl_set_menu_item_shortcut(fd_menubar->file, 1, "Qq#Q#q");
	fl_set_menu(fd_menubar->util, "Frequency spectrum%r0|FSK Parameters|GTOR Parameters|Pactor Parameters|"
		    "Amtor Parameters|RTTY Parameters|Mixer Parameters");
	fl_set_menu(fd_menubar->state, "Become IRS|Become ISS|QRT|Speedup%l|Uppercase|Lowercase|Figurecase");
	fl_set_menu(fd_menubar->mode, "Standby%l|Pactor ARQ|Pactor FEC%l|Gtor ARQ%l|Amtor ARQ|"
		    "Amtor Collective FEC|Amtor Selective FEC%l|RTTY Receive|RTTY Transmit");
      
	fl_set_menu_item_mode(fd_menubar->util, 1, FL_PUP_BOX);

	fl_set_input(fd_gtor_params->mycall, MYCALL);
	fl_set_input(fd_gtor_params->txdelay, "30");
	fl_set_input(fd_gtor_params->retry, "30");
	fl_set_input_maxchars(fd_gtor_params->destcall, 10);
	fl_set_input_maxchars(fd_gtor_params->mycall, 10);
	fl_set_input_maxchars(fd_gtor_params->txdelay, 3);
	fl_set_input_maxchars(fd_gtor_params->retry, 3);

	fl_set_input(fd_pactor_params->mycall, MYCALL);
	fl_set_input(fd_pactor_params->txdelay, "30");
	fl_set_input(fd_pactor_params->retry, "30");
	fl_set_input(fd_pactor_params->crc100chg, PCT_CRC_0);
	fl_set_input(fd_pactor_params->crc100, PCT_CRC_1);
	fl_set_input(fd_pactor_params->crc200chg, PCT_CRC_2);
	fl_set_input(fd_pactor_params->crc200, PCT_CRC_3);
	fl_set_input_maxchars(fd_pactor_params->destcall, 8);
	fl_set_input_maxchars(fd_pactor_params->mycall, 8);
	fl_set_input_maxchars(fd_pactor_params->txdelay, 3);
	fl_set_input_maxchars(fd_pactor_params->retry, 3);
	fl_set_input_maxchars(fd_pactor_params->crc100, 4);
	fl_set_input_maxchars(fd_pactor_params->crc100chg, 4);
	fl_set_input_maxchars(fd_pactor_params->crc200, 4);
	fl_set_input_maxchars(fd_pactor_params->crc200chg, 4);

	
	strncpy(buf+8, MYCALL, sizeof(buf)-8);
	bp2 = buf+7+strlen(buf+8);
	buf[sizeof(buf)-1] = 0;
	bp1 = buf+4;
	*bp1 = 0;
	while (bp2 >= buf+8) {
		if (isalpha(*bp2))
			*--bp1 = *bp2;
		bp2--;
		if (bp1 <= buf)
			bp1 = buf+1;
	}
	fl_set_input(fd_amtor_params->mycall, buf);
	fl_set_input(fd_amtor_params->selfeccall, buf);
	fl_set_input(fd_amtor_params->txdelay, "30");
	fl_set_input(fd_amtor_params->retry, "30");
	fl_set_input_maxchars(fd_amtor_params->destcall, 4);
	fl_set_input_maxchars(fd_amtor_params->mycall, 4);
	fl_set_input_maxchars(fd_amtor_params->selfeccall, 4);
	fl_set_input_maxchars(fd_amtor_params->txdelay, 3);
	fl_set_input_maxchars(fd_amtor_params->retry, 3);

	fl_set_input(fd_rtty_params->baud, "45");
	fl_set_input_maxchars(fd_rtty_params->baud, 3);

	fl_set_input_maxchars(fd_fsk_params->freqspace, 4);
	fl_set_input_maxchars(fd_fsk_params->freqmark, 4);
	set_fsk_freq(FREQ_MARK, FREQ_SPACE);
}

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
        int c;
        int err = 0;

	fl_initialize(&argc, argv, 0, 0, 0);
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
 	init();
	mainloop();
	return 0;
}

/* --------------------------------------------------------------------- */
