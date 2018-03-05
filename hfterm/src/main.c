/*****************************************************************************/

/*
 *      main.c  --  simple HF terminal program.
 *
 *      Copyright (C) 1996  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *        Swiss Federal Institute of Technology (ETH), Electronics Lab
 *	modified 2000-2004 by Axel Krause & Gnther Montag (dl4mge@darc.de).
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

#include "hft.h"

#ifndef MAXKEY
#define MAXKEY 2048
#endif

#ifndef MAXRX
#define MAXRX 2000
#endif

/* --------------------------------------------------------------------- */

char versioninfo[128];
char kernelstartcommand[128];
int rxfile_ready = 0, scope_on = 0, log_on = 0;
int kernelstart = 1;
static int fd_krnl = -1;
static char *name_kernel = "/var/run/hfapp";
char gmt[32];
GdkFont *radiofont = NULL;
struct par params;

/* --------------------------------------------------------------------- */
/*
 * Logging functions
 */

void errprintf(int severity, const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
	display_status( "hfterm[%lu]: ", (unsigned long)getpid());
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

/* --------------------------------------------------------------- */
void display_status(const char *fmt, ... )
{
	GtkText *txt = NULL;
	unsigned int len;
        va_list args;
	char display[256];
	
	if (!fmt || !(len = strlen(fmt)))
		return;
	
	memset (display, 0, sizeof(display));
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textstatus"));
	// to prevent stack overflow while running loooooong
	while (gtk_text_get_length(txt) > MAXMON)
	{
	 gtk_text_freeze(txt);
	  gtk_text_set_point (txt, 0);
	  gtk_text_forward_delete (txt, MAXMON / 2);
	  gtk_text_set_point (txt, gtk_text_get_length(txt));
	 gtk_text_thaw(txt);
	}
        va_start(args, fmt);
	vsnprintf(display, sizeof(display), fmt, args);	
        va_end(args);
	gtk_text_insert(txt, radiofont, NULL, NULL, display, strlen(display));
	gtk_text_insert(txt, NULL, NULL, NULL, "\n", 2);
}

/* ------------------------------------------------------------------ */

void rx_window_keep_small()
{
	GtkText *txt = NULL;
	int length;

	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textmain"));
	length = gtk_text_get_length(txt);
	// to prevent crashes from stack overflow with growing rx text
	while  (length > MAXRX) {
	  if (! rxfile_ready ) rx_routine_store_prepare();
	  gtk_text_freeze(txt);
	  rx_routine_store_part();
	  display_status("first half of rx text saved in ~/hf/hfrx.");
	  gtk_text_set_point (txt, 0);
	  gtk_text_forward_delete (txt, MAXRX / 2  ); // removed "-10"
	  length = gtk_text_get_length(txt);
	  gtk_text_set_point (txt, length);
	  gtk_text_thaw(txt);
	}
}

void write_input(unsigned char *data, int datalen)
{
	GtkText *txt;
	GtkEditable *entry;
	gint pos;
	int i, factor;
	static int squelchcount = 0;
	int squelchwait;
	
	if (!data && datalen <= 0) return;

	/* squelch. The squelch_passed... flags are updated in spectrum.c 
	 * squelch works only if the spectrum is on! 
	 */
	
	// no squelch for mt63, as no spectrum working
	if (lastrxmsg == HFAPP_MSG_STATE_MT63_RX) {
	  if (scope_on) {
/*
	      if (!squelch_passed_mt36_500_1000) {
		if (params.mt63.doubleinterleave) factor = 2;
		else factor = 1;
		squelchwait = (params.mt63.integration * factor) ; 
		squelchcount++;
		//display_status("MT63 squelch waits for %d units", squelchwait);
		//display_status("MT63 squelch count %d units", squelchcount);
		if (squelchcount > squelchwait) {
		//	display_status(
		//	"MT63 signal remained weak for %d units -> squelched",
		//	squelchwait);
	       return;
		}

	      } else squelchcount = 0;
*/	      
	  }
	}

	/* for cw mode under construction */
	else if ((lastrxmsg == HFAPP_MSG_STATE_CW_RX) &&
	    ( ! squelch_passed_cw)) {
	  if (scope_on) {
/*	  
	    squelchwait = 10;
	    squelchcount++;
	    if (squelchcount > squelchwait ) {
		display_status("cw signal remained weak for %3.1f s -> squelched",
		params.mt63.integration * factor);
		return;
	    }
*/	    
	  }
	}

	/* for all other modes, which are the fsk modes */
	else if (scope_on) {
	    if (!squelch_passed_fsk) {
		squelchwait = 0;
		squelchcount++;
		//display_status("FSK squelch waits for %d units", squelchwait);
		//display_status("FSK squelch count %d units", squelchcount);
		if (squelchcount > squelchwait) {
		    /*
		    display_status(
			"FSK signal remained weak for %d units -> squelched",
			squelchwait);
		    */
		    display_status("FSK signal weak since %d units -> squelched",	squelchwait);
		    return;
		}
	    } else squelchcount = 0;
	}
	
	fbbtest = 0;
	if (mailbox_on == 1) mailbox_input(data, datalen);
	rx_window_keep_small();
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textmain"));

/*
the other squelch flags 
extern int squelch_passed_mt36_1000_1500;
extern int squelch_passed_mt36_1500_2500;
could in future be processed here to feed mt63_bandwidth_autodetect() !
*/

/* carriage return handling : this i tried: 
 * removes the ugly hook by which the radiofont expresses a 
 * carriage-return. 
 */

/* works for rtty  and amtor : */
/*
	if (data[datalen - 1] == '\r') {
	    //display_status ("datalen is %d, last is carriage return", datalen);
	    data[datalen - 1] = ' '; 
	}
*/
/* works for pactor : */ 
/*
	if (data[datalen - 2] == '\r') {
	    //display_status ("datalen is %d, before last is carriage return", datalen);
	    data[datalen - 2] = ' ';
	}
*/
/* orig by Tom 
	gtk_text_insert(txt, radiofont, NULL, NULL, data, datalen);
*/
	for (i = 0; i < datalen; i++) {
	    if (data[i] == '\b') {
	    	entry = GTK_EDITABLE
		    (gtk_object_get_data(GTK_OBJECT(wmain), "textmain"));
		pos = gtk_editable_get_position(entry);
		if (pos > 0) {
		    gtk_editable_delete_text(entry, pos-1, pos);
		}
	    } 
	    else {
		if (data[i] == '\r') {
		    // data[i] = ' '; 
		    // but this removes newlines wheen text processed by
		    // dos/windows
		    gtk_text_insert(txt, NULL, NULL, NULL, data+i, 1); 
		    /* removes the ugly hook by which the radiofont expresses a 
		    carriage-return. */
		} else
	    	gtk_text_insert(txt, radiofont, NULL, NULL, data+i, 1);
	    }
	}
}

void write_output(unsigned char *data, int datalen)
{
	int i;
	GtkText *txt;
	GtkEditable *entry;
	gint pos;
	GdkColor txfg, txbg;

	txfg.pixel = 0;
	txfg.red = 65535;	
	txfg.green = 0;
	txfg.blue = 65535;
	/* pink */

	txbg.pixel = 0;
	txbg.red = 55000;
	txbg.green = 65535;
	txbg.blue = 55000;
	/* light green */
	
	if (!data && datalen <= 0) {
	    display_status ("hfterm/src/main.c: no data, len 0");
	    return;
	}
	if (!data ) {
	    display_status ("hfterm/src/main.c: no data");
	    return;
	}
	if ( datalen <= 0) {
	    display_status ("hfterm/src/main.c: len 0");
	    return;
	}
	fbbtest = 0;
	rx_window_keep_small();
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textmain"));

	if (radiofont == NULL) {
	    if ((radiofont = gdk_font_load("7x13")) == NULL)
		display_status("simple font 7x13 could not be loaded.");
	}

	    /*    
	    newline only at a line:
	    pactor: datalen = 2
	    all other: datalen = 1
	    
	    5 letters and newline in a line:
	    pactor: datalen = 7
	    all other: datalen = 1
	    */

	/*  with the 7x13 font 
	    the carriage return (\r) is printed as a hook, not looking nice!
	*/
	    /* works for rtty  and amtor !! */
/*
	if (data[datalen - 1] == '\r') {
	    //display_status ("datalen is %d, last is carriage return", datalen);
	    data[datalen - 1] = ' '; 
	}
*/
	    /* works for pactor !! */ 
/*
	if (data[datalen - 2] == '\r') {
	    //display_status ("datalen is %d, before last is carriage return", datalen);
	    data[datalen - 2] = ' ';
	}
*/
	    /* orig by Tom, modified by Gnther */
/*
	gtk_text_insert(txt, radiofont, 
//    &(GTK_WIDGET(txt)->style->fg[GTK_WIDGET_STATE(GTK_WIDGET(txt))]), 
	&txfg, &txbg, data, datalen);
*/
	for (i = 0; i < datalen; i++) {
	    if (data[i] == '\b') {
	    	entry = GTK_EDITABLE
		    (gtk_object_get_data(GTK_OBJECT(wmain), "textmain"));
		pos = gtk_editable_get_position(entry);
		if (pos > 0) {
		    gtk_editable_delete_text(entry, pos-1, pos);
		}
	    } 
	    else {
		if (data[i] == '\r') {
		    data[i] = ' ';
		    /* removes the ugly hook by which the radiofont expresses a 
		    carriage-return. */
		}
	    	gtk_text_insert(txt, radiofont, &txfg, &txbg, data+i, 1);
	    }
	}
}

void start_write_mailboxtest()
{
/* 
 * before start of mailbox-test
 * by writing to input window, 
 * set a color and 
 * go to end of input text 
 * and write a newline
 * 
 */
        GtkText *txt;
	int length;
	GdkColor boxtestfg, boxtestbg;

    	boxtestbg.pixel = 0;
	boxtestbg.red = 65535;	
	boxtestbg.green = 65535;
	boxtestbg.blue = 50000;
	/* light yellow */

	boxtestfg.pixel = 0;
	boxtestfg.red = 0;
	boxtestfg.green = 0;
	boxtestfg.blue = 65535;
	/* should be blue */
	
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textmain"));
	length = gtk_text_get_length(txt);
	gtk_text_set_point (txt, length);
	gtk_text_insert(txt, radiofont, 
//    &(GTK_WIDGET(txt)->style->fg[GTK_WIDGET_STATE(GTK_WIDGET(txt))]), 
        &boxtestfg, &boxtestbg, "\n", 1);
	rx_window_keep_small();
}

void write_monitor(unsigned char *data, int datalen)
{
	GtkText *txt = NULL;
	if (!data && datalen <= 0)
		return;
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmonitor), 
	    "textmonitor"));
	if (!GTK_WIDGET_DRAWABLE(GTK_WIDGET(txt)))
		return;
	while (gtk_text_get_length(txt) > MAXMON)
	{
	 // gtk_text_freeze(txt);
	  gtk_text_set_point (txt, 0);
	  gtk_text_forward_delete (txt, MAXMON / 2);
	  gtk_text_set_point (txt, gtk_text_get_length(txt));
	 // gtk_text_thaw(txt);
	}
	gtk_text_insert(txt, radiofont, NULL, NULL, data, datalen);
}

