/*****************************************************************************/

/*
 *      params.c  --  parameter and brag handling for hfterm.
 *
 *      Copyright (C) 1996  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *        Swiss Federal Institute of Technology (ETH), Electronics Lab
 *	modified 2000-2005 by Axel Krause & Günther Montag (dl4mge@darc.de).
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

/* --------------------------------------------------------------------- */

#ifndef FREQ_SPACE
#define FREQ_SPACE 1275
#endif

#ifndef FREQ_MARK
#define FREQ_MARK  1475
#endif

#ifndef MYCALL
#define MYCALL "DL4MGE"
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

struct par params;

/* --------------------------------------------------------------------- */

void param_set_defaults(void)
{
/* Falls keine config-Datei gefunden wird: params vorbelegen------- */
/* if binary per-user config ~/hfrc not fount, preset params ------ */
	char mycall[16];
	char buf[1024];
	char *bp1;
	const char *bp2;
	
	strncpy(mycall, MYCALL, sizeof(mycall));
	
	memset(&params, 0, sizeof(params));
	
	strncpy(params.brag.call, MYCALL, sizeof(params.brag.call));
	params.general.snd_corr = 1.0;
	params.general.tv_corr = 1.0;
	params.general.cpu_mhz = 0;
	strncpy(params.general.soundcard, "/dev/dsp0", sizeof(params.general.soundcard));
	strncpy(params.general.serial, "none", sizeof(params.general.serial));
	params.general.beaconpause = 10;
	params.general.squelchpercent = 5;
	params.cw.wpm = 12;
	params.cw.tone = 880;
	params.cw.farnsworth = 0;
	params.rtty.baud = 45.45;
	params.amtor.txdelay = 30;
	params.amtor.retry = 30;
	params.gtor.txdelay = 30;
	params.gtor.retry = 30;
	params.pactor.txdelay = 30;
	params.pactor.retry = 30;
	params.pactor.crcpreset[0] = strtoul(PCT_CRC_0, NULL, 16);
	params.pactor.crcpreset[1] = strtoul(PCT_CRC_1, NULL, 16);
	params.pactor.crcpreset[2] = strtoul(PCT_CRC_2, NULL, 16);
	params.pactor.crcpreset[3] = strtoul(PCT_CRC_3, NULL, 16);

	set_fsk_freq(FREQ_MARK, FREQ_SPACE);
	strncpy(params.gtor.mycall, mycall, sizeof(params.gtor.mycall));
	strncpy(params.pactor.mycall, mycall, sizeof(params.pactor.mycall));
	bp2 = mycall-1+strlen(mycall);
	buf[sizeof(buf)-1] = 0;
	bp1 = buf+4;
	*bp1 = 0;
	while (bp2 >= mycall) 
	{
		if (isalpha(*bp2))
			*--bp1 = *bp2;
		bp2--;
		if (bp1 <= buf)
			bp1 = buf+1;
	}
	strncpy(params.amtor.mycall, buf, sizeof(params.amtor.mycall));
	strncpy(params.amtor.selfeccall, buf, sizeof(params.amtor.selfeccall));
	params.mt63.bandwidth        = 1000;
	params.mt63.integration      = 32;
	strncpy(params.mt63.cwcall, "MT63____", sizeof(params.mt63.cwcall));
	params.mt63.doubleinterleave = 0;	
	params.mailbox.port = 6300;
	strncpy(params.mailbox.host, "127.0.0.1", sizeof(params.mailbox.host));
}

/* --------------------------------------------------------------------- */

