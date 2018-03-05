/*****************************************************************************/

/*
 *      gtor.h  --  GTOR implementation header.
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
      
#ifndef _GTOR_H
#define _GTOR_H

/* --------------------------------------------------------------------- */
/*
 * routines for use by the standby stuff
 */

extern int gtor_monitor
    (unsigned int bd, l1_soft_t *cur, l1_soft_t *prev, l1_time_t tm, 
    unsigned int ovrsampl, int freqdev);

/* --------------------------------------------------------------------- */

extern void gtor_set_params
    (const char *destcall, const char *mycall, int txd, int retry);
extern void *mode_gtor_arq(void *dummy);

extern void gtor_mode_qrt(void);
extern void gtor_mode_irs(void);
extern void gtor_mode_iss(void);
extern void gtor_mode_speedup(void);

/* --------------------------------------------------------------------- */
#endif /* _GTOR_H */
