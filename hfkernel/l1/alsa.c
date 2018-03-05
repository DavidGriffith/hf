/*****************************************************************************/

/*
 *      alsa.c  --  Linux sound full duplex I/O for hfkernel
 *
 *      Copyright (C) 2004 Günther Montag
 *	mni tnx to Jaroslaw Kysela, from his pcm.c from alsa docs
 *	i took most of the code here, could not have done it without.
 *
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
 *  This is the Linux realtime sound output driver
 */

/*****************************************************************************/
      
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include <syslog.h>

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"
#include "alsa.h"

/* --------------------------------------------------------------------- */
/*
 * Sample output
 */
/*
static int samples_remain = 0;
static int samples_count = 0;
static short *samples_ptr = NULL;
*/

/* --------------------------------------------------------------------- */

// from Jaroslaw's original pcm.c
//char *device = "plughw:0,0";			/* playback device */
snd_pcm_t *phandle;                             /* playback fd */
snd_pcm_t *chandle;                             /* capture fd */
snd_pcm_format_t format = SND_PCM_FORMAT_S16;	/* sample format */
unsigned int rate = 8000;			/* stream rate */
unsigned int channels = 1;			/* count of channels */
unsigned int buffer_time = 20000;		/* ring buffer length in us */
unsigned int period_time = 20000;		/* period time in us */
double freq = 440;				/* sinusoidal wave frequency in Hz */
int verbose = 1;				/* verbose flag */
int periods_in_buf = 4;                         /* longer buf could run smoother ?  */
                                                /* shorter than 2 did not work with my sb16 */
snd_pcm_sframes_t toms_period_size = 64;        /* from Tom's OSS driver */
snd_pcm_sframes_t buffer_size;
snd_pcm_sframes_t period_size;
snd_output_t *output = NULL;

// from Tom's oss.c, for hfkernel's time-management
unsigned long long itime = 0, otime = 0, 
inc_fragment;
l1_time_t inc_sample;
int ptt_frames;
short* in;
short* out;

/* --------------------------------------------------------------------- */

/*
static void generate_sine(const snd_pcm_channel_area_t *areas, 
			  snd_pcm_uframes_t offset,
			  snd_pcm_uframes_t count, double *_phase)
{
	static double max_phase = 2. * M_PI;
	double phase = *_phase;
	double step = max_phase*freq/(double)rate;
        // step is 1 sample
	double res;
	signed short *samples[channels];
	int steps[channels];
	int chn, ires;
	
	for (chn = 0; chn < channels; chn++) {
		if ((areas[chn].first % 8) != 0) {
			printf("areas[%i].first == %i, aborting...\n", chn, areas[chn].first);
			exit(EXIT_FAILURE);
		}
		samples[chn] = (signed short *)(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
		if ((areas[chn].step % 16) != 0) {
			printf("areas[%i].step == %i, aborting...\n", chn, areas[chn].step);
			exit(EXIT_FAILURE);
		}
		steps[chn] = areas[chn].step / 16;
		samples[chn] += offset * steps[chn];
	}
	while (count-- > 0) {
		res = sin(phase) * 32767;
		ires = res;
		for (chn = 0; chn < channels; chn++) {
			*samples[chn] = ires;
			samples[chn] += steps[chn];
		}
		phase += step;
		if (phase >= max_phase)
			phase -= max_phase;
	}
	*_phase = phase;
}
*/

