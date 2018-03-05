/*****************************************************************************/

/*
 *      timeenc.c  --  Linux soundcard DCF77 time information encoding.
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
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <time.h>

#include "dcf77.h"

/* --------------------------------------------------------------------- */

#define ZONE_MEZ  2
#define ZONE_MESZ 1

static struct timeenc_state {
	time_t time;
	unsigned int zone:2;
	unsigned int antenna:1;
	unsigned int ann_zone:1;
	unsigned int ann_leapsec:1;

	unsigned int sec;
	unsigned long long bits;

} t;

/* --------------------------------------------------------------------- */

static void encode(void)
{
	struct tm *tm;
	unsigned int u;
	time_t tt = t.time + (t.zone == ZONE_MESZ ? 7200 : 3600);
	
	tm = gmtime(&tt);
	t.sec = tm->tm_sec;
	u = (tm->tm_mday % 10) | ((tm->tm_mday / 10) << 4) |
		((tm->tm_mon % 10) << 9) | ((tm->tm_mon / 10) << 13) |
		((tm->tm_year % 10) << 14) | (((tm->tm_year / 10) % 10) << 18);
	if (tm->tm_wday == 0)
		u |= 0x7 << 6;
	else
		u |= tm->tm_wday << 6;
	u |= (hweight32(u) & 1) << 22;
	t.bits = ((unsigned long long)u) << 36;
	u = (tm->tm_hour % 10) | ((tm->tm_hour / 10) << 4);
	u |= (hweight8(u) & 1) << 6;
	t.bits |= ((unsigned long long)u) << 29;
	u = (tm->tm_min % 10) | ((tm->tm_min / 10) << 4);
	u |= (hweight8(u) & 1) << 7;
	t.bits |= u << 21;
	u = (1 << 20) | 0x3ff | (t.antenna << 15) | (t.ann_zone << 16) |
		(t.zone << 17) | (t.ann_leapsec << 19);
	t.bits |= u;
	vlprintf(1, "Transmitting time: %2u:%02u:%02u  %.3s %2u.%02u.%02u"
		"\n\t     zone:%4s ant:%c annz:%c "
		"annls:%c\n", tm->tm_hour, tm->tm_min, tm->tm_sec, 
		"SunMonTueWedThuFriSat"+3*tm->tm_wday,
		 tm->tm_mday, tm->tm_mon + 1, 
		 tm->tm_year + 1900, t.zone == ZONE_MESZ ? "MESZ" : "MEZ",
		 t.antenna ? 'R' : 'N', t.ann_zone ? '+' : ' ', 
		 t.ann_leapsec ? '+' : ' ');
}
//      corrected tm_mon to tm_mon + 1 and added 
// 	the +1900 to tm_year by günther to change output "Sat 12 May 104" to 
// 	"Sat 12 June 2004" on this day

/* --------------------------------------------------------------------- */

void timeenc_init(time_t curtime, unsigned int tz_mesz, unsigned int ant,
		  unsigned int ann_tz, unsigned int ann_leapsec)
{
	memset(&t, 0, sizeof(t));
	t.zone = tz_mesz ? ZONE_MESZ : ZONE_MEZ;
	t.antenna = !!ant;
	t.ann_zone = !!ann_tz;
	t.ann_leapsec = !!ann_leapsec;
	t.time = curtime;
	encode();
}

/* --------------------------------------------------------------------- */

int timeenc_getbit(void)
{
	int b;

	if (t.sec == 59)
		b = -1;
	else
		b = (t.bits & (1ULL << t.sec)) != 0;
	t.sec++;
	t.time++;
	if (t.sec >= 60)
		encode();
	return b;
}

/* --------------------------------------------------------------------- */
