/*****************************************************************************/

/*
 *      dcf77gen.c  --  Linux soundcard DCF77 signal generator.
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dcf77.h"

/* --------------------------------------------------------------------- */

int lsb_mode = 1;

/* --------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
        int c;
        int err = 0;
	time_t stime;
	void (*a_out)(const char *, const struct modulator *) = audio_out;
	const char *name_audio = NULL;
	const struct modulator *mod = &dcf77_modulator;
	unsigned int zone = 0;

	stime = time(NULL);
        while ((c = getopt(argc, argv, "a:snzv:glu")) != -1) 
                switch (c) {
                case 'a':
			a_out = audio_out;
                        name_audio = optarg;
                        break;

                case 's':
			a_out = audio_stdout;
                        break;

		case 'z':
			zone = 1;
			break;

                case 'n':
			a_out = oss_nommap_audio_out;
                        break;

		case 'v':
			verboselevel = strtoul(optarg, NULL, 0);
			break;

		case 'g':
			mod = &hbg_modulator;
			break;

		case 'l':
			lsb_mode = 1;
			break;

		case 'u':
			lsb_mode = 0;
			break;

                default:
                        err++;
                        break;
                }
        if (err) {
                fprintf(stderr, "usage: dcf77gen [-a <audio device>] [-g] [-l] [-n] [-s] [-u] [-v <vl>] [-z]\n\n"
                        "  -a: audio device path (default: /dev/dsp)\n"
			"  -l: LSB mode\n"
			"  -n: no mmap() (experimental!)"
                        "  -g: HBG modulator\n"		
                        "  -s: stdout\n"
			"  -u: USB mode\n"
                        "  -v: verboselevel (1 or 2)\n"
                        "  -z: MESZ\n");
                exit(1);
        }
	timeenc_init(stime, zone, 0, 0, 0);
	a_out(name_audio, mod);
        exit(0);
}

/* --------------------------------------------------------------------- */
