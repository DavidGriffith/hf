/*****************************************************************************/

/*
 *      timedec.c  --  Linux soundcard DCF77 receiver, time decoder.
 *
 *      Copyright (C) 1997  Thomas Sailer (sailer@ife.ee.ethz.ch)
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

#include <stdlib.h>
#include <time.h>
#include "dcf77.h"

/* --------------------------------------------------------------------- */

#define ZONE_MEZ  2
#define ZONE_MESZ 1

/* --------------------------------------------------------------------- */

void time_decode(unsigned long long bits, unsigned int samples)
{
	static const char *tz_name[4] = { "invalid 0", "MESZ", "MEZ", "invalid 3" };
	static const char *daynames = "MonTueWedThuFriSatSun";
	char system_set_date[128], system_set_time[128];
	unsigned int u;
	unsigned int pgrp1, pgrp2, pgrp3;
	struct tm tm, *tmp;
	time_t t;
	unsigned int dw;
	unsigned int tz;
	unsigned int tzchg;
	unsigned int leapsec;
	unsigned int backup_ant;

	pgrp1 = (bits >> 21) & 0xff;
	pgrp2 = (bits >> 29) & 0x7f;
	pgrp3 = (bits >> 36) & 0x7fffff;
	
//	if (!hbg_mode) { /* don't know the exact parity bits/format! */
		if (hweight8(pgrp1) & 1) {
			vlprintf(3, "DCF77 time: parity 1 invalid\n");
			goto error;
		}
		if (hweight8(pgrp2) & 1) {
			vlprintf(3, "DCF77 time: parity 2 invalid\n");
			goto error;
		}
		if (hweight32(pgrp3) & 1) {
			vlprintf(3, "DCF77 time: parity 3 invalid\n");
			goto error;
		}
		if (!(bits & 0x100000)) {
			vlprintf(3, "DCF77 time: startbit wrong!\n");
			goto error;
		}
//	}

	backup_ant = (bits & 0x8000) != 0;
	tzchg = (bits & 0x10000) != 0;
	tz = (bits >> 17) & 3;
	leapsec = (bits & 0x80000) != 0;

	u = pgrp1 & 0x7f;
	if ((u & 0x0f) >= 0x0a || u >= 0x60) {
		vlprintf(3, "DCF77 time: invalid minute field 0x%02x\n", u);
		goto error;
	}
	tm.tm_sec = tm.tm_isdst = 0;
	tm.tm_min = (u & 0x0f) + 10 * ((u >> 4) & 0x0f);
	u = pgrp2 & 0x3f;
	if ((u & 0x0f) >= 0x0a || u >= 0x24) {
		vlprintf(3, "DCF77 time: invalid hour field 0x%02x\n", u);
		goto error;
	}
	tm.tm_hour = (u & 0x0f) + 10 * ((u >> 4) & 0x0f);
	u = pgrp3 & 0x3f;
	if ((u & 0x0f) >= 0x0a || u >= 0x31) {
		vlprintf(3, "DCF77 time: invalid day field 0x%02x\n", u);
		goto error;
	}
	tm.tm_mday = (u & 0x0f) + 10 * ((u >> 4) & 0x0f);
	u = (pgrp3 >> 9) & 0x1f;
	if ((u & 0x0f) >= 0x0a || u >= 0x12) {
		vlprintf(3, "DCF77 time: invalid month field 0x%02x\n", u);
		goto error;
	}
	tm.tm_mon = (u & 0x0f) + 10 * ((u >> 4) & 0x0f);
	u = (pgrp3 >> 14) & 0xff;
	if ((u & 0x0f) >= 0x0a || u >= 0xa0) {
		vlprintf(3, "DCF77 time: invalid year field 0x%02x\n", u);
		goto error;
	}
	tm.tm_year = (u & 0x0f) + 10 * ((u >> 4) & 0x0f);
	if (tm.tm_year < 80)
		tm.tm_year += 100;
	dw = (pgrp3 >> 6) & 0x7;
	if (dw == 0) {
		vlprintf(3, "DCF77 time: invalid day of week field 0x%02x\n", u);
		goto error;
	}
	vlprintf(1, "\nDCF77 time: \n"
		"%s antenna, %s time zone change, time zone %s, "
		"leap second: %s\n", backup_ant ? "backup" : "normal", 
		tzchg ? "approaching" : "no", tz_name[tz], 
		leapsec ? "within 1 hour" : "none");
	vlprintf(1, "DCF77 time: TIME: %02d:%02d DATE: %.3s, " 
		// removed "-3" before .3s after date: %, typo? Günther
		"%02d.%02d.%02d\n",
		  tm.tm_hour, tm.tm_min, daynames+3*(dw-1), 
		  tm.tm_mday, tm.tm_mon, tm.tm_year + 1900);
	t = mktime(&tm);
	if (t == INVALID_TIME) {
		vlprintf(1, "mktime failed\n");
		goto error;
	}
/* XXX FreeBSD takes care of timezone internally, I don't know about others. 
 * -db (VA3DB)
 */
#ifdef __linux__
 	t -= timezone;
#endif
/*	t -= tz == ZONE_MESZ ? 7200 : 3600; */
/* bug? isn't summer time later than winter time ? */
	t -= tz == ZONE_MESZ ? 3600 : 7200 ; 
	tmp = localtime(&t);
	printf  ("DCF77 time: "
		"%02d:%02d:%02d  %02d.%02d.%02d\n",
		 tmp->tm_hour, tmp->tm_min, tmp->tm_sec, 
		 tmp->tm_mday, tmp->tm_mon, tmp->tm_year + 1900);
	tc_minute(t, samples);
	if (set_time) {
	    sprintf(system_set_date, "date -s %02d/%02d/%02d > /dev/null", 
		tmp->tm_mon, tmp->tm_mday, tmp->tm_year + 1900);
	    sprintf(system_set_time, "date -s %02d:%02d:%02d > /dev/null", 
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec); 
	    system(system_set_date);
	    system(system_set_time);
	    //system("date");
	    if (tz == ZONE_MESZ) 
	        printf("I have set the system time to MESZ = CEST: ");
	    else
	        printf("I have set the system time to MEZ = CET: ");
	    printf("%02d:%02d:%02d  %02d.%02d.%02d\n",
	        tmp->tm_hour, tmp->tm_min, tmp->tm_sec, 
	        tmp->tm_mday, tmp->tm_mon, tmp->tm_year + 1900);
	}
	return;
  error:
	tc_minute(INVALID_TIME, samples);
	return;
}

/* --------------------------------------------------------------------- */
