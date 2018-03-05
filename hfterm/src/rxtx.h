/*****************************************************************************/
/*
 *      rxtx.h  --  rx tx file selection fixtext routines
 * 	for hfterm, the simple HF terminal.
 *
 *      Copyright (C) 1996  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *        Swiss Federal Institute of Technology (ETH), Electronics Lab
 *
 *	modified 2001-2 by Axel Krystof DF3JRK
 * 	modified 2003 by Günther Montag DL4MGE
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
      
#ifndef _RXTX_H
#define _RXTX_H

#define MAXKEY 2048
#define MAXRX 2048
#define RX 0
#define TX 1

#include "hft.h"

/* --------------------------------------------------------------------- */

void transmit(const char *fmt, ... );
void textinsert(char *filename, GtkText *text);
void parse(char* magic, char* bragitem);
void fixtext_send(int fixnr);
void fixtext_read();
void fixtext_store();
void file_send();
void beacon_suspend();
void beacon_restore();
int  beacon_file_prepare();
void beacon_fixtext_prepare();
void beacon_send();
void beacon_stop();
void rx_routine_store_prepare();
void rx_routine_store_part();
void rx_routine_store_rest();
void fileselection_dialog(char* title);
void fileselection(GtkWidget *widget, GtkWidget * filedialog);
void fileselection_cancel(GtkWidget *widget, GtkWidget * filedialog);
void rx_demand_store();
void userrx();
void usertx();
void autotx();
void autorx();
void qrt();

extern int beaconcount, fix_on, beacon_on, beacon_was_on, fixbeacon, autorx_on, autotx_on;
extern int rxfile_ready, qrt_pending, arq;
extern int lasttxcmd, lastrxcmd, lastrxmsg, lasttxmsg;
extern char gmt[32];
extern char specialfile[256];
extern char rxfilehead[256], rxfilefoot[256];
extern char keytext[MAXKEY], beaconbuffer[MAXKEY];

#endif /* _RXTX_H */
