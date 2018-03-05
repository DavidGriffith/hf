/*****************************************************************************/

/*
 *      rxtx.c  --  routines for file selection,receiving, storing, sending
 *	and text macro management
 * 	for the simple HF terminal program.
 *
 *      Copyright (C) 1996  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *        Swiss Federal Institute of Technology (ETH), Electronics Lab
 *	modified 2000-2004 by Axel Krause & Günther Montag (dl4mge@darc.de).
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
#include "rxtx.h"

#ifndef DEBUG
#define DEBUG printf("%s: function %s still running at line %d...\n", \
__FILE__, __FUNCTION__,  __LINE__);

#define D DEBUG
#endif /* DEBUG */

/* --------------------------------------------------------------------- */

int autorx_on = 0;
int beacon_on = 0, beacon_was_on = 0, beaconcount = 0, fixbeacon = 0;
int lastrxcmd = 0, lastrxmsg = 0, lasttxcmd = 0, lasttxmsg = 0;
int qrt_pending = 0, arq;
char specialfile[256], rxfilehead[256], rxfilefoot[256]; 
char beaconbuffer[MAXKEY], keytext[MAXKEY];
const char *fixmagic = 
"	* * * * * * *	YOUR MAGIC KEYS: 	* * * * * * * \n"
"[B]	will transmit the text as BEACON. (Stop it with <Alt>b.)\n"
"[MYCALL] [OP] [QTH] [LOC] [RIG] [PWR] [ANT] [WWW]\n"
"	will be replaced by your entries from 'Personal Edit'.\n"
"[AMYCALL] (Amtor), \n"
"	is your own Amtor short-call, taken from 'Parameters'.\n"
"[TIME]	will print a timestamp.\n"
"[ACALL] (Amtor), [PCALL] = [CALL] (Pactor + RTTY), [GCALL] (Gtor)\n"
"	is your QSO-Partner's call, taken from 'Parameters'.\n"
"[LTIME] [LCALL] [LNAME] [LQTH] [LRSTIN] [LRSTOUT] [LNOTES] [LMODE] [LBAND]\n"
"will be replaced by the corresponding field of your last log entry.\n"
"You can use 'Strg-X, Strg-C, Strg-V to cut, copy and insert.\n"
"To use the fixtexts, press <Shift-F><number> or click the right buttons.\n"
"		* * * *  Have a lot of fun!  * * * * ";
FILE *rxfile;

/*
[PMYCALL] = [CALL] (Pactor + RTTY), [GMYCALL] (Gtor)
*/

/* --------------------------------------------------------------- */

void wait(int us) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = us;
    select (0, NULL, NULL, NULL, &tv);
}

/* --------------------------------------------------------------- */

void transmit(const char *fmt, ... )
{
// transmits text, e.g. to be used for automatic messages in mailbox mode...
	unsigned int len;
        va_list args;
	char text[256];
	char v;
	int l;
	GtkText *txt;
		
	if (!fmt || !(len = strlen(fmt)))
	    return;

	memset (text, 0, sizeof(text));
        va_start(args, fmt);
	vsnprintf(text, sizeof(text), fmt, args);	
        va_end(args);

	autotx();
	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textedit"));
	gtk_text_insert(txt, radiofont, NULL, NULL, text, strlen(text));
//	display_status("transmitting text %s", text);
	edit_newline();
	for(l = 0; l < strlen(text); l++) {
	    v = text[l];
	    edit_addchar(v);
	}	
	edit_newline();
	return;				
}

/* --------------------------------------------------------------- */

void create_rxfilehead()
{
    timequery();
    rxfilehead[0] = 0;
    sprintf(rxfilehead, "\n* * * * Start of received data by hfterm * * * *\n"
	"OP: %s \nGMT: %s \n\n", params.pactor.mycall, gmt);
    //display_status( "rx file head prepared: %s \n", rxfilehead);
}

/* --------------------------------------------------------------- */

void create_rxfilefoot()
{
    timequery();
    rxfilefoot[0] = 0;
    sprintf(rxfilefoot, "\n\n* * * * End of received data by hfterm * * * *\n"
	"OP: %s \nGMT: %s \n", params.pactor.mycall, gmt);
    //display_status( "rx file foot prepared: %s \n", rxfilehead);
}

/* --------------------------------------------------------------- */

void textinsert(char *filename, GtkText *text)
  {
  	FILE *fi;
  	char buf[16];
  	static int radiofont_loaded = 0;
	
	if (! radiofont_loaded ) {
	    if ((radiofont = gdk_font_load("7x13")) == NULL)
		display_status("simple font 7x13 could not be loaded.");
  	    else radiofont_loaded = 1;
	}
	fi = fopen(filename, "r");
  	if(fi == NULL)
  	  {
  	    display_status( "file %s can not be opened.\n", filename);
	    return;
	  }
  	while ( !feof(fi) )
	  {
            memset(buf, 0, sizeof(buf));
	    fgets(buf, 15, fi);
	    gtk_text_insert(text, radiofont, NULL, NULL, buf, -1);
	 }
  	fclose(fi);				
 }

/* --------------------------------------------------------------- */