/* ------------------------------------------------------------------ */

void write_kernel(unsigned char *data, int datalen)
{
	int i;
	int failmax = 100;
	static int failcount = 0;

	if (fd_krnl < 0) {
	    failcount++;
	    //display_status ("hfterm: can not write to hfkernel %d.time", failcount);
	    return;
	}
	while (datalen > 0) {
		i = write(fd_krnl, data, datalen);
		if (i < 0) {
		    if (errno == EAGAIN) {
			failcount++;
			if (failcount > failmax) {
			    errstr(SEV_FATAL, "write hfterm -> hfapp");
			} 
		    }
		    if (errno != EAGAIN)
			errstr(SEV_FATAL, "write hfterm -> hfapp");
		} else {
		    failcount = 0;	
		    data += i;
		    datalen -= i;
		}
	}
}

/* ------------------------------------------------------------------ */

//static 
void edit_newline(void)
{
        struct hfapp_msg msg;
//	GtkEntry *entry;
//	see below
//	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wmain), "textedit"));
//	gtk_entry_set_text(entry, "");

	msg.data.b[0] = '\r';
	msg.data.b[1] = '\n';
        msg.hdr.type = htonl(HFAPP_MSG_DATA_TRANSMIT);
        msg.hdr.len = htonl(2);
        msg.hdr.err = htonl(ERR_NOERR);
        msg_send(&msg);
}

