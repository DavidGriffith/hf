/*****************************************************************************/

/*
 *      util.c  --  Linux soundcard DCF77, misc utilities.
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "dcf77.h"

/* --------------------------------------------------------------------- */

unsigned int verboselevel = 0;

/* --------------------------------------------------------------------- */

void die(const char *func)
{
	vlprintf(-1, "%s: %s", func, strerror(errno));
        exit(1);
}

/* --------------------------------------------------------------------- */

void vlprintf(int vl, const char *fmt, ...)
{
        va_list args;
	
	if (vl > (int)verboselevel)
		return;
        va_start(args, fmt);
	fprintf(stderr, "dcf77[%lu]: ", (unsigned long)getpid());
	vfprintf(stderr, fmt, args);
        va_end(args);
        if (vl <= -1)
                exit(1);
}

/* --------------------------------------------------------------------- */