/* --------------------------------------------------------------------- */
void alsa_input_samples(const snd_pcm_channel_area_t *areas, 
			  snd_pcm_uframes_t offset,
			  int count)
{
	signed short *samples[channels];
	int steps[channels];
	int chn, i;

        /* verify and prepare the contents of areas */
	for (chn = 0; chn < channels; chn++) {
		if ((areas[chn].first % 8) != 0) {
		printf("areas[%i].first == %i, aborting...\n", 
			    chn, areas[chn].first);
			exit(EXIT_FAILURE);
		}
		samples[chn] = 
		    (signed  short *)
		    (((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
		if ((areas[chn].step % 16) != 0) {
			printf("areas[%i].step == %i, aborting...\n", 
			    chn, areas[chn].step);
			exit(EXIT_FAILURE);
		}
		steps[chn] = areas[chn].step / 16;
		samples[chn] += offset * steps[chn];
	}
        
        /* printf("start of input: a period of %d frames\n", (int)count);
        it is always 64 samples. */

	for (i = 0; i < count; i++) {
        		in [i] = *samples[0];
			samples[0] += steps[0];
	}
        //printf("i");
        if (samples_remain > 0) {
	    i = 2*samples_remain;
	    if (i > 2*period_size)
		i = 2*period_size;
	    memcpy(samples_ptr, in, i);
//	    printf("\nsamples_remain: %d, samples_ptr: %p, i: %d", 
//		samples_remain, samples_ptr, i);  			
	    samples_ptr += i/2;
	    samples_remain -= i/2;
	}

        /* copy the channel areas to input buffer */
	l1_input_samples(itime >> 24, inc_sample, in, count); 
	itime += (inc_fragment);			
}

/* --------------------------------------------------------------------- */

int alsa_output_samples(const snd_pcm_channel_area_t *areas, 
			  snd_pcm_uframes_t offset,
			  int count)
{
	signed short *samples[channels];
	int steps[channels];
	int chn, res, i;

/*this would make tx start later */
/*
	int startwait = 999;
	if (startcount < startwait) {
	    startcount++;
	} else {
	    otime += inc_fragment;			
	}
*/
	otime += inc_fragment;			
        res = l1_output_samples(otime >> 24, inc_sample, 
		        (short*)out, period_size); 
	
	if (res == 0) {
	    //printf ("-");
            output_ptt(0);
	    memset(out, 0, sizeof(out));
	    /* the coming sends silence after first tx, 
               without it, buzz after first tx */
             
	    /* verify and prepare the contents of areas */
	    for (chn = 0; chn < channels; chn++) {
		if ((areas[chn].first % 8) != 0) {
			printf("areas[%i].first == %i, aborting...\n", chn, areas[chn].first);
			exit(EXIT_FAILURE);
		}
		samples[chn] = (signed short *)(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
		if ((areas[chn].step % 16) != 0) {
			printf("areas[%i].step == %i, aborting...\n", chn, areas[chn].step);
			exit(EXIT_FAILURE);
		}
		steps[chn] = areas[chn].step / 16;
		samples[chn] += offset * steps[chn];
	    }
	    /* fill the channel areas */
	    while (count-- > 0) {
		for (chn = 0; chn < channels; chn++) {
			*samples[chn] = 0;
			samples[chn] += steps[chn];
		}
	    }
	    return 0;
	}

	if (res == 1) 
	    //printf ("o");
            output_ptt(1);
	/* verify and prepare the contents of areas */
	for (chn = 0; chn < channels; chn++) {
		if ((areas[chn].first % 8) != 0) {
		printf("areas[%i].first == %i, aborting...\n", 
			    chn, areas[chn].first);
			exit(EXIT_FAILURE);
		}
		samples[chn] = 
		    (signed short *)
		    (((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
		if ((areas[chn].step % 16) != 0) {
			printf("areas[%i].step == %i, aborting...\n", 
			    chn, areas[chn].step);
			exit(EXIT_FAILURE);
		}
		steps[chn] = areas[chn].step / 16;
		samples[chn] += offset * steps[chn];
	}
	/* fill the channel areas */
	for (i = 0; i < count; i++) {
		for (chn = 0; chn < channels; chn++) {
			*samples[chn] = out[i];
			samples[chn] += steps[chn];
		}
	}
	return res;
}

/* --------------------------------------------------------------------- */

static int set_hwparams(snd_pcm_t *handle,
			snd_pcm_hw_params_t *params,
			snd_pcm_access_t access)
{
	unsigned int rrate;
	int err, dir;

	/* choose all parameters */
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		printf("Broken configuration : no configurations available: %s\n", snd_strerror(err));
		return err;
	}
	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, params, access);
	if (err < 0) {
		printf("Access type not available : %s\n", snd_strerror(err));
		return err;
	}
	/* set the sample format */
	err = snd_pcm_hw_params_set_format(handle, params, format);
	if (err < 0) {
		printf("Sample format not available : %s\n", snd_strerror(err));
		return err;
	}
	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0) {
		printf("Channels count (%i) not available s: %s\n", channels, snd_strerror(err));
		return err;
	}
	/* set the stream rate */
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
	if (err < 0) {
		printf("Rate %iHz not available : %s\n", rate, snd_strerror(err));
		return err;
	}
	if (rrate != rate) {
		printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
		return -EINVAL;
	}
        /* set the buffer size */
        buffer_size = toms_period_size * periods_in_buf;
        err = snd_pcm_hw_params_set_buffer_size_near
            (handle, params, &buffer_size);
	if (err < 0) {
		printf("Unable to set buffer size %li : %s\n", 
                    period_size, snd_strerror(err));
		return err;
	}
	err = snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (err < 0) {
		printf("Unable to get buffer size: %s\n", 
                    snd_strerror(err));
		return err;
	}
        /* set the period size */
        period_size = buffer_size / periods_in_buf;
	err = snd_pcm_hw_params_set_period_size_near
            (handle, params, &period_size, 0);
	if (err < 0) {
		printf("Unable to set period size %li : %s\n", 
                    period_size, snd_strerror(err));
		return err;
	}
	err = snd_pcm_hw_params_get_period_size(params, &period_size, &dir);
	if (err < 0) {
		printf("Unable to get period size: %s\n", 
                    snd_strerror(err));
		return err;
	}
	/* write the parameters to device */
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		printf("Unable to set hw params : %s\n", snd_strerror(err));
		return err;
	}
	inc_sample = 1000000.0*snd_corr/rate;
	inc_fragment = period_size*1000000.0*snd_corr/rate*(1<<24);
	return 0;
}

/* --------------------------------------------------------------------- */

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
	int err;

	/* get the current swparams */
	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		printf("Unable to determine current swparams : %s\n", snd_strerror(err));
		return err;
	}
	/* start the transfer when the buffer is almost full: */
	/* (buffer_size / avail_min) * avail_min */
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
	if (err < 0) {
		printf("Unable to set start threshold mode : %s\n", snd_strerror(err));
		return err;
	}
	/* allow the transfer when at least period_size samples can be processed */
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
	if (err < 0) {
		printf("Unable to set avail min : %s\n", snd_strerror(err));
		return err;
	}
	/* align all transfers to 1 sample */
	err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
	if (err < 0) {
		printf("Unable to set transfer align : %s\n", snd_strerror(err));
		return err;
	}
	/* write the parameters to the playback device */
	err = snd_pcm_sw_params(handle, swparams);
	if (err < 0) {
		printf("Unable to set sw params : %s\n", snd_strerror(err));
		return err;
	}
	return 0;
}

/* --------------------------------------------------------------------- */

/*
 *   Underrun and suspend recovery
 */
 
static int xrun_recovery(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE) {	/* under-run */
		err = snd_pcm_prepare(handle);
		if (err < 0)
			printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			sleep(1);	/* wait until the suspend flag is released */
		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0)
				printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}
	return err;
}

