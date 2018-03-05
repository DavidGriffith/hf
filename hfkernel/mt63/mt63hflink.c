/*
 *	mt63hflink.c  --  by Gnther Montag DL4MGE
 *	"glue" - links Pawel Jalocha's MT63 code to
 *	Tom Sailer's hf code		
 *	for integration of MT63 into hf
 *		
 *
 *    hf and MT63 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    hf & MT63 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with MT63; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*DEBUG*/
#ifndef DEBUG
#define DEBUG printf("%s: function %s still running at line %d...\n", \
__FILE__, __FUNCTION__,  __LINE__);

#define D DEBUG
#endif /*DEBUG*/

#include <stdio.h>
#include <sys/stat.h>
//#include <fcntl.h> makes conflicts with types in msg.h
#include <errno.h>
#include <ctype.h>
#include <syslog.h>
#include <unistd.h>
#include <pthread.h>

#include "mt63hflink.h"
#include "mt63hf.h"
#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"
#include "oss.h"
#ifdef HAVE_ALSA_ASOUNDLIB_H
#include "alsa.h"
#endif /* HAVE_ALSA_ASOUNDLIB_H */

// ============================================================================

struct mt63p mt63p = { 1000, 32, "MT63", 0 };

// ============================================================================

short mt63rxbuf[MT63RXBUFLEN];
short mt63txbuf[MT63TXBUFLEN];
int mt63rxbuf_written, mt63rxbuf_read;
int mt63txbuf_written, mt63txbuf_read;
short encodebuf[ENCODEBUFSIZE];
int encodelen;
int mt63_data_waiting;
pthread_mutex_t mt63_inputmut  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  mt63_inputcond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mt63_outputmut  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  mt63_outputcond = PTHREAD_COND_INITIALIZER;
int switched_to_nommap = 0;

// ============================================================================

void mt63_set_params(unsigned int bandwidth, unsigned int integration,
    const unsigned char* cwcall, unsigned int doubleinterleave)
{
        errprintf(SEV_INFO, 
	    "mt63 params: bandwidth %u Hz with %s interleave for tx, \n"
	    "the time/frequency synchronizer integrates over %u symbols,\n"
	    "at tx the call %s is sent in cw as background.\n",
	    bandwidth, doubleinterleave ? "DOUBLE (64)" : "SINGLE (32)",
	    integration, cwcall);
	mt63p.bandwidth = bandwidth;
	mt63p.doubleinterleave = doubleinterleave;
	mt63p.integration = integration;
	sprintf(mt63p.cwcall, "%s", cwcall);
}

/* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- */

static void mt63_inputcleanup(void *dummy) 
{
	printf("mt63_inputcleanup: I will try to close sound device.\n");
	mt63_finish_rx();
	close_audio_rx();
}
 
/* --------------------------------------------------------------------- */

static void mt63_outputcleanup(void *dummy)  
{
	printf("mt63_outputcleanup: I will transmit rest data and close sound device.\n");
	mt63_finish_tx();
	mt63_send_jam();
	close_audio_tx();
	printf("mt63 tx finished ...\n");
}

/* --------------------------------------------------------------------- */
 
