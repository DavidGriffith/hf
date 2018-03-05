#include "hft.h"
#include "gui.h"
#include "callbacks.h"

extern GtkWidget *textmain;
extern guint squelch;

int fbbtest = 0;
int way = TX, beacon_file_prepared = 0, mailbox_prepared = 0;

int elbugstate;
#define PAUSE 0
#define DIT 1
#define DAH 2
#define TUNE 3

/* --- File menu ----------------------------------------------------- */ 

void on_tx_file_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	file_send();
}

void on_beacon_file_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (beacon_was_on) {
	    display_status
		("Beacon was suspended during a connect, \n"
		"you can not switch it on now manually.");
	    beacon_on = 0;
	    return;
	}
	if (beacon_on == 0) {
	    if ((beacon_file_prepared == 0) && (fixbeacon == 0)) {
		/* 
		 * if no file and no fixtext prepared for beacon:
		 * prepare a file.
		 * If user wants to send fixtext as beacon,
		 * he / she clicks on fixtext button.
		 */	    
		if ( beacon_file_prepare())  { // if file o.k.
		    display_status
			("Beacon file prepared, beacon running.");
	    	    beacon_file_prepared = 1;
		    beacon_on = 1;
		    beacon_send();
		    // must be started one time, from then on by loop in msg.c 
		    return;
		} else {
		    beacon_file_prepared = 0;
	    	    display_status("Beacon file could not be prepared ...");
		    beacon_on = 0;
		    return;
		}
	    }
	    else {
		/* 
		 * if file OR fixtext prepared for beacon:
		 * just switch on the beacon modus.
		 */
		beacon_on = 1;
		display_status
		 ("Beacon text already prepared, I switch beacon on.");
	    	 beacon_send(); 
		 // must be started one time, from then on by loop in msg.c 
		return;
	    }
	}
	if (beacon_on == 1) {
	    beacon_on = 0;
	    beacon_stop();	
	    display_status("Beacon mode off.");
	    return;
	}
}

/* --- Spektrum ----------------------------------------------------- */ 
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
	return;
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
//	printf("on_scope_motion_event: x %g y %g\n", event->x, event->y);
	return FALSE;
}

gboolean on_spec_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	unsigned int freq;
	static int shift = 0;
	static unsigned int freq1 = 0, freq2 = 0;
	static int clickcount = 0;
	int usershift = 0;
	guint squelchvalue;

	shift = get_freq_shift();	
	freq = ((SRATE/2) * event->x + SPECTRUM_WIDTH/2) / SPECTRUM_WIDTH;
	squelchvalue = event->y;
	
	if (shift) {
	    switch (event->button) {
		case 1:
		    set_fsk_freq(freq, freq+shift);
		    break;
		case 2:
		    set_fsk_freq(freq-shift/2, freq+shift/2);
		    break;
		/*
		case 3:
		    set_fsk_freq(freq-shift, freq);
		    break;
		*/
/* i borrow the right mouse button for squelch!
 * since now squelch-set works o.k. but 2nd button not working
 * any more for middlefreq...
 * but it is midnight almost. I stop for today. 19.jan 2005
 */
		case 3:
		    goto set_squelch;
		    break;
	    }
	return FALSE;
	}
	if (shift == 0) {
	    display_status 
    		("user-defined shift. Click 2 times to set mark and space.");
	    switch (event->button) {
		case 1:
		case 2:
		    clickcount++;
		    if (clickcount == 1) {
			freq1 = freq;
			freq = 0;
			//display_status ("First frequency: %d", freq1);
			set_fsk_freq(freq1, freq1+1);
			break;
		    }
		    if (clickcount == 2) {
			freq2 = freq;
			//display_status ("Second frequency: %d.");
			clickcount = 0;
			usershift = max(freq1, freq2) - min(freq1, freq2);
			//display_status 
			//(" Will set mark & space. Shift is %d",  usershift);
			set_fsk_freq(min(freq1, freq2), max(freq1, freq2));
			freq1 = freq2 = 0;
			usershift = 0;
			break;
	    	    }
		    break;
		case 3:
		    goto set_squelch;
		    break;
	    }
	}
//	printf("on_scope_button_press_event: x %g y %g  button 0x%04x\n", event->x, event->y, event->button);
	return FALSE;

set_squelch:
	squelch = squelchvalue;
	params.general.squelchpercent = 100 - (squelch * 100 / SPECTRUM_HEIGHT);
	display_status("spec click: set squelch to %d", params.general.squelchpercent);
    	return FALSE;
}

gboolean on_wspec_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_hide(widget);
	scope_on = 0;
	return TRUE;
}

void on_parameters_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(wpar);
}	