/* --------------------------------------------------------------------- */

/*
 *   Transfer method - direct write only
 */
/*
static int direct_loop(snd_pcm_t *handle,
		       signed short *samples,
		       snd_pcm_channel_area_t *areas)
*/
static int direct_loop()
{
	const snd_pcm_channel_area_t *my_playback_areas, *my_capture_areas;
	snd_pcm_uframes_t poffset, coffset, pframes, cframes, psize, csize;
	snd_pcm_sframes_t avail, commitres;
	snd_pcm_state_t state;
	int err, cfirst = 1, pfirst = 1;
//        double phase = 0;  /* for generate_sine() */
        printf("alsa fdx: direct_loop starts\n");
	while (1) {

// playback
		state = snd_pcm_state(phandle);
                if (state == SND_PCM_STATE_XRUN) {
			err = xrun_recovery(phandle, -EPIPE);
			if (err < 0) {
				printf("XRUN recovery failed: %s\n", 
				    snd_strerror(err));
				return err;
			}
			pfirst = 1;
		} else if (state == SND_PCM_STATE_SUSPENDED) {
			err = xrun_recovery(phandle, -ESTRPIPE);
			if (err < 0) {
				printf("SUSPEND recovery failed: %s\n", 
				    snd_strerror(err));
				return err;
			}
		}
		avail = snd_pcm_avail_update(phandle);
		if (avail < 0) {
			err = xrun_recovery(phandle, avail);
			if (err < 0) {
				printf("avail update failed: %s\n", 
				    snd_strerror(err));
				return err;
			}
			pfirst = 1;
			continue;
		}
		if (avail < period_size) {
			if (pfirst) {
				pfirst = 0;
				err = snd_pcm_start(phandle);
				if (err < 0) {
					printf("Start error: %s\n", 
					    snd_strerror(err));
					exit(EXIT_FAILURE);
				}
			} else {
				err = snd_pcm_wait(phandle, -1);
				if (err < 0) {
					if ((err = xrun_recovery(phandle, err)) < 0) {
						printf("snd_pcm_wait error: %s\n", 
						    snd_strerror(err));
						exit(EXIT_FAILURE);
					}
					pfirst = 1;
				}
			}
			continue;
		}
		psize = period_size;
		while (psize > 0) {
			pframes = psize;
			err = snd_pcm_mmap_begin
			    (phandle, &my_playback_areas, &poffset, &pframes);
			if (err < 0) {
				if ((err = xrun_recovery(phandle, err)) < 0) {
					printf("MMAP begin avail error: %s\n", 
					    snd_strerror(err));
					exit(EXIT_FAILURE);
				}
				pfirst = 1;
			}
//			generate_sine(my_playback_areas, poffset, pframes, &phase);
			alsa_output_samples
			    (my_playback_areas, poffset, pframes);
			commitres = snd_pcm_mmap_commit
			    (phandle, poffset, pframes);
			if (commitres < 0 || commitres != pframes) {
				if ((err = xrun_recovery(phandle, commitres 
				    >= 0 ? -EPIPE : commitres)) < 0) {
					printf("MMAP commit error: %s\n", 
					    snd_strerror(err));
					exit(EXIT_FAILURE);
				}
				pfirst = 1;
			}
			psize -= pframes;
		}
// capture 
		state = snd_pcm_state(chandle);
		if (state == SND_PCM_STATE_XRUN) {
			err = xrun_recovery(chandle, -EPIPE);
			if (err < 0) {
				printf("XRUN recovery failed: %s\n", 
				    snd_strerror(err));
				return err;
			}
			cfirst = 1;
		} else if (state == SND_PCM_STATE_SUSPENDED) {
			err = xrun_recovery(chandle, -ESTRPIPE);
			if (err < 0) {
				printf("SUSPEND recovery failed: %s\n", 
				    snd_strerror(err));
				return err;
			}
		}
		avail = snd_pcm_avail_update(chandle);
		if (avail < 0) {
			err = xrun_recovery(chandle, avail);
			if (err < 0) {
				printf("avail update failed: %s\n", 
				    snd_strerror(err));
				return err;
			}
			cfirst = 1;
			continue;
		}
		if (avail < period_size) {
			if (cfirst) {
				cfirst = 0;
				err = snd_pcm_start(chandle);
				if (err < 0) {
					printf("Start error: %s\n", 
					    snd_strerror(err));
					exit(EXIT_FAILURE);
				}
			} else {
				err = snd_pcm_wait(chandle, -1);
				if (err < 0) {
					if ((err = xrun_recovery(chandle, err)) < 0) {
						printf("snd_pcm_wait error: %s\n", 
						    snd_strerror(err));
						exit(EXIT_FAILURE);
					}
					cfirst = 1;
				}
			}
			continue;
		}
		csize = period_size;
		while (csize > 0) {
			cframes = csize;
			err = snd_pcm_mmap_begin
			    (chandle, &my_capture_areas, &coffset, &cframes);
			if (err < 0) {
				if ((err = xrun_recovery(chandle, err)) < 0) {
				    printf("MMAP begin avail error: %s\n", 
				        snd_strerror(err));
				    exit(EXIT_FAILURE);
				}
				cfirst = 1;
			}
			alsa_input_samples(my_capture_areas, coffset, cframes);
                	commitres = snd_pcm_mmap_commit
			    (chandle, coffset, cframes);
			if (commitres < 0 || commitres != cframes) {
				if ((err = xrun_recovery
				    (chandle, commitres 
					>= 0 ? -EPIPE : commitres)) < 0) {
					printf("MMAP commit error: %s\n", 
					    snd_strerror(err));
					exit(EXIT_FAILURE);
				}
				cfirst = 1;
			}
			csize -= cframes;
		}
	}
}