void parse(char* magic, char* bragitem)
{
/* parses fixtexts for macros. Uncomment fprintfs to see ho it 
   works, its greaaat, from scratch by dl4mge -----------------------*/
// if you change this, change char fixmagic, see above
    if(strstr(keytext, magic) != 0)
    {
	char a[2048];
	char *c;
	while(strstr(keytext, magic) != 0)
	{
	  memset(a, 0, sizeof(a));
	  c =  strstr(keytext, magic);
//	  display_status( "\n\na anfangs\n%s", a); 
//	  display_status( "\ngefunden: %d\n", c - keytext); 
	  strncpy(a, keytext, (c - keytext));
//	  display_status( "\n\na nach 1. Teil\n%s", a); 
	  strcat(a, bragitem);
//	  display_status( "\n\na nach brag\n%s", a); 
	  strcat(a, c + strlen(magic));
//	  display_status( "\n\na nach 2. Teil\n%s", a);
	  strcpy(keytext, a);
//	  printf("\nkeytext:\n%s", keytext);
	}
    }
    return;
}

/* --------------------------------------------------------------- */

void fixtext_send(int fixnr)
{
/* calls parse function for call, rig, ant etc. 
   Also if the fixtext is to be transmitted as beacon, 
   it has to be parsed before every transmit,
   because it may contain a timestamp ! -------------------------------------*/
	int i = fixnr, l, fixlength;
	char v, *fixtext, fixtextname[8];
	GtkText *txt;
		
	sprintf(fixtextname, "text%hu", i);
	fixlength = gtk_text_get_length
	    (GTK_TEXT(gtk_object_get_data
		(GTK_OBJECT(Wfixtext), fixtextname)));
	fixtext = gtk_editable_get_chars
	    (GTK_EDITABLE(gtk_object_get_data
		(GTK_OBJECT(Wfixtext), fixtextname)), 0, fixlength);
		
	if (fixlength > 0)
	{
	  timequery();
	  strcpy(keytext, fixtext);
	  parse("[CALL]", params.pactor.destcall);
          parse("[PCALL]", params.pactor.destcall);
	  parse("[PMYCALL]", params.pactor.mycall);
          parse("[ACALL]", params.amtor.destcall);
	  parse("[AMYCALL]", params.amtor.mycall);
          parse("[GCALL]", params.gtor.destcall);
	  parse("[GMYCALL]", params.gtor.mycall);
          parse("[MYCALL]", params.brag.call);
          parse("[OP]", params.brag.op);
          parse("[QTH]", params.brag.qth);
          parse("[LOC]", params.brag.loc);
          parse("[RIG]", params.brag.rig);
          parse("[PWR]", params.brag.pwr);
          parse("[ANT]", params.brag.ant);
          parse("[WWW]", params.brag.www);
          parse("[TIME]", gmt);
          if (lb.logsize != 0) {
          	parse ("[LTIME]", lb.line[lb.logsize].time);
          	parse ("[LCALL]", lb.line[lb.logsize].call);
          	parse ("[LNAME]", lb.line[lb.logsize].name);
          	parse ("[LQTH]", lb.line[lb.logsize].qth);
          	parse ("[LRSTIN]", lb.line[lb.logsize].rstin);
          	parse ("[LRSTOUT]", lb.line[lb.logsize].rstout);
          	parse ("[LNOTES]", lb.line[lb.logsize].notes);
          	parse ("[LMODE]", lb.line[lb.logsize].mode);
          	parse ("[LBAND]", lb.line[lb.logsize].band);
          	display_status("I replaced magic keys [L...] from log entry Nr. %d", lb.logsize);
          }
          else {
/*
          	display_status
		    ("did not parse magic keys from macro concerning log,\n"
          	    "because no log entry was specified");
*/
          }
	  if ( strstr(keytext, "[B]") ) {
	      display_status("[B] magic key in fixtext -> beacon");
              parse("[B]", " ");
	      fixbeacon = i;
	      beacon_file_prepared = 0;
	      // fixtext beacon beats file beacon
	      beacon_on = 1;
	      autotx();     
	      autorx_on = 1; 
	  } else 
	  autotx();
/*
//	no use now, autotx / rx is now default
	  if ( strstr(keytext, "[TX]") ) {
	      display_status("[TX] magic key in fixtext -> autotx");
              parse("[TX]", " ");
	      autotx();
	  }
	  if ( strstr(keytext, "[RX]") ) {
	      autorx_on = 1;
	      parse("[RX]", " ");
	      display_status("[RX] magic key in fixtext -> autorx after tx.");
	  }
*/
	  txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textedit"));
	  gtk_text_insert(txt, radiofont, NULL, NULL, keytext, strlen(keytext));

	  display_status("transmitting fixtext %hu",  i);
	  edit_newline();
	  for(l = 0; l < strlen(keytext); l++)
	  {
	    	v = keytext[l];
	    	edit_addchar(v);
	  }	
	  edit_newline();
	}
	return;				
}

/* --------------------------------------------------------------- */