void *mode_mt63_rx(void *dummy)
{
	int i;
	
	errprintf(SEV_INFO, "mode: mt63 rx\n");
	send_short_msg(HFAPP_MSG_STATE_MT63_RX, ERR_NOERR);

	/* prepare cleanup */
	pthread_cleanup_push(mt63_inputcleanup, NULL);

	/* initialize mt36 params */
	mt63_rx_start
	    (mt63p.bandwidth, mt63p.doubleinterleave, 
		mt63p.integration, snd_corr);

	/* messages */
	bufprintf(HFAPP_MSG_DATA_STATUS, 
	    "MT63 RX.\nBandwidth %u, interleave: %s,\n"
	    "integration: %u -> Please wait %1.1f seconds ... ", 
	    mt63p.bandwidth, 
	    mt63p.doubleinterleave ? "DOUBLE (64)" : "SINGLE (32)",
	    mt63p.integration, 
	    mt63p.integration / 10.0);

	printf("MT63 RX.\nBandwidth %u, interleave: %s,\n"
	    "integration: %u -> Please wait %1.1f seconds ... ", 
	    mt63p.bandwidth, 
	    mt63p.doubleinterleave ? "DOUBLE (64)" : "SINGLE (32)",
	    mt63p.integration, 
	    mt63p.integration / 10.0);

	/* open sound card with Pawel`s OSS code */
	printf("I will open audio for mt63 rx...\n");
          if ( ! (open_audio_input(name_audio_OSS)))
	    printf("audio opened for mt63 rx.\n");
  	
	for(;;) {
	    mt63_rx();
	    //mt63_decode(code);
	    //printf(/*MT63 rx data in 'code':*/"%s", code);
	    
	    for(i=0; i<strlen(code); i++) { 
		if((code[i]>=' ')||(code[i]==0x08)||(code[i]==0x09)) {
		    bufprintf(HFAPP_MSG_DATA_RECEIVE, "%c", code[i]);
		    //bufprintf(HFAPP_MSG_DATA_STATUS, "%c", code[i]);
		    bufprintf(HFAPP_MSG_DATA_MONITOR, "%c",code[i]);
		    //printf("%c",code[i]);
		} else if ((code[i] == 10) || (code[i] == 13))  {//'\r') {
            	    code[i] = '\n'; //outcomment = test for mailbox
		    bufprintf(HFAPP_MSG_DATA_RECEIVE, "%c", code[i]);
		    bufprintf(HFAPP_MSG_DATA_MONITOR, "%c", code[i]);
		    //printf("%c", code[i]);
		} else if(code[i]!='\0') {
		    //bufprintf(HFAPP_MSG_DATA_MONITOR, "<%02x> ", code[i]);
		    bufprintf(HFAPP_MSG_DATA_MONITOR, "<%d>", (int)code[i]);
                    //printf("<%02X> ",code[i]); //orig !!
            	    //printf("<%d>", (int)code[i]); //test  !!
		}
            }
	    fflush(stdout);
	}
	    pthread_cleanup_pop(0);
}

/* --------------------------------------------------------------------- */

unsigned char mt63_getcharbits(void)
{
	unsigned char *bp;
	unsigned short ch;
	static int idlecount = 0; 
	int eof_ack_start =  64;
	
	kbd_negack();
	if ((ch = kbd_get()) == KBD_EOF) {
	    /*  We wait with giving the first kbd_ack() for eof_ack_start times `KBD_EOF` 
		to prevent a race condition between the autorx() function of hfterm 
		(in rxtx.c) and the real transmit here which lasts some time.
	    */	   
    
	    idlecount++;
	    //   bufprintf(HFAPP_MSG_DATA_STATUS, 
	    //	"mt63_getcharbits: KBD_EOF nr. %d", idlecount);
	    //
	    if (idlecount >= eof_ack_start) {
	    //	bufprintf(HFAPP_MSG_DATA_STATUS, 
	    //    "mt63_getcharbits: I kbd_ack() the EOF at %d", idlecount);
	    	kbd_ack();
	    	idlecount = 0;
	    }
	    return 0; //0xff;
	}  else idlecount = 0; //something to transmit: -> reset of idlecount
	ch &= KBD_CHAR;
	bp = (unsigned char)ch;
	//printf ("mt63_getcharbits: %c\n", bp);
	kbd_ack();
	return bp;
}

/* --------------------------------------------------------------------- */

void *mode_mt63_tx(void *dummy)
{
	char txchar;

	/* prepare cleanup*/
	pthread_cleanup_push(mt63_outputcleanup, NULL);

	/* initialize mt36 params */
	mt63_tx_start (mt63p.bandwidth, mt63p.doubleinterleave, 
		snd_corr, mt63p.cwcall);

	/* messages */
	errprintf(SEV_INFO, "mode: mt63 tx\n");
	bufprintf(HFAPP_MSG_DATA_STATUS, 
	    "MT63 TX:\nBandwidth: %u, interleave: %s, \nCW id: %s",
	    mt63p.bandwidth, 
	    mt63p.doubleinterleave ? "DOUBLE (64)" : "SINGLE (32)",
	    mt63p.cwcall);
	send_short_msg(HFAPP_MSG_STATE_MT63_TX, ERR_NOERR);
	
	/* open sound card with Pawel`s OSS code */
	printf("will open audio for mt63 tx...\n");
          if (! (open_audio_output(name_audio_OSS)))
	    printf("audio opened for mt63 tx.\n");
  
	for(;;) {
	    //mt63_getcharbits();
	    txchar = mt63_getcharbits();
	    //txchar = 'f';
	    //txchar = '\0';
	    //bufprintf(HFAPP_MSG_DATA_MONITOR, 
	    //	"%02d ", (int)((txchar >> 1) & 0x1f));
	    //printf ("%c", txchar);
		mt63_tx(txchar);
		// to not too often look for new input
		usleep(10000);
	}
	pthread_cleanup_pop(0);
}

/* --------------------------------------------------------------------- */ 