void on_parok_clicked(GtkButton *button, gpointer user_data)
{
	param_get();
	param_set();
	param_kernel();
	gtk_widget_hide(wpar);
	param_store();
}

void on_parcancel_clicked(GtkButton *button, gpointer user_data)
{
	param_set();
	gtk_widget_hide(wpar);
}

void on_quit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	finit();
}

void on_become_irs_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_IRS);
	msg_send(&msg);
	display_status("SLAVE");
	lastrxcmd = HFAPP_MSG_MODE_IRS;
	way = RX;
}

void on_become_iss_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_ISS);
	msg_send(&msg);
	display_status("MASTER");
	lasttxcmd = HFAPP_MSG_MODE_ISS;
	usertx();
}

void on_qrt_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_QRT);
	msg_send(&msg);
	display_status("QRT");
	lasttxcmd = HFAPP_MSG_MODE_QRT;
	beacon_restore();
}

void on_speedup_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_SPEEDUP);
	msg_send(&msg);
	display_status("SPEEDUP");
}

void on_uppercase_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_CASE_UPPER);
	msg_send(&msg);
	display_status("UPPERCASE");
}

void on_lowercase_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_CASE_LOWER);
	msg_send(&msg);
	display_status("LOWERCASE");
}

void on_figurecase_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_CASE_FIGURE);
	msg_send(&msg);
	display_status("FIGURECASE");
}

void on_mailbox_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (mailbox_on == 0) {
		display_status("Mailbox standby ...");
		mailbox_on = 1;
		autorx_on = 1; 
//		gtk_text_set_editable (GTK_TEXT (textmain), TRUE);
		display_status("You can test it: Write into rx-window !");
		return;
	}
	if (mailbox_on == 1) {
		mailbox_loggedout();
		display_status
		    ("Mailbox mode off. Restart by <Alt> M.");
		// in case of mailbox-advertising in beacon...
		if (beacon_on) {
		    beacon_on = 0;
		    display_status
		    ("I also switched the beacon off."
		    "(It might have announced the mailbox...)"
		    "You can activate the beacon again by <Alt> B.");
		}
		mailbox_on = 0;
		autorx_on = 1;
		return;
	}
}

void on_cw_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_CW_TX);
	msg_send(&msg);
	display_status("CW ELBUG");
	lastrxcmd = HFAPP_MSG_START_STANDBY;
	lastrxmsg = 0;
	lasttxcmd = HFAPP_MSG_MODE_CW_TX;
	way = TX;
}

void on_standby_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_STANDBY);
	msg_send(&msg);
	display_status("STANDBY FOR PACTOR - AMTOR - GTOR");
	lastrxcmd = HFAPP_MSG_START_STANDBY;
	/* to prevent for autotx "remembering" last rx msg from hfkernel ,
	   which may be irrelevant */
	lastrxmsg = 0;
	qrt_pending = 0;
	way = RX;
	beacon_restore();
}

void on_standby_pactor_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_PACTOR_STANDBY);
	msg_send(&msg);
	display_status("STANDBY FOR PACTOR ");
	lastrxcmd = HFAPP_MSG_START_PACTOR_STANDBY;
	/* to prevent for autotx "remembering" last rx msg from hfkernel ,
	   which may be irrelevant */
	lastrxmsg = 0;
	qrt_pending = 0;
	way = RX;
	beacon_restore();
}

void on_pactor_arq_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_PACTOR_ARQ);
	msg_send(&msg);
	display_status("PACTOR-ARQ");
	lasttxcmd = HFAPP_MSG_START_PACTOR_ARQ;
	qrt_pending = 0;	
	beacon_suspend();
}

void on_pactor_fec_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_PACTOR_FEQ);
	msg_send(&msg);
	display_status("PACTOR-FEC");
	lasttxcmd = HFAPP_MSG_START_PACTOR_FEQ;
	qrt_pending = 0;
	usertx();
}


void on_gtor_arq1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_GTOR_ARQ);
	msg_send(&msg);
	display_status("GTOR-ARQ");
	lasttxcmd = HFAPP_MSG_START_GTOR_ARQ;
	qrt_pending = 0;
	beacon_suspend();
}


void on_amtor_arq_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_AMTOR_ARQ);
	msg_send(&msg);
	display_status("AMTOR-ARQ");
	lasttxcmd = HFAPP_MSG_START_AMTOR_ARQ;
	qrt_pending = 0;
	beacon_suspend();
}

void on_amtor_collective_fec_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_AMTOR_COLFEQ);
	msg_send(&msg);
	display_status("AMTOR-COLFEC");
	lasttxcmd = HFAPP_MSG_START_AMTOR_COLFEQ;
	qrt_pending = 0;
	usertx();
}