/* --------------------------------------------------------------------- */

struct transfer_method {
	const char *name;
	snd_pcm_access_t access;
        int (*transfer_loop)();
/*
	int (*transfer_loop)(snd_pcm_t *handle,
			     signed short *samples,
			     snd_pcm_channel_area_t *areas);
*/
};

static struct transfer_method transfer_methods[] = {
//	{ "write", SND_PCM_ACCESS_RW_INTERLEAVED, write_loop },
//	{ "write_and_poll", SND_PCM_ACCESS_RW_INTERLEAVED, write_and_poll_loop },
//	{ "async", SND_PCM_ACCESS_RW_INTERLEAVED, async_loop },
//	{ "async_direct", SND_PCM_ACCESS_MMAP_INTERLEAVED, async_direct_loop },
//	{ "direct_interleaved", SND_PCM_ACCESS_MMAP_INTERLEAVED, direct_loop },
	{ "direct_noninterleaved", SND_PCM_ACCESS_MMAP_NONINTERLEAVED, direct_loop },
//	{ "direct_write", SND_PCM_ACCESS_MMAP_INTERLEAVED, direct_write_loop },
//	{ NULL, SND_PCM_ACCESS_RW_INTERLEAVED, NULL }
};

/* --------------------------------------------------------------------- */

