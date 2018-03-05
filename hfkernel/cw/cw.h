/*****************************************************************************/

/*
 *      cw.h  --  CW mouse-elbug implementation.
 *	By Günther Montag, dl4mge, with most of the code from 
 *      rtty.h and hf package by Thomas Sailer (sailer@ife.ee.ethz.ch)
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
      
#ifndef _CW_H
#define _CW_H

/* --------------------------------------------------------------------- */

extern void cw_set_params(unsigned int wpm, unsigned int tone, 
    unsigned int farnsworth, unsigned int dtr);
extern void *mode_cw_rx(void *dummy);
extern void *mode_cw_tx(void *dummy);

extern int lastcwmsg;

/* --------------------------------------------------------------------- */

#endif /* _CW_H */

