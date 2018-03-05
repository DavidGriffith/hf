/*****************************************************************************/

/*
 *      xgintf.c  --  xgraph interface.
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
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include"config.h"
#endif

#include <stdlib.h> 
#include<stdio.h>
#include "xgintf.h"
#include "dcf77.h"  /* die */

/* --------------------------------------------------------------------- */

#define XG_MAXVAL 8
#define XG_MAXLEN 32768

/* --------------------------------------------------------------------- */

static const char *xg_vtitle[XG_MAXVAL] = { NULL, };
static float xg_v[XG_MAXVAL][XG_MAXLEN] = { };
static unsigned int xg_valused = 0;
static unsigned int xg_count = 0;

/* --------------------------------------------------------------------- */

void xg_next(unsigned int len, const char *title)
{
	FILE *p;
	unsigned int u, l;

	xg_count++;
	if (xg_count < XG_MAXLEN && xg_count < len)
		return;
	if (len > XG_MAXLEN)
		len = XG_MAXLEN;
	if (!(p = popen("xgraph", "w")))
		die("popen");
	if (title)
		fprintf(p, "TitleText: %s\n", title);
	for (l = 0; l < xg_valused; l++) {
		if (xg_vtitle[l])
			fprintf(p, "\n\"%s\nmove ", xg_vtitle[l]);
		else
			fprintf(p, "\nmove ");
		for (u = 0; u < len; u++)
			fprintf(p, "%d %f\n", u, xg_v[l][u]);
	}
	pclose(p);
	exit(0);
}

/* --------------------------------------------------------------------- */

void xg_val(unsigned int tr, float v, const char *tit)
{
	if (tr >= XG_MAXVAL)
		return;
	if (tr >= xg_valused)
		xg_valused = tr+1;
	if (tit && !xg_vtitle[tr])
		xg_vtitle[tr] = tit;
	xg_v[tr][xg_count] = v;
}

/* --------------------------------------------------------------------- */