//static 
void edit_backspace(void)
{
        struct hfapp_msg msg;
	GtkEditable *entry;
	gint pos;

	entry = GTK_EDITABLE(gtk_object_get_data(GTK_OBJECT(wmain), "textedit"));
	pos = gtk_editable_get_position(entry);
	if (pos > 0) {
//		gtk_editable_set_position(entry, pos-1);
		gtk_editable_delete_text(entry, pos, pos);
	}
	msg.data.b[0] = '\b';
        msg.hdr.type = htonl(HFAPP_MSG_DATA_TRANSMIT);
        msg.hdr.len = htonl(1);
        msg.hdr.err = htonl(ERR_NOERR);
        msg_send(&msg);
}

//static 
void edit_addchar(char v)
{
        struct hfapp_msg msg;
	char buf[2];
	buf[0] = v;
	buf[1] = 0;
	msg.data.b[0] = v;
        msg.hdr.type = htonl(HFAPP_MSG_DATA_TRANSMIT);
        msg.hdr.len = htonl(1);
        msg.hdr.err = htonl(ERR_NOERR);
        msg_send(&msg);
}

/* --------------------------------------------------------------------- */

//static 
gboolean poll_prepare(gpointer source_data, GTimeVal *current_time, gint *timeout, gpointer user_data)
{
	*timeout = -1;
        return FALSE;
}

