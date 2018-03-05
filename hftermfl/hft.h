/*****************************************************************************/

/*
 *      hft.h  --  Simple HF terminal.
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
      
#ifndef _HFT_H
#define _HFT_H

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

#define SPEC_WIDTH 512
#define SPEC_HEIGHT 384

/* --------------------------------------------------------------------- */

extern int scope_on;

/* --------------------------------------------------------------------- */

extern void display_status(const char *data);
extern void scope_draw(float *data);
extern void write_kernel(unsigned char *data, int datalen);
extern void write_term(int term, unsigned char *data, int datalen);
extern void msg_process(int fd);
extern void term_process(int trm, int fd);
extern void msg_send(struct hfapp_msg *msg);

/* --------------------------------------------------------------------- */
#endif /* _HFT_H */