void on_amtor_selective_fec_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_AMTOR_SELFEQ);
	msg_send(&msg);
	display_status("AMTOR-SELFEC");
	lasttxcmd = HFAPP_MSG_START_AMTOR_SELFEQ;
	qrt_pending = 0;
	usertx();
}

void on_rtty_receive_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_RTTY_RX);
	msg_send(&msg);
	display_status("RTTY-RX");
	lastrxcmd = HFAPP_MSG_START_RTTY_RX;
	qrt_pending = 0;
	way = RX;
}

void on_rtty_transmit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_RTTY_TX);
	msg_send(&msg);
	display_status("RTTY-TX");
	lasttxcmd = HFAPP_MSG_START_RTTY_TX;
	qrt_pending = 0;
	usertx();
}

void on_mt63_rx_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_MT63_RX);
	msg_send(&msg);
	display_status("MT63-RX");
	lastrxcmd = HFAPP_MSG_START_MT63_RX;
	qrt_pending = 0;
	way = RX;
}

void on_mt63_tx_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct hfapp_msg msg;

	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_START_MT63_TX);
	msg_send(&msg);
	display_status("MT63-TX");
	lasttxcmd = HFAPP_MSG_START_MT63_TX;
	qrt_pending = 0;
	usertx();
}

/* --- USER DATA -------------------------------------------------- */ 
/*
void on_brag_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	  gtk_widget_show(wpersonal);
	  brag_set();
}

void on_brag_ok_button_clicked(GtkButton *button, gpointer user_data)
{
	brag_get();
	brag_set();
	gtk_widget_hide(wpersonal);
	brag_store();
}

void on_brag_cancel_button_clicked(GtkButton *button, gpointer user_data)
{
	brag_set();
	gtk_widget_hide(wpersonal);
}

void on_brag_delete_button_clicked(GtkButton *button, gpointer user_data)
{
	brag_delete();
	brag_set();
}
*/

/* --- Fixtexte  ---------------------------------------------------- */ 
void on_Fix_button1_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(1);
}

void on_Fix_button2_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(2);
}

void on_Fix_button3_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(3);
}

void on_Fix_button4_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(4);
}

void on_Fix_button5_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(5);
}

void on_Fix_button6_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(6);
}

void on_Fix_button7_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(7);
}

void on_Fix_button8_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(8);
}

void on_Fix_button9_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(9);
}

void on_Fix_button10_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(10);
}

void on_Fix_button11_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(11);
}

void on_Fix_button12_clicked(GtkButton *button, gpointer user_data)
{
	fixtext_send(12);
}

void on_monitor_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	  gtk_widget_show(wmonitor);
}

void on_about_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(wabout);
}

/* ------------ HILFEDATEI -------------- */
void on_index1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(whilfe);
}

/* ------------	FIXTEXTE --------------  */
void on_Wfixtext_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	  gtk_widget_show(Wfixtext);
	  display_status("welcome to text macro studio.");
}

/* -- FIXTEXTE BEENDEN & SICHERN ------------------------------------ */
void on_button_Fixtext_OK_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(Wfixtext);
	fixtext_store();
}

void on_button_Fixtext_Cancel_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(Wfixtext);
}

void on_parabutton_clicked(GtkButton *button, gpointer user_data)
{
	  gtk_widget_show(wpar);
}

/* the two rx_keypress / keyrelease  functions are for 
testing the mailbox by writing into the rx window! */

gboolean on_rx_keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	static unsigned char data[128];
	static int datalen = 0;
//	char c = event->keyval;
//	display_status("on_text_keypress_event: 0x%x %c  state %u\n",
//	event->keyval, (c >= ' ' && c <= 0x7f) ? c : '.', event->state);

//	if (event->state & ~(GDK_SHIFT_MASK | GDK_LOCK_MASK))
//		return FALSE;
//	do't understand meaning of this, confuses !	

	if (event->keyval == GDK_BackSpace) {
		return TRUE;
	}
	
	if (event->keyval == GDK_Return && mailbox_on == 1 ){
    	    data[datalen] = '\r' ;   //event->keyval;
// 		F6FBB starts when it gets \r! \n does it NOT!
// 		I needed hours of debugging to get this idea ...
	    datalen++;
//	    display_status 
//		("Will send these %d bytes to mailbox for test: \n%s", 
//		datalen, data);
	    mailbox_input(data, datalen);
	    memset (data, 0, sizeof(data));
	    datalen = 0;
	    return TRUE; 
	} 
	if (event->keyval >= 32 && event->keyval < 128 && mailbox_on == 1 ) {
	    if (datalen >= sizeof(data) -1 ) {
		display_status
		    ("Too much input for mailbox test! Deleted.");
		memset (data, 0, sizeof(data));
		datalen = 0;
		return FALSE;
	    } else {
		if (! fbbtest) {	    
		/* 
		 * before start of test, set a color 
		 * go to end of input text 
		 * and write a newline
		 * 
		 */
		    fbbtest = 1;
		    start_write_mailboxtest();
		}
	        data[datalen] = event->keyval;
		datalen++;
//	        display_status ("will send this to mailbox for test: %s", data);
//		mailbox_rx_2_inbuf(data, datalen);
//		memset (data, 0, sizeof(data));
//		datalen = 0;
		return TRUE;
	    }
	}
	return FALSE;
}