void fixtext_read()
{
/* - 12 Fix - Texte einlesen ----------------------------------------*/
	FILE *fix;
	int i; 
	char fixfilename[32], fixtextname[8], buf[16];

	static int radiofont_loaded = 0;
	if (! radiofont_loaded ) {
	    if ((radiofont = gdk_font_load("7x13")) == NULL)
		display_status("simple font 7x13 could not be loaded.");
  	    else radiofont_loaded = 1;
	}

	for (i = 1; i < 13; i++) {
	    sprintf(fixfilename, "hf/fix.%02hu", i);
	    sprintf(fixtextname, "text%hu", i);
	    fix = fopen(fixfilename, "r");
	    if(fix == NULL) {
		display_status( "fixtextfile %s can not be opened.\n", fixfilename);
		break;
	    }
	    while ( !feof(fix) ) {
		memset(buf, 0, sizeof(buf));
		fgets(buf, 15, fix);
		gtk_text_insert 
		    ((GTK_TEXT(gtk_object_get_data
		    (GTK_OBJECT(Wfixtext), fixtextname))) , 
		    radiofont, NULL, NULL, buf, -1);
	    }
	    fclose(fix);				
	}
}	
  
/* --------------------------------------------------------------- */

void fixtext_store()
{
/* - 12 Fix - Texte speichern ---------------------------------------*/
	FILE *fix;
	int i, fixlength;
	char *fixtext, fixfilename[32], fixtextname[8];

	for (i = 1; i < 13; i++)
	{
  	  sprintf(fixfilename, "hf/fix.%02hu", i);
	  sprintf(fixtextname, "text%hu", i);
	  fix = fopen(fixfilename, "w");
	  if(fix == NULL)
	  {
	    display_status( "fixtextfile %s can not be opened.\n", fixfilename);
	    return;
	  }
	  fixlength = gtk_text_get_length(GTK_TEXT(gtk_object_get_data(GTK_OBJECT(Wfixtext), fixtextname)));
	  fixtext = gtk_editable_get_chars(GTK_EDITABLE(gtk_object_get_data(GTK_OBJECT(Wfixtext), fixtextname)), 0, fixlength);
	  if (fixlength > 0)
	  {
	    if ( (fwrite(fixtext, fixlength, 1, fix)) != 1)
	    {
	      display_status("Error in writing fixtextfile %s\n", fixfilename);
	      return;
	    }  
	    fclose(fix);				
	  }
	}
}

/* --------------------------------------------------------------- */

void file_send()
{
/* Datei senden, ohne Parserfunktion -------------------------------------------------*/
	FILE *txfile;
	char c = 0;
	GtkText *txt;

	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textedit"));
	fileselection_dialog("choose file for transmit ...");
	txfile = fopen(specialfile, "r");
	if(txfile == NULL) {
	    display_status("transmit file %s can not be opened.", specialfile);
	    return;
	}
	display_status("starting transmission of file %s ...", specialfile);
	edit_newline();
	while (1) {
	    c = fgetc(txfile);      
	    if (!feof(txfile)) {
	    	gtk_text_insert(txt, radiofont, NULL, NULL, &c, 1);
    	    edit_addchar(c);
    	    }
    	    else break;
	}	
	edit_newline();
	fclose (txfile);
	display_status("transmission of file %s complete.", specialfile);
	return;				
}

/* --------------------------------------------------------------- */

void beacon_suspend() {
	if (beacon_on == 1) {
	    beacon_on = 0;
	    beacon_was_on = 1;
	    display_status 
	        ("Beacon is suspended during time of connect.");
	}
}

/* --------------------------------------------------------------- */

void beacon_restore() {
	if (beacon_was_on == 1) {
	    beacon_on = 1;
	    beacon_was_on = 0;
	    display_status 
	        ("Beacon was running before -> I start it again.\n"
		"You can stop the beacon by <Alt>B.");
	}
}

/* --------------------------------------------------------------- */

int beacon_file_prepare()
{
/* prepare beacon with fileselection dialog ----------------------------------------------*/
	FILE *beaconfile;
	char c = 0;
	int buffercount = 0;

	memset(beaconbuffer, 0, sizeof(beaconbuffer));
	fileselection_dialog
	    ("select small (<512 characters) file for beacon text ...");
	beaconfile = fopen(specialfile, "r");
	if(beaconfile == NULL) {
	    display_status("beacon file %s can not be opened.", specialfile);
	    return 0; // error
	}
	display_status
	    ("will read (part of) beaconfile %s into buffer", specialfile);
	//display_status("empty buffer contains: \n%s", beaconbuffer);
	
	while (buffercount < sizeof(beaconbuffer) - 1) {
	    c = fgetc(beaconfile);
    	    if (!feof(beaconfile)) {
		beaconbuffer[buffercount] = c;
		buffercount++;
	    }
	    else break;
    	}
	fclose(beaconfile);				
	//display_status
	//    ("have read (part of) beaconfile %s into buffer", specialfile);
	//display_status("buffer contains now: \n%s", beaconbuffer);
	display_status("Will start beacon tx from file %s ...\n"
		"You can stop the beacon with <Alt>b .", specialfile);
	return 1;
}			

/* --------------------------------------------------------------- */

void beacon_fixtext_prepare()
{
/* prepare beacon with fixtext ----------------------------------------------*/
/* not used, found better way */
	memset(beaconbuffer, 0, sizeof(beaconbuffer));
	strcpy(beaconbuffer, keytext);
	display_status
	    ("copied parsed fixtext into beaconbuffer.\n"
	    "Starting beacon transmission of fixtext.\n"
	    "You can stop the beacon with <Alt>b.");
	beacon_on = 1;
	autorx_on = 1;
//	beacon_send();
}			

