/*****************************************************************************/

/*
 *      pactor.h  --  HF standby mode (search for pactor/amtor calling).
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
      
#ifndef _PACTOR_H
#define _PACTOR_H

/* --------------------------------------------------------------------- */
/*
 * routines for use by the standby stuff
 */

extern void pactor_monitor_200(unsigned char *pkt, l1_time_t tm);
extern void pactor_monitor_100(unsigned char *pkt, l1_time_t tm);
extern void pactor_monitor_init(l1_time_t tm);
extern int pactor_check_call(unsigned char *pkt, l1_soft_t *s, l1_time_t tm, int freqdev);

/* --------------------------------------------------------------------- */

extern void pactor_set_params(const char *destcall, const char *mycall, int txd, int retry, int lp,
			      unsigned int crc100chg, unsigned int crc100, unsigned int crc200chg, 
			      unsigned int crc200);
extern void *mode_pactor_arq(void *dummy);
extern void *mode_pactor_fec(void *dummy);

extern void pactor_mode_qrt(void);
extern void pactor_mode_irs(void);
extern void pactor_mode_iss(void);
extern void pactor_mode_speedup(void);

/* --------------------------------------------------------------------- */
#endif /* _PACTOR_H */