//static 
gboolean poll_check(gpointer source_data, GTimeVal *current_time, gpointer user_data)
{
    static int poll_check_count = 0;
    int poll_check_max = 1000;
	if (msgpfd.revents & G_IO_IN) {
	    poll_check_count = 0;
	    return TRUE;
	}
	poll_check_count++;
	if (poll_check_count > poll_check_max) {
	    errstr(SEV_FATAL, 
		"hfterm: connection to hfkernel disturbed. Exiting.");
	}
        return FALSE;
}

//static 
gboolean poll_dispatch(gpointer source_data, GTimeVal *current_time, gpointer user_data)
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

void timequery()
{
	/*Datum-Systemabfrage*/
	time_t now;
	time(&now);
	// "GENORMT":
	//sprintf(gmt, "%s", asctime(gmtime(&now)));	
	// This i like better:
	// strftime(gmt, 31, "%a %d.%m.%y, %H:%M", gmtime(&now));
	// but to makie it compatible to cabrillo:
	strftime(gmt, 31, "%Y-%m-%d %H%M", gmtime(&now));
} 

//static 
void init(void)
{
	struct sockaddr_un saddr;
	int kernel_running = 0;
	char systemstring[256];
	
	system("cd ~"); 
	system("pwd"); 
		sprintf(systemstring, 
	 	"if ! [ -d ~/hf ] ; then mkdir ~/hf; cp -ruv %s/share/hf/hf-examplefiles/* ~/hf; fi", PREFIX);
	system(systemstring);
	
	sprintf(versioninfo,  "hf %s. All Docs in: %s/share/hf", 
	    PACKAGE_VERSION, PREFIX);
	display_status(versioninfo);
	
	param_set_defaults();
	param_read();	
	fixtext_read();
	log_read();
	log_list();
	param_set();
	logbook_window_show(NULL, NULL	);
	if ((radiofont = gdk_font_load("7x13")) == NULL)
	    display_status("simple font 7x13 could not be loaded.");
		
	if(kernelstart) {
	    sprintf(kernelstartcommand, 
		"hfkernel -s %f -t %f -m %f -a %s -p %s %s &",
		 params.general.snd_corr, 
		 params.general.tv_corr, 
		 params.general.cpu_mhz, 
		 params.general.soundcard, 
		 params.general.serial, 
		 params.general.kerneloptions);
	
		printf(" the hfkernel start command is: \n %s\n", 		kernelstartcommand);
	
	// I kill all hfkernel threads first to go sure no zombies around there	
	    system("hfkernel -k");
	    
	// I start hfkernel new
	    display_status("I start hfkernel now....");
	    system(kernelstartcommand);
	    display_status("I will wait a second....");
	    sleep(1);
	    display_status("I will try to connect...");
	}
	
	// I connect the kernel
	if ((fd_krnl = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		errstr(SEV_FATAL, "socket");
	saddr.sun_family = AF_UNIX;
	strncpy(saddr.sun_path, name_kernel, sizeof(saddr.sun_path));
	if (connect(fd_krnl, (struct sockaddr *)&saddr, sizeof(saddr)))
	{
	    errstr(SEV_WARNING, "connect");
	    close(fd_krnl);
	    msgpfd.fd = fd_krnl = -1;
	    if (kernelstart) {
		display_status
		    ("I could not start hfkernel.\n"
		    "Please try to start it on a console first.\n" 
		    "Write down the options you need, \n"
		    "enter them into the configure menu.\n"
		    "See F1 or /usr/share<doc/>(packages/)/hf/HF-HOWTO for help.\n"); 
	    } else {
	        display_status
		    ("By option -n you did not want to start hfkernel.\n"
		    "It also seems not to have been running before.\n" 
		    "This mode is for debug only.\n" );
	    }
	} else {
		// if the connect is o.k.
		
		g_source_add(G_PRIORITY_HIGH, FALSE, 
		    &poll_funcs, NULL, NULL, NULL);
		msgpfd.fd = fd_krnl;
		msgpfd.events = G_IO_IN;
		msgpfd.revents = 0;
		g_main_add_poll(&msgpfd, G_PRIORITY_HIGH);
		kernel_running = 1;
		display_status
		    ("hfterm has connected hfkernel All is good.\n" );
	}
	    	
	if(kernel_running) {
	    param_kernel();
	
	/*
	    msg.hdr.err = htonl(ERR_NOERR);
	    msg.hdr.len = htonl(0);
	    msg.hdr.type = htonl(HFAPP_MSG_START_STANDBY);
	    msg_send(&msg);
	    display_status("start with STANDBY FOR PACTOR-AMTOR- GTOR...");
	    lastrxcmd = HFAPP_MSG_START_STANDBY;
	    way = RX;
	*/
	}

}

/* ------------------------------------------------------------------ */

void finit(void)
{
	/*is called from callbacks.c */
	errprintf (SEV_INFO,"hfterm: 73 & hpe cuagn sn !! \n");        
	display_status("\n73 !!! by hfterm !");        
	gtk_main_quit();
/* - Rx-Text-Rest speichern ------------------------------------------*/
	rx_routine_store_rest();
/* - Fix - Text speichern --------------------------------------------*/
	fixtext_store();
/* - Config-Datei speichern ------------------------------------------*/
	param_store();
/* - Log-Datei speichern ---------------------------------------------*/
	log_store();
/* - finish hfkernel     ---------------------------------------------*/
	system ("hfkernel -k");
}


/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
        int c, err = 0;
//	compiler:  "statement with no effect"
//	bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
//	textdomain(PACKAGE);
//		
	gtk_set_locale();
	gtk_init(&argc, &argv);
	
        while ((c = getopt(argc, argv, "nc:h:p:")) != -1) 
                switch (c) {
                case 'n':
                	kernelstart = 0;
			break;

		case 'c':
                	if (optarg != NULL) name_kernel = optarg;
                        break;

                case 'h':
                        sprintf(params.mailbox.host, "%s", optarg);
                        break;

                case 'p':
                        params.mailbox.port = atoi(optarg);
                        break;

                default:
                        err++;
                        break;
                }
        if (err) {
                errprintf 
		    (SEV_WARNING,
		    "usage: hfterm [n] [-c <socket>] [-p <port>] [-h <host>]\n"
		    " -n: no start of hfkernel (debug, graphical interface only)\n"
		    " -c: socket to hfkernel, default: /var/run/hfapp\n"
		    " -h: ip-adress of mailbox-host, default: 127.0.0.1\n"
		    " -p  port of mailbox programm, default: 6300\n");
                exit(1);
        }

#if MAP 
//  same would be:
//  add_pixmap_directory("/usr/local/share/hfterm/pixmaps");
//  this is outcommented because it will hfterm on my old box slooow  !!

	add_pixmap_directory(PACKAGE_DATA_DIR "/pixmaps");
	add_pixmap_directory(PACKAGE_SOURCE_DIR "/pixmaps");
#endif
	/*
	 * The following code was added by Glade to create one of each component
	 * (except popup menus), just so that you see something after building
	 * the project. Delete any components that you don't want shown initially.
	 */
//	wrxfileselection = create_wrxfileselection();
	wmain = create_wmain();
	wspec = create_wspec();
	wpar = create_wpar();
	wabout = create_wabout();
	whilfe = create_whilfe();
	Wfixtext = create_Wfixtext();
	Wsearchlogentr = create_Wsearchlogentr();
	wlistalllog = create_wlistalllog();
	wqsoeditor = create_wqsoeditor();
//	whinweis = create_whinweis();
//	wmap = create_wmap();	
	wmonitor = create_wmonitor();
	gtk_widget_show(wmain);

	init();
	gtk_main();
	exit(0);
}

/* --------------------------------------------------------------------- */