/* --------------------------------------------------------------- */

void beacon_send() {
	GtkText *txt = NULL;			
	int l;
	char v = 0;

    /* 	bugtracking, 
	beacon_suspend() while mailbox connect did not work */
	if (beacon_on == 0) {
	    display_status("wrong call of beacon_send(),\n"
		"beacon off at the moment");
	    return;
	}
	if (beacon_was_on == 1) {
	    display_status("wrong call of beacon_send(),\n"
		"beacon is suspended off at the moment");
	    beacon_on = 0;
	    return;
	}
	
	if (fixbeacon) {
	// a fixtext with the [B] magic key is already chosen as beacon text
	    autorx_on = 1;
	    autotx();
	    fixtext_send(fixbeacon);
	}
	else if (beaconbuffer[0]) {
	    // a file selected before has been read into beaconbuffer
	    txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), "textedit"));
	    gtk_text_insert 
		(txt, radiofont, NULL, NULL, beaconbuffer, strlen(beaconbuffer));
	    autorx_on = 1;
	    autotx();
	    for(l = 0; l < strlen(beaconbuffer); l++) {
		v = beaconbuffer[l];
		edit_addchar(v);
	    } 
	}
	else {
	    display_status("no beacon text prepared!\n"
		"You can select a fixtext containing the [B] key as beacon,\n"
		"or you can select a file with <Alt>B.");
	    beacon_on = 0;
	    return;
	}	
	autorx_on = 1;
	beaconcount++;
	display_status("Beacon count: %d ...", beaconcount);
	if (beaconcount % 10 == 0) {
	    display_status("HF is running as beacon.\n"
		"You can stop the beacon with <Alt>b !");
	}
}

/* --------------------------------------------------------------- */

void beacon_stop() {
	autorx();
	display_status("Beacon stopped after %d transmissions.", beaconcount);
	beaconcount = 0;
	fixbeacon = 0; 
	// resets nr. of fixtext last sent as beacon
	// so next time beacon is again the first selected file
	return;				
}

/* --------------------------------------------------------------- */

void rx_routine_store_prepare()
{
/* - Rx - Text vorbereiten   --------------------------------------------*/
	//if you want many rxfiles:
	//sprintf(rxfilename, "hf/hfrx.%hu", getpid());
	//or maybe instead of getpid do something with timestamp / gmt
	char *rxfilename =  "hf/hfrx";

	if (rxfile_ready) return;

	remove("hf/hfrx.old.01");
  	rename("hf/hfrx.old.02", "hf/hfrx.old.01");	  
  	rename("hf/hfrx.old.03", "hf/hfrx.old.02");
	rename("hf/hfrx.old.04", "hf/hfrx.old.03");	  
  	rename("hf/hfrx.old.05", "hf/hfrx.old.04");
	rename("hf/hfrx", "hf/hfrx.old.05");	  
	create_rxfilehead();
	rxfile = fopen(rxfilename, "w");
	if(rxfile == NULL) {
	    display_status("rx-file %s can not be opened first time.\n", rxfilename);
	    return;
	}
	if ( (fwrite(rxfilehead, strlen(rxfilehead), 1, rxfile)) != 1) {
    	    display_status
		("Error in writing head lines to new routine rx-file %s\n", 
		rxfilename);
	    errprintf(SEV_WARNING,
		"Error in writing head lines to new routine rx-file %s\n", 
		rxfilename);
        return;
	}
	fclose(rxfile);
	// open rxfile for append for duration of whole program
	// hope so quicker and prevent crashes in pactor rx when lot of rx
	rxfile = fopen(rxfilename, "a");
	if(rxfile == NULL)
	  {
	    display_status("rx-file %s can not be opened.\n", rxfilename);
	    errprintf(SEV_WARNING,
		"Error in opening routine rx-file %s for appending data\n", 
		rxfilename);
	    return;
	  }
	display_status("rx-file %s is opened for rx data\n", rxfilename);
	rxfile_ready = 1;
}

/* --------------------------------------------------------------- */

void rx_routine_store_part()
{
/* - Rx - Text bei laufendem Programm teilweise (hier: halb) speichern ------*/
	char *rxfilename =  "hf/hfrx";
	char *rxtxt;
	int rxlength = 0;
	rxlength = gtk_text_get_length(GTK_TEXT
	    (gtk_object_get_data(GTK_OBJECT(wmain), 
	    "textmain")));
	rxtxt = gtk_editable_get_chars(GTK_EDITABLE
	    (gtk_object_get_data(GTK_OBJECT(wmain), 
	    "textmain")), 0, rxlength);
	if ( (fwrite(rxtxt, MAXRX / 2 -1, 1, rxfile)) != 1)
	  {
	    display_status
		("Error in writing to rx-file %s\n", rxfilename);
	    errprintf(SEV_WARNING, 
		"Error in writing to rx-file %s\n", rxfilename);
	    return;
	  }
//	fwrite("\n", strlen("\n"), 1, rxfile);
//	fclose(rxfile);	
//	display_status("Older half of Rx Data stored in ~/hf/hfrx\n");
	return;
}

