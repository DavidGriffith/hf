/*****************************************************************************/
/*
 *      cw.c  --  CW encoder implementation.
 *	
 *	
 *      Copyright (C) 2004 Günther Montag dl4mge@darc.de 
 *	based on rtty.c by Thomas Sailer (sailer@ife.ee.ethz.ch)
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

#include <stdlib.h> 
#include <assert.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef __linux__
#include <sys/io.h>
#define IOPERM ioperm
#endif
#ifdef __FreeBSD__ 
#include <machine/cpufunc.h>
#include <machine/sysarch.h>
#define IOPERM i386_set_ioperm
#endif
#include <sys/time.h>
#include <time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#if defined(HAVE_SYS_KD_H)
#	include <sys/kd.h>			/* Linux, UnixWare */
#else /* not HAVE_SYS_KD_H */
#	if defined(HAVE_SYS_VTKD_H)
#		include <sys/vtkd.h>		/* OpenServer */
#	else /* not HAVE_SYS_VTKD_H */
#		if defined(HAVE_SYS_KBIO_H)
#			include <sys/kbio.h>		/* FreeBSD */
#		endif /* not HAVE_SYS_KBIO_H */
#	endif /* not HAVE_SYS_VTKD_H */
#endif /* not HAVE_SYS_KD_H */

#include "fskl1.h"
#include "fskutil.h"
#include "msg.h"
#include "main.h"
#include "cw.h"

/* --------------------------------------------------------------------- */


/* from cwdaemon/lib.c */
typedef struct {
const unsigned char character;	/* The character represented */
const char *representation;     /* Dot-dash shape of the character */
} cw_entry_t;

static const cw_entry_t cw_table[] = {
			/* ASCII 7bit letters */
	{ 'A', ".-"     }, { 'B', "-..."   }, { 'C', "-.-."   },
	{ 'D', "-.."    }, { 'E', "."      }, { 'F', "..-."   },
	{ 'G', "--."    }, { 'H', "...."   }, { 'I', ".."     },
	{ 'J', ".---"   }, { 'K', "-.-"    }, { 'L', ".-.."   },
	{ 'M', "--"     }, { 'N', "-."     }, { 'O', "---"    },
	{ 'P', ".--."   }, { 'Q', "--.-"   }, { 'R', ".-."    },
	{ 'S', "..."    }, { 'T', "-"      }, { 'U', "..-"    },
	{ 'V', "...-"   }, { 'W', ".--"    }, { 'X', "-..-"   },
	{ 'Y', "-.--"   }, { 'Z', "--.."   },
			/* Numerals */
	{ '0', "-----"  }, { '1', ".----"  }, { '2', "..---"  },
	{ '3', "...--"  }, { '4', "....-"  }, { '5', "....."  },
	{ '6', "-...."  }, { '7', "--..."  }, { '8', "---.."  },
	{ '9', "----."  },
			/* Punctuation */
	{ '"', ".-..-." }, { '\'',".----." }, { '$', "...-..-"},
	{ '(', "-.--."  }, { ')', "-.--.-" }, { '+', ".-.-."  },
	{ ',', "--..--" }, { '-', "-....-" }, { '.', ".-.-.-" },
	{ '/', "-..-."  }, { ':', "---..." }, { ';', "-.-.-." },
	{ '=', "-...-"  }, { '?', "..--.." }, { '_', "..--.-" },
	{ '@', ".--.-." },
			/* Cwdaemon special characters */
	{ '<', "...-.-" }, { '>', "-...-.-"}, { '!', "...-." }, 
	{ '&', ".-..."  }, { '*', ".-.-."  },
			/* ISO 8859-1 accented characters */
	{ 0334,"..--"   },	/* U with diaresis */
	{ 0304,".-.-"   },	/* A with diaeresis */
	{ 0307,"-.-.."  },	/* C with cedilla */
	{ 0326,"---."   },	/* O with diaresis */
	{ 0311,"..-.."  },	/* E with acute */
	{ 0310,".-..-"  },	/* E with grave */
	{ 0300,".--.-"  },	/* A with grave */
	{ 0321,"--.--"  },	/* N with tilde */
			/* ISO 8859-2 accented characters */
	{ 0252,"----"   },	/* S with cedilla */
	{ 0256,"--..-"  },	/* Z with dot above */
			/* Sentinel end of table value */
	{ '\0', NULL } };

