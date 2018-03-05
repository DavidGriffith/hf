/*****************************************************************************/

/*
 *      main.h  --  hfkernel main header, thread-starting, 
 *	basic sound functions.
 *	By Tom Sailer, modified by Günther Montag
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

#ifndef MAIN_H
#define MAIN_H

/*DEBUG*/
#ifndef DEBUG
#define DEBUG printf("%s: function %s still running at line %d...\n", \
__FILE__, __FUNCTION__,  __LINE__);

#define D DEBUG
#endif /*DEBUG*/

/* --------------------------------------------------------------------- */

extern void errprintf(int severity, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
extern void errstr(int severity, const char *st);

void output_ptt(int ptt);
void l1_cleanup(void *dummy);
void l1_switch_to_mmap();
void l1_switch_to_nommap();
void l1_init(void);
void l1_stop(void);
void l1_start_sample(short *data, unsigned int len);
int l1_sample_finished(void);
void l1_input_samples(l1_time_t tstart, l1_time_t tinc, 
		      short *samples, unsigned int nsamples);
int l1_output_samples(l1_time_t tstart, l1_time_t tinc, 
		      short *samples, unsigned int nsamples);

/* --------------------------------------------------------------------- */

/* these global variables take hfkernel's arguments: */
extern unsigned int comm_perm;
extern const char *name_comm;
extern const char *name_audio;
extern const char *name_audio_OSS;
extern const char *name_ptt;
extern const char *name_mixer;
extern float snd_corr, tv_corr, cpu_mhz;
extern int dis_rdtsc, invert_ptt, force_halfduplex;
/**/

extern int fd_ptt, fd_mixer, fd_audio, modefamily;
extern int samples_remain;
extern int samples_count;
extern short *samples_ptr;
extern pthread_t thr_l1;
extern int mt63_data_waiting;
extern int fsk_l1_loop_running;

/* --------------------------------------------------------------------- */

#endif /* MAIN_H */