gboolean on_rx_keyrelease_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
//	char c = event->keyval;
//	printf("on_text_keyrelease_event: 0x%x %c  state %u\n", 
//	event->keyval, (c >= ' ' && c <= 0x7f) ? c : '.', event->state);
	return TRUE;
}

/* for transmit by keyboard */
gboolean on_tx_keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	int c = event->keyval;
//	display_status("tx keypress: hex 0x%x is char %c",
//	    c, (c >= 32 && c < 128) ? c : '.');
		
//	if (event->state & ~(GDK_SHIFT_MASK | GDK_LOCK_MASK)) {
//              display_status("tx keypress: Shift or NumLock set. Do nothing.");
//		return FALSE;
//	}
// don't understand meaning... if numlock on no 
// autotx-on-write... why?

	if (c == GDK_BackSpace) {
		if( way == RX) autotx();
		edit_backspace();
		return TRUE;
	}
	if (c == GDK_Return) {
		if( way == RX) autotx();
		edit_newline();
		return TRUE;
	}
	if (c >= 32 && c < 128) {
	    	if( way == RX) {
		    autotx();
		}
		edit_addchar(event->keyval);
		return TRUE;
	}
	return FALSE;
}

gboolean on_tx_keyrelease_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
//	char c = event->keyval;
//	printf("on_text_keyrelease_event: 0x%x %c  state %u\n", 
//	event->keyval, (c >= ' ' && c <= 0x7f) ? c : '.', event->state);
	return TRUE;
}

/* for cw elbug */
gboolean on_textedit_button_press_event
    (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	struct hfapp_msg msg;
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);

	if (lasttxcmd != HFAPP_MSG_MODE_CW_TX) return FALSE;
	
	switch (event->button) {
	case 3: /* dit */
		msg.hdr.type = htonl(HFAPP_MSG_CW_ELBUG_DIT);
		elbugstate = DIT;
		//display_status("cw dit");
		break;
	case 2: /* tune */ /* UNUSED */
		msg.hdr.type = htonl(HFAPP_MSG_CW_ELBUG_TUNE);
		elbugstate = TUNE;
		//display_status("cw tune / tone ...");
		break;
 	case 1: /* dah */
		msg.hdr.type = htonl(HFAPP_MSG_CW_ELBUG_DAH);
		elbugstate = DAH;
		//display_status("cw dah");
		break;
	}
 	msg_send(&msg);
	return TRUE;
}

gboolean on_textedit_button_release_event
    (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	int validpause = 0;
	struct hfapp_msg msg;
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_CASE_UPPER);

	switch (event->button) {
	case 3: /* dit */
		if (elbugstate == DIT) {
		    validpause = 1;
		}
		break;
	case 2: /* tune */
		validpause = 1;
		break;
	case 1: /* dah */
		if (elbugstate == DAH) {
		    validpause = 1;
		}
		break;
	}
	/* not every release sends a message.
	 * Imagine: i press dit, then dah just a ms before releasing dit,
	 * then i release the dit key. This should have no effect.
	 */	
	if (validpause) {
	    msg.hdr.type = htonl(HFAPP_MSG_CW_ELBUG_PAUSE);
	    msg_send(&msg);
	}
	return TRUE;
}

gboolean on_wmain_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	finit();
	return TRUE;
}

gboolean on_wmain_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	finit();
	return TRUE;
}

void on_aboutok_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(wabout);
}

/* -------------- HILFEDATEI BEENDEN -------------------------------- */
void on_button17_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(whilfe);
}

/* -------------- HINWEIS BEENDEN -------------- */
void on_button22_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(whinweis);
}
/* -------------- HINWEIS BEENDEN --------------- */

/* ----------------- ZEIGEN DER WELTKARTE  ----------------------- */
void on_map1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(wmap);
}

/* -------------- MAP BEENDEN ---------------- */
void on_enough_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(wmap);
}


