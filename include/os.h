/*****************************************************************************/

/*
 *      os.h  --  Linux io.
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
      
#ifndef _OS_H
#define _OS_H

/* --------------------------------------------------------------------- */

#include <syslog.h>
#include "hfapp.h"

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

#define KBD_CHAR  0xff
#define KBD_BIN   0x100
#define KBD_EOF   0xffff

extern unsigned short kbd_get(void);
extern void kbd_ack(void);
extern void kbd_negack(void);
extern void kbd_back(void);
extern void kbd_clear_and_fill(const unsigned char *data, unsigned int length);

/* --------------------------------------------------------------------- */

extern void bufprintf(unsigned int which, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
extern void bufwrite(unsigned int which, const unsigned char *data, unsigned int datalen);

/* --------------------------------------------------------------------- */

extern void send_short_msg(u_int32_t type, u_int32_t err);
extern void send_msg(struct hfapp_msg *msg);

/* --------------------------------------------------------------------- */

#ifndef HAVE_VSNPRINTF

#include <stdarg.h>

extern int snprintf(char *buf, size_t len, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
extern int vsnprintf(char *buf, size_t len, const char *fmt, va_list ap);

#endif

/* --------------------------------------------------------------------- */
#endif /* _OS_H */