/* --------------------------------------------------------------- */

void rx_routine_store_rest()
{
/* - Rx - Text vor Beenden speichern ---------------------------------------*/
	char *rxfilename =  "hf/hfrx";
	char *rxtxt;
	GtkText *txt;
	int rxlength, i;

	txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain),"textmain"));
	rxlength = gtk_text_get_length(GTK_TEXT
	    (gtk_object_get_data(GTK_OBJECT(wmain), 
	    "textmain")));

	if (rxlength > 0)
	{
	    if (! rxfile_ready) {
		//display_status("rx data file not yet prepared. Will do it...");
		//errprintf
		//    (SEV_INFO, "rx data file not yet prepared. Will do it...\n");
		rx_routine_store_prepare();
		//display_status("... done.");
		//errprintf
		//    (SEV_INFO, "... done.\n");
		
	    }
	    gtk_text_freeze(txt);
	    rxtxt = gtk_editable_get_chars(GTK_EDITABLE
		(gtk_object_get_data(GTK_OBJECT(wmain), 
	    	"textmain")), 0, rxlength);
	    if ( (i = fwrite(rxtxt, rxlength -1, 1, rxfile)) != 1) {
		//display_status
		//    ("Error in final writing to rx-file %s", rxfilename);
		errprintf
		    (SEV_WARNING, "Error in final writing to rx-file %s\n", 
		    rxfilename);
		return;
	    }
	    //display_status
	//	("final writing to rx-file %s: rxlength %d, written %d bytes", 
	//	rxfilename, rxlength, i);
	    create_rxfilefoot();	
	    fwrite(rxfilefoot, strlen(rxfilefoot), 1, rxfile);
	    fclose(rxfile);	
	    //display_status( "Rest of Rx Data stored in %s\n", rxfilename);
	    //errprintf
	    //	(SEV_INFO, "Rest of Rx Data stored in rx-file %s\n", 
	    //	    rxfilename);
	    return;
	} else {
	    //errprintf
	    //	(SEV_INFO,  "You had no rx - you get no rx-file.\n");
	    return;
	}  
}

/* --------------------------------------------------------------- */

/* - fileselection universal---------------------------------------------- */

void fileselection_dialog(char *title)
{
	GtkWidget *filedialog;
//there should be defined global in this file a char specialfile[256]; 
//long array because maybe with path.
	memset(specialfile, 0, sizeof(specialfile));	
//	display_status("specialfilereset  o.k.: %s\n", specialfile);
	filedialog = gtk_file_selection_new(title);
	gtk_window_set_modal(GTK_WINDOW(filedialog), TRUE);
		gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filedialog)->ok_button),
	    "clicked", (GtkSignalFunc) fileselection, filedialog);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filedialog)->cancel_button),
	    "clicked", (GtkSignalFunc) fileselection_cancel, filedialog);
	gtk_signal_connect(GTK_OBJECT(filedialog),
	    "destroy", (GtkSignalFunc) fileselection_cancel, filedialog);
//	display_status( "widget filedialog created.\n");
//	display_status("widget filedialog created.\n");
	gtk_widget_show(filedialog);
//	display_status("after showing file selection menu \n");
//	display_status( "after showing file selection menu \n");
	gtk_main();
//	display_status("after gtk-main \n");
//	display_status( "after gtk-main \n");
//	display_status( "specialfile still o.k.: %s\n", specialfile);
//	display_status("specialfile still o.k.: %s\n", specialfile);
}

/* --------------------------------------------------------------- */

void fileselection(GtkWidget *widget, GtkWidget * filedialog)
{
	char *selectedname = NULL;
		
	selectedname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(filedialog));
//	display_status( "fileselection o.k.: %s\n", selectedname);
	if ((strlen(selectedname) > 0) && (strlen(selectedname) < 64))  {
	    display_status("filename length between 1 and 64, o.k.: %s", selectedname);
	    sleep (1);
	    if (selectedname[strlen(selectedname) -1] == 47) {
	 	//sleep(1);   
		display_status("You selected %s, it's a directory! Try again!", 
		    selectedname);
	    }
	    else {
	        //display_status("You selected %s, o.k.", selectedname);
	        strcpy(specialfile, selectedname); 
	        display_status("You selected %s, o.k.", specialfile);
	    }
	}
	gtk_widget_destroy(filedialog);
   	gtk_main_quit();
}

void fileselection_cancel(GtkWidget *widget, GtkWidget * filedialog)
{
    gtk_widget_destroy(filedialog);
    gtk_main_quit();
}

/* - fileselection ------------------------------------------------------- */

/* --------------------------------------------------------------- */

