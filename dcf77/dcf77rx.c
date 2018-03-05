/*****************************************************************************/

/*
 *      dcf77rx.c  --  Linux soundcard DCF77 receiver.
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
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dcf77.h"

/* --------------------------------------------------------------------- */

int lsb_mode = 1;
int use_mmap = 1;
int set_time = 0;

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
        int c;
        int err = 0;
	void (*a_in)(const char *, const struct demodulator *) = audio_in;
	const char *name_audio = NULL;
	const struct demodulator *dem = &dcf77_demodulator_ampl;

        while ((c = getopt(argc, argv, "a:snv:gpultu")) != -1)  {
                switch (c) {
                case 'a':
			a_in = audio_in;
                        name_audio = optarg;
                        break;

                case 's':
			a_in = audio_stdin;
                        break;

		case 'n':
			use_mmap = 0;
			a_in = oss_nommap_audio_in;
			break;

		case 'v':
			verboselevel = strtoul(optarg, NULL, 0);
			break;

		case 'g':
			dem = &hbg_demodulator;
			break;

		case 'p':
			dem = &dcf77_demodulator_pn;
			break;

		case 'u':
			lsb_mode = 0;
			break;

		case 'l':
			lsb_mode = 1;
			break;

		case 't':
			set_time = 1;
			break;

		default:
                        err++;
                        break;
                }
	}

        if (err) {
                fprintf(stderr, "usage: dcf77rx [-a <audio device>] [-g] [-l] [-n] [-p]  [-s] [-t] [-u] [-v <vl>]\n\n"
			"  -a: audio device path (default: /dev/dsp)\n"
			"  -s: input from standard in\n"
			"  -n: no mmap() (experimental! For mmap-unable soundcards)\n"			
			"  -v: verboselevel (1 or 2)\n"
			"  -g: HBG mode\n"
			"  -p: DCF77 BPSK PN mode\n"
			"  -u: USB mode\n"
			"  -l: LSB mode\n"
			"  -t: set system time, only MESZ for now\n" );
                exit(1);
        }
	printf("dcf77 time decoding program by Tom Sailer\n");
	a_in(name_audio, dem);
	exit(0);
}

/* --------------------------------------------------------------------- */
