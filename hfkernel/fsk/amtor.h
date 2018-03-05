/*****************************************************************************/

/*
 *      amtor.h  --  Amtor protocol.
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
      
#ifndef _AMTOR_H
#define _AMTOR_H

/* --------------------------------------------------------------------- */

extern int amtor_monitor_arq(l1_time_t tm, unsigned char pk01, unsigned char pk02, 
			     unsigned char pk03, unsigned char pk11, unsigned char pk12,
			     unsigned char pk13, l1_soft_t *samples, int freqdev);
extern int amtor_monitor_fec(l1_time_t tm, unsigned char rx0, unsigned char dx0,
			     unsigned char rx1, unsigned char dx1,
			     unsigned char rx2, unsigned char dx2,
			     unsigned char rx3, unsigned char dx3,
			     unsigned char rx4, unsigned char dx4,
			     unsigned char rx5, unsigned char dx5, l1_soft_t *samples, int freqdev);
extern void amtor_monitor_init(l1_time_t tm);

/* --------------------------------------------------------------------- */

extern void amtor_set_params(const unsigned char *destcall, const unsigned char *selfeccall,
			     const unsigned char *mycall, unsigned int txdelay,
			     unsigned int retry, int rxinvert, int txinvert);

extern void amtor_mode_qrt(void);
extern void amtor_mode_irs(void);
extern void amtor_mode_iss(void);

extern void amtor_reset_uppercase(void);
extern void amtor_reset_lowercase(void);
extern void amtor_reset_figurecase(void);

extern void *mode_amtor_arq(void *dummy);
extern void *mode_amtor_colfec(void *dummy);
extern void *mode_amtor_selfec(void *dummy);

/* --------------------------------------------------------------------- */
#endif /* _AMTOR_H */