void rx_demand_store()
{
/* - Dateiauswahlmenü zum Speichern des Rx-Testes -------------------------*/
    GtkText *txt;
    FILE *rxfile;
    char *rxtxt = NULL;
    int rxlength = 0;
    
    txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain),"textmain"));
    rxlength = gtk_text_get_length(GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), 
	"textmain")));
    if (rxlength == 0) {
    	// this no no stuff for test of display_status.....
	display_status("You had %s rx - you can get %s rx-file.", "no", "no");
    	return;
    }
    else {
    	display_status("You received something ... choose a file...");
	fileselection_dialog("Save rx input as file ...");
    }
    if (specialfile[0] == 0) {
    	display_status(	"rxfilename %s not yet specified.\n", specialfile);
        return;
    }
    display_status( "rx-file %s has been prepared.\n", specialfile);
    display_status("rx-file %s has been prepared.", specialfile);
    rxfile = fopen(specialfile, "w");
    if (rxfile == NULL){
        display_status(	"rx-file %s can not be opened.\n", specialfile);
        return;
    }
    gtk_text_freeze(txt);
    rxlength = gtk_text_get_length(GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), 
	"textmain")));
    rxtxt = gtk_editable_get_chars(GTK_EDITABLE(gtk_object_get_data(GTK_OBJECT(wmain), 
	"textmain")), 0, rxlength);
    create_rxfilehead();
    if ( (fwrite(rxfilehead, strlen(rxfilehead), 1, rxfile)) != 1) {
        display_status(	
	    "Error in writing head lines to choosen rx-file %s\n", 
	    specialfile);
        return;
    }
    if ( (fwrite(rxtxt, rxlength, 1, rxfile)) != 1) {
        display_status(	
	    "Error in writing rx-text to choosen rx-file %s\n", 
	    specialfile);
        return;
    }
    create_rxfilefoot();
    if ( fwrite(rxfilefoot, strlen(rxfilefoot), 1, rxfile)!= 1) {
        display_status(	
	    "Error in writing rx-text to choosen rx-file %s\n", 
	    specialfile);
        return;
    }
    fclose(rxfile);	
    gtk_text_thaw(txt);
    display_status("rx-file %s has been written.", specialfile);
    display_status( "Rx Data stored in %s\n", specialfile);    
    return;
}	

/* --------------------------------------------------------------- */

void usertx() 
{
	// user-tx beats (switches off) the autorx() function.
	// display_status("tx on demand -> no auto rx.");
	// errprintf((SEV_INFO, "tx on demand -> resetting auto rx and tx");
	way = TX;
	beacon_suspend();
	autorx_on = 0;
}

/* --------------------------------------------------------------- */

void autotx()
{
/*
 * automatic transmit function;
 * used from callbacks.c: on_tx_keypres_event
 * and by the fixtext & mailbox functions
 */
	struct hfapp_msg msg;
	int fittingtxcmd = 0;
	if (way == TX) {
	    //display_status ("autotx: way is tx already."); 
	    return;
	}

	switch (lastrxmsg) {

    	    case HFAPP_MSG_STATE_PACTOR_STANDBY:
		fittingtxcmd = HFAPP_MSG_START_PACTOR_FEQ;
		display_status ("autotx: Pactor Standby -> Pactor FEC"); 
		break;

	    case HFAPP_MSG_STATE_PACTOR_IRS:
		fittingtxcmd = HFAPP_MSG_MODE_ISS;
		display_status ("autotx: Pactor IRS -> ISS"); 
		break;

	    case HFAPP_MSG_STATE_AMTOR_IRS:
		fittingtxcmd = HFAPP_MSG_MODE_ISS;
		display_status ("autotx: Amtor IRS -> ISS"); 
		break;

	    case HFAPP_MSG_STATE_GTOR_IRS:
		fittingtxcmd = HFAPP_MSG_MODE_ISS;
		display_status ("autotx: Gtor IRS -> ISS"); 
		break;

	    case HFAPP_MSG_STATE_PACTOR_ARQ_SLAVECONNECT:
		fittingtxcmd = HFAPP_MSG_MODE_ISS;
		display_status ("autotx: Pactor Slaveconnect -> ISS"); 
		break;

	    case HFAPP_MSG_STATE_AMTOR_ARQ_SLAVECONNECT:
		fittingtxcmd = HFAPP_MSG_MODE_ISS;
		display_status ("autotx: Amtor Slaveconnect -> ISS"); 
		break;

	    case HFAPP_MSG_STATE_GTOR_ARQ_SLAVECONNECT:
		fittingtxcmd = HFAPP_MSG_MODE_ISS;
		display_status ("autotx: Gtor Slaveconnect -> ISS"); 
		break;

	    case HFAPP_MSG_STATE_AMTOR_SELFEC_RX:	
		fittingtxcmd = HFAPP_MSG_MODE_ISS; 
		display_status ("autotx: Amtor Selfec RX -> ISS"); 
		break;

	    case HFAPP_MSG_STATE_AMTOR_COLFEC_RX:
		fittingtxcmd = HFAPP_MSG_START_AMTOR_COLFEQ;
		display_status ("autotx: Amtor Colfec RX -> Amtor colfec TX"); 
		break;

	    case HFAPP_MSG_STATE_RTTY_RX:
		fittingtxcmd = HFAPP_MSG_START_RTTY_TX;
		display_status ("autotx: RTTY RX -> TX"); 
		break;

	    case HFAPP_MSG_STATE_MT63_RX:
		fittingtxcmd = HFAPP_MSG_START_MT63_TX;
		display_status ("autotx: MT63 RX -> TX"); 
		break;

	    default:
		//not every command results in a message from hfkernel.
		//display_status 
		//    ("autotx: last RX msg not clear. Checking last TX cmd."); 

		switch (lasttxcmd) {

		    case HFAPP_MSG_START_PACTOR_ARQ:
			fittingtxcmd = HFAPP_MSG_START_PACTOR_FEQ;
			display_status ("autotx: Standby -> Pactor FEC"); 
			break;

		    case HFAPP_MSG_START_PACTOR_FEQ:
			fittingtxcmd = HFAPP_MSG_START_PACTOR_FEQ;
			display_status ("autotx: Standby -> Pactor FEC"); 
			break;

		    case HFAPP_MSG_START_GTOR_ARQ:
			fittingtxcmd = HFAPP_MSG_START_PACTOR_FEQ;
			display_status ("autotx: Standby -> Pactor FEC"); 
			break;

		    case HFAPP_MSG_START_AMTOR_ARQ:  
			fittingtxcmd = HFAPP_MSG_START_AMTOR_SELFEQ;
			display_status ("autotx: Standby -> Amtor Selfec"); 
			break;
	    		
		    case HFAPP_MSG_START_AMTOR_COLFEQ:  
			fittingtxcmd = HFAPP_MSG_START_AMTOR_COLFEQ;
			display_status ("autotx: Standby -> Amtor Colfec"); 
			break;
	    		
		    case HFAPP_MSG_START_AMTOR_SELFEQ: 
			fittingtxcmd = HFAPP_MSG_START_AMTOR_SELFEQ;
			display_status ("autotx: Standby -> Amtor Selfec"); 
			break;
	    
		    default:
		    // here unclear what to do -> do what user did last 	
			display_status("autotx: Please select a tx mode.");
			//fittingtxcmd = HFAPP_MSG_START_PACTOR_FEQ;
			//display_status ("-> guess and switch to Pactor FEC..."); 
			return;
		}
		break;
	}
	
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(fittingtxcmd);
	msg_send(&msg);
// 	display_status("autotx(): sending msg for mode %d", fittingtxcmd);
	lasttxcmd = fittingtxcmd;
	wait(3000);
	autorx_on = 1;
	way = TX;
	return;
}