//static 
void param_get(void)
{
	GtkEntry *entry;
	GtkToggleButton *tog;
	char par[64];
	
	/* brag */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry27"));
	strncpy(params.brag.call, gtk_entry_get_text(entry), sizeof(params.brag.call));
	strncpy(params.gtor.mycall, gtk_entry_get_text(entry), sizeof(params.gtor.mycall));
	strncpy(params.pactor.mycall, gtk_entry_get_text(entry), sizeof(params.pactor.mycall));
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry28"));
	strncpy(params.brag.op, gtk_entry_get_text(entry), sizeof(params.brag.op));

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry29"));
	strncpy(params.brag.qth, gtk_entry_get_text(entry), sizeof(params.brag.qth));

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry30"));
	strncpy(params.brag.loc, gtk_entry_get_text(entry), sizeof(params.brag.loc));

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry31"));
	strncpy(params.brag.rig, gtk_entry_get_text(entry), sizeof(params.brag.rig));

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry32"));
	strncpy(params.brag.pwr, gtk_entry_get_text(entry), sizeof(params.brag.pwr));

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry33"));
	strncpy(params.brag.ant, gtk_entry_get_text(entry), sizeof(params.brag.ant));

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry34"));
	strncpy(params.brag.www, gtk_entry_get_text(entry), sizeof(params.brag.www));
 	
	/* general */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "soundcorr"));
	params.general.snd_corr = strtod(gtk_entry_get_text(entry), NULL);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "timecorr"));
	params.general.tv_corr = strtod(gtk_entry_get_text(entry), NULL);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "cpumhzcorr"));
	params.general.cpu_mhz = strtod(gtk_entry_get_text(entry), NULL);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar),
	"soundcardvalue"));
	strncpy(par, gtk_entry_get_text(entry), sizeof(par));
	if (!par[0]) sprintf(par, "none");
 	strncpy(params.general.soundcard, par, sizeof(par));
 	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar),
	"serialvalue"));
	strncpy(par, gtk_entry_get_text(entry), sizeof(par));
	if (!par[0]) sprintf(par, "none");
 	strncpy(params.general.serial, par, sizeof(par));
 	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar),
	"kerneloptions"));
	strncpy(params.general.kerneloptions, gtk_entry_get_text(entry), sizeof(params.general.kerneloptions));
 	
	
	/* FSK parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskspacefreq"));
	params.fsk.freq[0] = strtoul(gtk_entry_get_text(entry), NULL, 0);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskmarkfreq"));
	params.fsk.freq[1] = strtoul(gtk_entry_get_text(entry), NULL, 0);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "squelchpercent"));
	params.general.squelchpercent = strtoul(gtk_entry_get_text(entry), NULL, 0);
	
	/* CW parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "wpm"));
	params.cw.wpm = strtoul(gtk_entry_get_text(entry), NULL, 0);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "tone"));
	params.cw.tone = strtoul(gtk_entry_get_text(entry), NULL, 0);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "farnsworth"));
	params.cw.farnsworth = gtk_toggle_button_get_active(tog);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "dtr"));
	params.cw.dtr = gtk_toggle_button_get_active(tog);
	
	/* RTTY parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "rttybaudrate"));
	params.rtty.baud = strtod(gtk_entry_get_text(entry), NULL);
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
	if (params.amtor.retry < 4) params.amtor.retry = 4;
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "amtorinvert"));
	params.amtor.rxinvert = gtk_toggle_button_get_active(tog);
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "amtorrxtxinvert"));
	params.amtor.txinvert = gtk_toggle_button_get_active(tog);
	if (params.amtor.rxinvert)
		params.amtor.txinvert = !params.amtor.txinvert;
	
	/* GTOR parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtordestcall"));
	strncpy(params.gtor.destcall, gtk_entry_get_text(entry), sizeof(params.gtor.destcall));
	//entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtormycall"));
	//strncpy(params.gtor.mycall, gtk_entry_get_text(entry), sizeof(params.gtor.mycall));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtorretry"));
	params.gtor.retry = strtoul(gtk_entry_get_text(entry), NULL, 0);
	if (params.gtor.retry < 4) params.gtor.retry = 4;
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtortxdelay"));
	params.gtor.txdelay = strtoul(gtk_entry_get_text(entry), NULL, 0);
	
	/* Pactor parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcall"));
	strncpy(params.pactor.destcall, gtk_entry_get_text(entry), sizeof(params.pactor.destcall));
	//entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactormycall"));
	//strncpy(params.pactor.mycall, gtk_entry_get_text(entry), sizeof(params.pactor.mycall));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorretry"));
	params.pactor.retry = strtoul(gtk_entry_get_text(entry), NULL, 0);
	if (params.pactor.retry < 4) params.pactor.retry = 4;
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
	
	/* MT63 parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mt63_bandwidth_value"));
	params.mt63.bandwidth = strtoul(gtk_entry_get_text(entry), NULL, 0);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mt63_integration"));
	params.mt63.integration = strtoul(gtk_entry_get_text(entry), NULL, 0);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mt63_cwcall"));
	strncpy(params.mt63.cwcall, gtk_entry_get_text(entry), sizeof(params.mt63.cwcall));
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "mt63_doubleinterleave"));
	params.mt63.doubleinterleave = gtk_toggle_button_get_active(tog);
	
	/* mailbox parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mailbox_host"));
	strncpy(params.mailbox.host, gtk_entry_get_text(entry), sizeof(params.mailbox.host));
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mailbox_port"));
	params.mailbox.port = strtoul(gtk_entry_get_text(entry), NULL, 0);
}
	
//static 
void param_set(void)
{
	GtkEntry *entry;
	GtkToggleButton *tog;
	char buf[256];

	/* brag */

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry27"));
	strncpy(buf, params.brag.call, sizeof(params.brag.call));
	buf[sizeof(params.brag.call)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry28"));
	strncpy(buf, params.brag.op, sizeof(params.brag.op));
	buf[sizeof(params.brag.op)] = 0;
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry29"));
	strncpy(buf, params.brag.qth, sizeof(params.brag.qth));
	buf[sizeof(params.brag.qth)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry30"));
	strncpy(buf, params.brag.loc, sizeof(params.brag.loc));
	buf[sizeof(params.brag.loc)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry31"));
	strncpy(buf, params.brag.rig, sizeof(params.brag.rig));
	buf[sizeof(params.brag.rig)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry32"));
	strncpy(buf, params.brag.pwr, sizeof(params.brag.pwr));
	buf[sizeof(params.brag.pwr)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry33"));
	strncpy(buf, params.brag.ant, sizeof(params.brag.ant));
	buf[sizeof(params.brag.ant)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "entry34"));
	strncpy(buf, params.brag.www, sizeof(params.brag.www));
	buf[sizeof(params.brag.www)] = 0;
	gtk_entry_set_text(entry, buf);
 
	/* general */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "soundcorr"));
	snprintf(buf, sizeof(buf), "%f", params.general.snd_corr);
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "timecorr"));
	snprintf(buf, sizeof(buf), "%f", params.general.tv_corr);
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "cpumhzcorr"));
	snprintf(buf, sizeof(buf), "%f", params.general.cpu_mhz);
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar),
	"soundcardvalue"));
	strncpy(buf, params.general.soundcard, sizeof(params.general.soundcard));
	buf[sizeof(params.general.soundcard)] = 0;
	gtk_entry_set_text(entry, buf);
	 	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar),
	"serialvalue"));
	strncpy(buf, params.general.serial, sizeof(params.general.serial));
	buf[sizeof(params.general.serial)] = 0;
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar),
	"kerneloptions"));
	strncpy(buf, params.general.kerneloptions, sizeof(params.general.kerneloptions));
	buf[sizeof(params.general.kerneloptions)] = 0;
	gtk_entry_set_text(entry, buf);
		
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "beaconpausevalue"));
	snprintf(buf, sizeof(buf), "%u", params.general.beaconpause);
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "squelchpercent"));
	snprintf(buf, sizeof(buf), "%u", params.general.squelchpercent);
	gtk_entry_set_text(entry, buf);
		
	/* FSK parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskspacefreq"));
	snprintf(buf, sizeof(buf), "%u", params.fsk.freq[0]);
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskmarkfreq"));
	snprintf(buf, sizeof(buf), "%u", params.fsk.freq[1]);
	gtk_entry_set_text(entry, buf);
	
	/* CW parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "wpm"));
	snprintf(buf, sizeof(buf), "%u", params.cw.wpm);
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "tone"));
	snprintf(buf, sizeof(buf), "%u", params.cw.tone);
	gtk_entry_set_text(entry, buf);
	
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "farnsworth"));
	gtk_toggle_button_set_active(tog, params.cw.farnsworth);
	
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "dtr"));
	gtk_toggle_button_set_active(tog, params.cw.dtr);
	
	
	/* RTTY parameters */
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "rttybaudrate"));
	snprintf(buf, sizeof(buf), "%f", params.rtty.baud);
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
	
	
	/* GTOR parameters */
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtordestcall"));
	strncpy(buf, params.gtor.destcall, sizeof(params.gtor.destcall));
	buf[sizeof(params.gtor.destcall)] = 0;
	gtk_entry_set_text(entry, buf);
	
	//entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtormycall"));
	//strncpy(buf, params.gtor.mycall, sizeof(params.gtor.mycall));
	//buf[sizeof(params.gtor.mycall)] = 0;
	//gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtorretry"));
	snprintf(buf, sizeof(buf), "%u", params.gtor.retry);
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "gtortxdelay"));
	snprintf(buf, sizeof(buf), "%u", params.gtor.txdelay);
	gtk_entry_set_text(entry, buf);
	/* Pactor parameters */
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactorcall"));
	strncpy(buf, params.pactor.destcall, sizeof(params.pactor.destcall));
	buf[sizeof(params.pactor.destcall)] = 0;
	gtk_entry_set_text(entry, buf);
	
	//entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "pactormycall"));
	//strncpy(buf, params.pactor.mycall, sizeof(params.pactor.mycall));
	//buf[sizeof(params.pactor.mycall)] = 0;
	//gtk_entry_set_text(entry, buf);
	
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
	
	
	/* MT63 parameters */
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mt63_bandwidth_value"));
	snprintf(buf, sizeof(buf), "%u", params.mt63.bandwidth);
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mt63_integration"));
	snprintf(buf, sizeof(buf), "%u", params.mt63.integration);
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mt63_cwcall"));
	strncpy(buf, params.mt63.cwcall, sizeof(params.mt63.cwcall));
	buf[sizeof(params.mt63.cwcall)] = 0;
	gtk_entry_set_text(entry, buf);
	
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wpar), "mt63_doubleinterleave"));
	gtk_toggle_button_set_active(tog, params.mt63.doubleinterleave);
	
	
	/* mailbox parameters */
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mailbox_host"));
	strncpy(buf, params.mailbox.host, sizeof(params.mailbox.host));
	buf[sizeof(params.mailbox.host)] = 0;
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "mailbox_port"));
	snprintf(buf, sizeof(buf), "%d", params.mailbox.port);
	gtk_entry_set_text(entry, buf);
}
	