char *cable = "\n"
"* * * elbug - electronic morse key by dl4mge. How to cable: * * *\n"
	"*  Middle pad: +9V battery via Resistor 2 k                     *\n"                           
	"*  Left  contact: -> DCD (9-pin plug: 1) (25-pin plug: 8)       *\n"
	"*  Right contact: -> CTS (9-pin plug: 8) (25-pin plug: 5)       *\n"
	"*  Ground: -pole battery (9-pin plug: 5) (25-pin plug: 7)       *\n"
	"*  But for now, try the mouse in the rx-window!\n"
	"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n" ;

/* --------------------------------------------------------------------- */
	
#define PAUSE 0
#define DIT 1
#define DAH 2
#define WORDPAUSE 4

char* name_spkr = "/dev/tty1";
int lastcwmsg = 0, dotus = 100000, port = 0, pin = TIOCM_RTS;
int fd_ptt, fd_spkr;
int elbug_rts, elbug_ok, spkr = 0, argp;

/* --------------------------------------------------------------------- */

static struct {
	unsigned int wpm, tone, farnsworth, dtr;
} cwp = { 12, 880, 0, 0};

void decode(int sign) 
{
	static char morse[16];
	int i;
	static int loop = 0, pause = 0, pausecount = 0, bit = 0;
	unsigned char element;

	if(!loop) {
		loop = sizeof(cw_table) / sizeof(cw_entry_t);
		printf("decode function: %d signs in table.\n", loop);
	}
	
	if (sign == DIT) element = '.';
	if (sign == DAH) element = '-';

	if (sign) {
	    pause = 0;
	    pausecount = 0;
	    morse[bit] = element;
	    bit++;
	    bufprintf(HFAPP_MSG_DATA_MONITOR, "%c", element);
	} else {
	    if (pause) return;
	    if (!pausecount) { /* compare with morse table */
	    	morse[bit] = '\0';
	    	bit = 0;
	    	for (i = 0; i < loop - 1; i++) {
	            //printf ("comparing sign %d...\n", i);
	            if(!strcmp (cw_table[i].representation, 	morse)) {
		    	//printf("%c", cw_table[i].character);
		    	//fflush(stdout);
			bufprintf(HFAPP_MSG_DATA_RECEIVE, "%c", cw_table[i].character);
	       	    	break;
		    }	       	
	   	    if (i == loop - 2) {
	       	    	//printf(" ?? "); fflush(stdout);
			bufprintf(HFAPP_MSG_DATA_RECEIVE, " ?? ");
	       	    }
	       	}
	    } 
	    pausecount++;
	    if (pausecount > WORDPAUSE) {
	        //printf(" "); fflush(stdout);
		bufprintf(HFAPP_MSG_DATA_RECEIVE, " ");
		bufprintf(HFAPP_MSG_DATA_MONITOR, " ");
	        pause = 1;
	    }
	}
}

/* --------------------------------------------------------------------- */


void wait(int us) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = us;
    select (0, NULL, NULL, NULL, &tv);
}

/* --------------------------------------------------------------------- */

void output_elbug_serial(int ptt)
{
	int status;
	
	if (fd_ptt < 0) {
	    printf("no serial port open for output.\n");
	    exit (1);
	}

	/* tone */
	if (ptt) {
		if (spkr) ioctl(fd_spkr, KIOCSOUND, argp);
	} else {
		if (spkr) ioctl(fd_spkr, KIOCSOUND, 0);
	}
	
	if (invert_ptt) ptt = !ptt;
	if (ioctl(fd_ptt, TIOCMGET, &status)) {
	    printf ("ioctl: TIOCMGET, cannot read serial port.\n");
	    if (spkr) ioctl(fd_spkr, KIOCSOUND, 0);
	    exit (1);
	}
	if (ptt) {
		status &= pin;
	} else {
		status &= ~pin;
	}
	if (ioctl(fd_ptt, TIOCMSET, &status)) {
	    printf ("ioctl: TIOCMSET, cannot write to serial port.\n");
	    if (spkr) ioctl(fd_spkr, KIOCSOUND, 0);
	    exit (1);
	}
}

/* --------------------------------------------------------------------- */

