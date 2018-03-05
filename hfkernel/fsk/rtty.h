/*****************************************************************************/

/*
 *      rtty.h  --  RTTY "uart" implementation.
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
      
#ifndef _RTTY_H
#define _RTTY_H

/* --------------------------------------------------------------------- */

//extern void rtty_set_params(unsigned int baud, int rxinvert, int txinvert);
extern void rtty_set_params(float baud, int rxinvert, int txinvert);

extern void rtty_reset_uppercase(void);
extern void rtty_reset_figurecase(void);

extern void *mode_rtty_rx(void *dummy);
extern void *mode_rtty_tx(void *dummy);

/* --------------------------------------------------------------------- */
#endif /* _RTTY_H */