//static 
void param_kernel(void)
{
	/* we send the params to hfkernel */
	struct hfapp_msg msg;
	/* general parameters */
	msg.hdr.type = htonl(HFAPP_MSG_SET_GENERALPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.generalpar));
	msg.data.generalpar.beaconpause = htons(params.general.beaconpause);
	msg_send(&msg);
	/* FSK parameters */
	set_fsk_freq(params.fsk.freq[1], params.fsk.freq[0]);
	/* CW parameters */
	msg.hdr.type = htonl(HFAPP_MSG_SET_CWPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.cwpar));
	msg.data.cwpar.wpm = htons(params.cw.wpm);
	msg.data.cwpar.tone = htons(params.cw.tone);
	msg.data.cwpar.farnsworth = params.cw.farnsworth;
	msg.data.cwpar.dtr = params.cw.dtr;
	msg_send(&msg);
	/* RTTY parameters */
	msg.hdr.type = htonl(HFAPP_MSG_SET_RTTYPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.rpar));
	//msg.data.rpar.baud = htons(params.rtty.baud);
	msg.data.rpar.baud = (params.rtty.baud);
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
	/* GTOR parameters */
	msg.hdr.type = htonl(HFAPP_MSG_SET_GTORPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.gpar));
	strncpy(msg.data.gpar.destcall, params.gtor.destcall, sizeof(msg.data.gpar.destcall));
	strncpy(msg.data.gpar.mycall, params.gtor.mycall, sizeof(msg.data.gpar.mycall));
	msg.data.gpar.txdelay = htons(params.gtor.txdelay);
	msg.data.gpar.retry = htons(params.gtor.retry);
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
	/* MT63 parameters */
	msg.hdr.type = htonl(HFAPP_MSG_SET_MT63PAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.mt63par));
	msg.data.mt63par.bandwidth = htons(params.mt63.bandwidth);
	msg.data.mt63par.integration = htons(params.mt63.integration);
	strncpy(msg.data.mt63par.cwcall, params.mt63.cwcall, sizeof(msg.data.mt63par.cwcall));
	msg.data.mt63par.doubleinterleave = params.mt63.doubleinterleave;
	msg_send(&msg);
}

void param_read()
{
/* aus config-Datei gespeicherte params einlesen ------------------- */
	FILE *conf;
	conf = fopen("hf/hfterm.rc", "r");
	if(conf == NULL) {
	  display_status("Configuration File hf/hfterm.rc can not be opened.");
	  return;
	}  
	else	
	
	display_status ("Configuration File hf/hfterm.rc opened.");
	if(fread(&params, sizeof(params), 1, conf) != 1) {
		display_status("Error while reading configuration file.");
		fclose(conf);	
		return;
	}
	fclose(conf);	
} 

void param_store()
{
/* - Konfiguration speichern ----------------------------------------*/
	FILE *conf = fopen("hf/hfterm.rc", "w");
	if(conf == NULL) {
	  display_status
	      ("Configuration File hf/hfterm.rc can not be opened.");
	  return;
	} else	
	    display_status("Configuration File hf/hfterm.rc opened.");
	if(fwrite(&params, sizeof(params), 1, conf) != 1)
	{
	  display_status
	      ("Error while writing configuration file hf/hfterm.rc.");
	  return;
	}
	else	
	display_status("Configuration stored in hf/hfterm.rc.");
	fclose(conf);	
}

/* ------------------------------------------------------------------ */