#ifdef KIOCSOUND
/* from cwlib, simplified */
console_open ()
{
	/* Open the console device file, for write only. */
	fd_spkr = open (name_spkr, O_WRONLY);
	if (fd_spkr == -1)
	    {
		printf ("console speaker can not be opened\n");
		return 0;
	    }
	
	/* Check to see if the file can do console tones. */
	if (ioctl (fd_spkr, KIOCSOUND, 0) == -1)
	    {
		close (fd_spkr);
		printf ("console speaker can not speak\n");
		return 0;
	    }

	printf ("console speaker opened.\n");
	return 1;
}
#endif

/* --------------------------------------------------------------------- */

void elbug_send_dit() {
    if (! elbug_ok) {
	printf("no serial port prepared for elbug\n");
	wait(3000);
	return;
    }
    output_elbug_serial(1);
    wait(dotus);
    output_elbug_serial(0);
    wait(dotus);
}

/* --------------------------------------------------------------------- */

void elbug_send_dah() {
    if (! elbug_ok) {
	printf("no serial port prepared for elbug\n");
	wait(3000);
	return;
    }
    output_elbug_serial(1);
    wait(dotus * 3);
    output_elbug_serial(0);
    wait(dotus);
}

/* --------------------------------------------------------------------- */

void cw_set_params(unsigned int wpm, unsigned int tone, 
    unsigned int farnsworth, unsigned int dtr)
{
	errprintf(SEV_INFO, "cw params: wpm %u, tone %d, %s\n",
		  wpm, tone, farnsworth ? " Farnsworth Spacing." : "");
	if (wpm >= 3 && wpm  <= 100)
		cwp.wpm = wpm;
	if ((tone >= 50 && tone <= 5000) || (!tone))
	cwp.tone = tone;
	cwp.dtr = !!dtr;
	cwp.farnsworth = !!farnsworth;
	if (dtr) pin = TIOCM_DTR;
}

/* --------------------------------------------------------------------- */
/*
void *mode_cw_rx(void *dummy)
{
  printf("dummy! sorry, cw rx mode is not yet implemented! See gMFSK!! \n");
}
*/
/* --------------------------------------------------------------------- */

unsigned char morse(char keypress)
{
// encodes cw, still to be stolen... 
//  return (unsigned char)42; 
// test: 42 = 2 + 8 + 32, bin 0101010, looks like morse `s`
  return (unsigned char)0; 
}

/* --------------------------------------------------------------------- */

static unsigned char cw_getcharbits(void)
{
	unsigned char bp;
//	unsigned short ch;
unsigned short dit = 2;     // 2 bits, one dot and one pause, binary: 000010
unsigned short dah = 14;    // 4 bits, 3 dots and one pause,  binary: 001110
unsigned short tune = 1;  // 1 bit, short to react quick in hand-keying
unsigned short pause = 0;   // 1 bit, short to make onset of dah/dit quick 
	for (;;) {
//	    kbd_negack();
	    /*
	     * serial elbug is better than mouse and
	     * gets priority
	     */
	    if (elbug_ok) {
		if ((inb(port + 6) & 128 )) {
		    bp = (unsigned char)dah;
		    break;
		}
		else if ((inb(port + 6) & 16)) {
		    bp = (unsigned char)dit;
	    	    break;
		}
	    }
	    /* 
	     * mouseclicks and -releases cause messages
	     * from hfterm to hfkernel
	     */
	    if (lastcwmsg == HFAPP_MSG_CW_ELBUG_DIT) {
		bp = (unsigned char)dit;
		break;
	    }
	    else if (lastcwmsg == HFAPP_MSG_CW_ELBUG_DAH) {
		bp = (unsigned char)dah;
	    	break;
	    }
	    else if (lastcwmsg == HFAPP_MSG_CW_ELBUG_TUNE) {
		bp = (unsigned char)tune;
	    	break;
	    }
	    else if (lastcwmsg == HFAPP_MSG_CW_ELBUG_PAUSE) {
		bp = (unsigned char)pause;
	    	break; 
	    }
	    else {
		bp = (unsigned char)pause;
	    	break; 
	    }

/* if pause, maybe i want to send with keyboard */
/*  maybe later i will implement this...        */
/*
	    if ((ch = kbd_get()) == KBD_EOF) {
		idlecount++;
		if (idlecount >= eof_ack_start) {
		    //bufprintf(HFAPP_MSG_DATA_STATUS, 
		    //"I kbd_ack() the EOF at %d", idlecount);
		    kbd_ack();
		    return 0xff;
	    } else idlecount = 0; 
		//something to transmit: -> reset of idlecount
	    //bufprintf(HFAPP_MSG_DATA_STATUS, "idlecount: %d", idlecount);
	    ch &= KBD_CHAR;
	    if (ch >= 'a' && ch <= 'z')
		ch -= 'a'-'A';
	    }
	    bp = morse(ch);
	    break;
*/
	}
//	kbd_ack();
	return bp;
}