/* --------------------------------------------------------------- */

void autorx()
{
	struct hfapp_msg msg;
	int fittingrxcmd;

	if (way	== RX) {
	    //display_status("autorx(): way is rx already.");
	    return;
	}
	if (autorx_on == 0) {
	    //display_status ("autorx_on = 0. No autorx."); 
	    return;
	} 
	if (arq) {
	    //display_status 
		//("ARQ mode running. It will handle tx->rx better. No autorx."); 
	    return;
	} 
	//display_status ("autorx_on = 1. Preparing autorx function."); 
	//errprintf(SEV_INFO, "autorx_on = 1. Preparing autorx function."); 
//	display_status 
//	    ("autorx: Checking last tx msg..."); 
	
        switch (lasttxmsg) {

	    case HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT:
		// Disconnect msgs are treated as tx msgs
		// so that autorx works and switches to standby !
		fittingrxcmd = HFAPP_MSG_START_STANDBY;
		qrt_pending = 0;
		display_status 
		    ("autorx: Pactor ARQ Disconnect -> Standby"); 
		break;

	    case HFAPP_MSG_STATE_AMTOR_ARQ_DISCONNECT:
		// Disconnect msgs are treated as tx msgs
		// so that autorx works and switches to standby !
		fittingrxcmd = HFAPP_MSG_START_STANDBY;
		qrt_pending = 0;
		display_status 
		    ("autorx: Amtor ARQ Disconnect -> Standby"); 
		break;

	    case HFAPP_MSG_STATE_AMTOR_FEC_DISCONNECT:
		// Disconnect msgs are treated as tx msgs
		// so that autorx works and switches to standby !
		fittingrxcmd = HFAPP_MSG_START_STANDBY;
		qrt_pending = 0;
		display_status 
		    ("autorx: Amtor FEC Disconnect -> Standby"); 
		break;

	    case HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT:
		// Disconnect msgs are treated as tx msgs
		// so that autorx works and switches to standby !
		fittingrxcmd = HFAPP_MSG_START_STANDBY;
		qrt_pending = 0;
		display_status 
		    ("autorx: Gtor ARQ Disconnect -> Standby"); 
		break;

	    case HFAPP_MSG_STATE_PACTOR_FEC_CONNECT:
		// bad name, just means fec tx
		fittingrxcmd = HFAPP_MSG_START_STANDBY;
		// was before: Pactor only.
		// But: if mailbox-beacon in p-fec running,
		// still Amtor Connect for mailbox shall be possible.
		display_status 
		    ("autorx: Pactor FEC -> Standby"); 
		break;

	    case HFAPP_MSG_STATE_PACTOR_ISS:
		fittingrxcmd = HFAPP_MSG_MODE_IRS;
		display_status ("autorx: Pactor ISS -> IRS"); 
		break;

	    case HFAPP_MSG_STATE_AMTOR_ISS:
		fittingrxcmd = HFAPP_MSG_MODE_IRS;
		display_status ("autorx: Amtor ISS -> IRS"); 
		break;

	    case HFAPP_MSG_STATE_GTOR_ISS:
		fittingrxcmd = HFAPP_MSG_MODE_IRS;
		display_status ("autorx: Gtor ISS -> IRS"); 
		break;

	    case HFAPP_MSG_STATE_PACTOR_ARQ_MASTERCONNECT:
		fittingrxcmd = HFAPP_MSG_MODE_IRS;
		lastrxmsg = HFAPP_MSG_STATE_PACTOR_IRS; 
		/* it should come, but after fresh master connect it 
		  lasts too long -> erroneous switch from 
		  (wrong guessed) standby to e.g. Pactor FEC by 
		  next autotx !! So i insert it artificially... */
		display_status ("autorx: Pactor Master Connect -> IRS"); 
		break;

	    case HFAPP_MSG_STATE_AMTOR_ARQ_MASTERCONNECT:
		fittingrxcmd = HFAPP_MSG_MODE_IRS;
		lastrxmsg = HFAPP_MSG_STATE_AMTOR_IRS; 
		/* it should come, but after fresh master connect it 
		  lasts too long -> erroneous switch from 
		  (wrong guessed) standby to e.g. Amtor Col-FEC by 
		  next autotx !! So i insert it artificially... */
		display_status ("autorx: Amtor Master Connect -> IRS"); 
		break;

	    case HFAPP_MSG_STATE_GTOR_ARQ_MASTERCONNECT:
		fittingrxcmd = HFAPP_MSG_MODE_IRS;
		lastrxmsg = HFAPP_MSG_STATE_GTOR_IRS; 
		/* it should come, but after fresh master connect it 
		  lasts too long -> erroneous switch from 
		  (wrong guessed) standby to e.g. Pactor FEC by 
		  next autotx !! So i insert it artificially... */
		display_status ("autorx: Gtor Master Connect -> IRS"); 
		break;

	    case HFAPP_MSG_STATE_RTTY_TX:
		fittingrxcmd = HFAPP_MSG_START_RTTY_RX;
		display_status ("autorx: RTTX TX -> RX"); 
		break;

	    case HFAPP_MSG_STATE_MT63_TX:
		fittingrxcmd = HFAPP_MSG_START_MT63_RX;
		display_status ("autorx: MT63 TX -> RX"); 
		break;

	    default:
		/*not every command results in a message from hfkernel.*/
		//display_status 
		//    ("autorx: last TX msg not clear. Checking last TX cmd."); 
		switch (lasttxcmd) {

		    case HFAPP_MSG_MODE_ISS:
			fittingrxcmd = HFAPP_MSG_MODE_IRS;
			//display_status ("autorx: ISS -> IRS"); 
			break;

		    case HFAPP_MSG_START_PACTOR_FEQ: 
			// was before: Pactor only.
			// But: if mailbox-beacon in p-fec running,
			// still Amtor Connect for mailbox shall be possible.
			fittingrxcmd = HFAPP_MSG_START_STANDBY;
			display_status 
			    ("autorx: Pactor FEC -> Standby"); 
			break;

		    case HFAPP_MSG_START_AMTOR_COLFEQ:
			fittingrxcmd = HFAPP_MSG_START_STANDBY;
			display_status 
			    ("autorx: Amtor Colfec -> Standby"); 
			break;

		    case HFAPP_MSG_START_AMTOR_SELFEQ:
			fittingrxcmd = HFAPP_MSG_START_STANDBY;
			display_status 
			    ("autorx: Amtor Selfec -> Standby"); 
			break;

		    case HFAPP_MSG_START_RTTY_TX:
			fittingrxcmd = HFAPP_MSG_START_RTTY_RX;
			display_status ("autorx: RTTX TX -> RX"); 
			break;
		
		    case HFAPP_MSG_MODE_QRT:
			fittingrxcmd = HFAPP_MSG_START_STANDBY;
			qrt_pending = 0;
			display_status 
			    ("autorx: QRT -> Standby"); 
			break;
		    
		    default:
	    		fittingrxcmd = lastrxcmd;
			if ( ! fittingrxcmd ) {   
			    display_status("autorx: Please select a rx mode.");
			    //fittingrxcmd = HFAPP_MSG_START_STANDBY;
			    //display_status 
			    //("autorx: -> I guess and switch to Pactor Amtor Gtor Standby"); 
			    return;
			} else {
			    display_status
				("autorx: switching to last rx mode: %d", fittingrxcmd);
			}
			break;
		}
		break;
	}
/* 
 *  switch to the fittingrxcmd mode by sending a msg
 */
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(fittingrxcmd);
	msg_send(&msg);
	//display_status("AUTO RECEIVE MODE: %d", fittingrxcmd);
	//errprintf(SEV_INFO, "autorx: auto rx mode %d", fittingrxcmd); 
	lastrxcmd = fittingrxcmd;
	way = RX;
	return;
}

/* --------------------------------------------------------------------- */

void qrt() {
	struct hfapp_msg msg;

	if (!arq) return;
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(0);
	msg.hdr.type = htonl(HFAPP_MSG_MODE_QRT);
	msg_send(&msg);
	display_status("automatic QRT.");
	lasttxcmd = HFAPP_MSG_MODE_QRT; /* qrt is a tx !!! */
	way = TX; /* if this is not so, autorx will not work*/
	autorx_on = 1;
	qrt_pending = 1;  // means we are waiting for qrt machine to finish
}

/* --------------------------------------------------------------------- */
