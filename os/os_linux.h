/*****************************************************************************/

/*
 *      linux.h  --  Linux specific I/O header file.
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
      
#ifndef _LINUX_H
#define _LINUX_H

/* --------------------------------------------------------------------- */

#include "os.h"
#include "hfapp.h"
#include <pthread.h>

/* --------------------------------------------------------------------- */

extern void *io_process(void *dummy);
extern void *l1_process(void *dummy);
extern void process_sample_msg(unsigned int u);
extern void check_sample_msg_ready(void);

/* --------------------------------------------------------------------- */

extern int invert_ptt;
extern unsigned int comm_perm;
extern const char *name_comm;
extern const char *name_audio;
extern const char *name_ptt;
extern pthread_t thr_l2;

/* --------------------------------------------------------------------- */
#endif /* _LINUX_H */