/* --------------------------------------------------------------------- */

void *mode_cw_tx(void *dummy)
{
	/* duration of 1 dot, i suppose a word has 50 dots */
	/* ---> so i think 1 baud in cw is 50 * wpm / 60   */
	int i = 0, err = 0, pausecount = 0, idlewait, status, cts, dcd;

	//bufprintf(HFAPP_MSG_DATA_STATUS, "%s", cable);
	printf("%s", cable);
	
	if (name_ptt) {
	    if (! strcmp (name_ptt, "/dev/ttyS0")) {
		    port = 0x3F8;
	    }
	    if (! strcmp (name_ptt, "/dev/ttyS1")) {
		    port = 0x2F8;
	    }
	    if (! strcmp (name_ptt, "/dev/ttyS2")) {
		    port = 0x3E8;
	    }
	    if (! strcmp (name_ptt, "/dev/ttyS3")) {
		    port = 0x2E8;
	    }
	}
	if (! port) {
	    bufprintf(HFAPP_MSG_DATA_STATUS, 
		"CW: No serial port specified for elbug.");
	    printf("CW: No serial port specified for elbug.\n");
	}
	if ((err = IOPERM(port, 8, 1))) {
	    printf("CW: permission problem for serial port %04x: ioperm = %d\n", port, err);
	    printf("This program has to be called with root permissions.\n");
	}
	/*
	if (port && (fd_ptt < 0)) {
	    if ((fd_ptt = open(name_ptt, O_RDWR, 0)) < 0) {
		printf("CW: error in opening ptt %s - try another serial port.\n", 
		    name_ptt);
	    } else {
		printf("CW: opened ptt %04x\n for elbug.", port); 
	    } 
	}
	*/
	if (port && !err && fd_ptt) {
	    bufprintf(HFAPP_MSG_DATA_STATUS, 
		"CW: serial port %04x = %s prepared for elbug.", port, name_ptt);
	    bufprintf(HFAPP_MSG_DATA_STATUS, 
		"CW: output pin %s.\n", cwp.dtr ? "DTR" : "RTS");
	    printf("CW: serial port %04x = %s prepared for elbug.\n",port, name_ptt);
	    printf("CW: output pin %s.\n", cwp.dtr ? "DTR" : "RTS");
	    elbug_ok = 1;
	}
        if (cwp.tone) argp = 1193180/cwp.tone; else argp = 0;
	dotus = 1000000 / (cwp.wpm * 50 / 60);
	errprintf(SEV_INFO, "mode: cw tx\n");
	printf("CW TX: %d WPM, %d Hz, 1 dit is %d ms\n",
	    cwp.wpm, cwp.tone, dotus / 1000);
	bufprintf(HFAPP_MSG_DATA_STATUS, 
	    "CW TX: %d WPM, %d Hz, 1 dit is %d msn",
	    cwp.wpm, cwp.tone, dotus / 1000);
	    
	
#ifdef KIOCSOUND
	if (argp) spkr = console_open();
#endif        
    	
	for (;;) {
    		status = 0;
        	
		/* look for serial elbug input first */
		ioctl(fd_ptt, TIOCMGET, &status);
		cts = status & TIOCM_CTS;
		dcd = status & TIOCM_CAR;
		//printf("cts: %d, dcd: %d\n", cts, dcd);
 		if (cts) {
			elbug_send_dit();
			decode(DIT);
			continue;
		}
		if (dcd) {
			elbug_send_dah();
			decode(DAH);
			continue;
		}
		    
		/* look for mouse key input */
		if (lastcwmsg == HFAPP_MSG_CW_ELBUG_DIT) {
			elbug_send_dit();
			decode(DIT);
			continue;

		}
	    	else if (lastcwmsg == HFAPP_MSG_CW_ELBUG_DAH) {
			elbug_send_dah();
			decode(DAH);
			continue;
		}
	    	else if (lastcwmsg == HFAPP_MSG_CW_ELBUG_PAUSE) {
			decode(PAUSE);
			continue;
		}
 
		wait(3000);
		idlewait += 3000;
		if (idlewait > dotus) {
			idlewait = 0;
			decode(PAUSE);
		}
    	}
}