void *alsa_fdx_driver(void *name_audio)
{
        int err;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
	int method = 0;
	//signed short *samples;
	//int chn;
	//snd_pcm_channel_area_t *areas;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);
	
	verbose = 1;

	err = snd_output_stdio_attach(&output, stdout, 0);
	if (err < 0) {
		printf("Output failed: %s\n", snd_strerror(err));
		return 0;
	}
	printf("Playback device is %s\n", (char*)name_audio);
	printf("Stream parameters are %iHz, %s, %i channels\n", 
	    rate, snd_pcm_format_name(format), channels);
	printf("Sine wave rate is %.4fHz\n", freq);
	printf("Using transfer method: %s\n", 
	    transfer_methods[method].name);

	if ((err = snd_pcm_open(&chandle, name_audio, SND_PCM_STREAM_CAPTURE, 0)) 
	    < 0) {
		printf("Capture open error: %s\n", snd_strerror(err));
		return 0;
	}
	if ((err = snd_pcm_open(&phandle, name_audio, SND_PCM_STREAM_PLAYBACK, 0)) 
	    < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		return 0;
	}
	if ((err = set_hwparams
	    (chandle, hwparams, transfer_methods[method].access)) 
	    < 0) {
		printf("Setting of hwparams for capture failed: %s\n", 
                    snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = set_swparams(chandle, swparams)) < 0) {
		printf("Setting of swparams for capture failed: %s\n", 
                    snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = set_hwparams(phandle, hwparams, 
	    transfer_methods[method].access)) 
	    < 0) {
		printf("Setting of hwparams for playback failed: %s\n", 
                    snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = set_swparams(phandle, swparams)) < 0) {
		printf("Setting of swparams for playback failed: %s\n", 
                    snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if (verbose > 0) {
		snd_pcm_dump(chandle, output);
        	snd_pcm_dump(phandle, output);                
        }

	in = (short*)malloc(period_size * 2);
	out = (short*)malloc(period_size * 2);
	otime = inc_fragment;
	itime = 0;
 	err = transfer_methods[method].transfer_loop();
	if (err < 0)
		printf("Transfer failed: %s\n", snd_strerror(err));

	free(in);
	free(out);
	snd_pcm_close(chandle);
	snd_pcm_close(phandle);
	return NULL;
}


/* --------------------------------------------------------------------- */

/*
static void *alsa_nommap_hdx_driver(void *name_audio)
{
// dummy
	errstr(SEV_FATAL, 
	    "alsa_nommap_hdx_driver: this function is being developed, "
	    "but not yet implemented. "
	    "Please look at the hftermhackers mailinglist!");
	return(0);
}
*/
/* --------------------------------------------------------------------- */
