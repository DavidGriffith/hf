/*****************************************************************************/
/*
 *      mailbox.c  --  mailbox-via-tcp-port feature for hfterm.
 *
 *	Can be used also for other ham radio programs,
 *	why not also making a psk31-mailbox!?
 *
 *
 * 	2004 by Günther Montag DL4MGE
 *      some ideas taken from select_tut man page.
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

#include "mailbox.h"
#include "hft.h"
#include <sys/socket.h>
#include <netinet/in.h>
// if still error `SHUT_RDWR undeclared' try #include <arpa/inet.h>

/* --------------------------------------------------------------------- */

#define SHUT_mailbox_fd {		              		\
               if (mailbox_fd >= 0) {                		\
                   shutdown (mailbox_fd, SHUT_RDWR);  		\
                   close (mailbox_fd);                		\
                   mailbox_fd = -1;                   		\
               }                               			\
           }

/* --------------------------------------------------------------------- */

#define MAILBOX_IN_DATA_LEN 512
int mailbox_on = 0, mailbox_connected = 0, mailbox_fd = -1;
int mailbox_pactor_login = 0, mailbox_output_waiting = 0;
int port, txwaitbytes  = 0;
int mailbox_input_waiting = 0;
char *ipaddress;
// these two are options to hfterm
// note in f6fbb doc examples the port is 6300, hex 189C
char buf_hf2box[BUF_SIZE], buf_box2hf[BUF_SIZE];
// buf_hf2box what we received and hfterm sends to mailbox prog = rx
// buf_box2hf what hfterm's port reads from mailbox prog and will send = tx
int buf_box2hf_avail = 0, in_oldavail = 0, buf_box2hf_written = 0;
int buf_hf2box_avail = 0, out_oldavail = 0, buf_hf2box_written = 0;
// avail is end of content, written is what has been processed

/* --------------------------------------------------------------------- */

void millisleep (int milliseconds) {
	struct timespec my_time_spec_nano;
	my_time_spec_nano.tv_sec = 0;
	my_time_spec_nano.tv_nsec = milliseconds * 1000000;   	
//	my_time_spec_nano.tv_nsec = 200000000;  // 200 ms 	
//	display_status 
//	    ( "have a little sleep of %d milliseconds....",
//	    my_time_spec_nano.tv_nsec / 1000000);
	nanosleep(&my_time_spec_nano, NULL);	
//	display_status 
//	    ( "end of sleep, had a fine dream....");
}

/* --------------------------------------------------------------------- */

void mailbox_input(unsigned char *data, int datalen) {
/* 
 * writes received data to input buffer  for mailbox.
 *
 * If mailbox_on == 1, 
 * this function is invoked by main.c: write_input
 * and also (as a mailbox test feature)
 * by just writing something to the rx window.
 *
 * If remote user has not yet logged in, it prepares the call
 * for check. 
 *
 * If last character of input is \n (newline) or \r (carriage return)
 * mailbox_check_io() is invoked to start dialog with the mailbox program 
 * via the telnet port.
 *
 */
	int i = 0;

	char c = 0, call[16];
	unsigned char mailbox_in_data[MAILBOX_IN_DATA_LEN];

	if (!mailbox_on) {
	    display_status (   
	     "mailbox_rx_2_inbuf: mailbox mode off. ");
	    return;
	}
/*
 *  provide that we are not called if data are still being processed
 *  on their way to mailbox program or within it ...
 */	
	if (mailbox_input_waiting) {
	    display_status (   
	     "mailbox_input: last input still in process \n"
	     "-> I don't accept new input for now.");
	    return;
	}
/*
 *  provide that we are not called if data are still being processed
 *  on their way from mailbox program to transmit, which is artifically 
 *  delayed so that remote user can switch from tx to rx ...
 */	
	if (mailbox_output_waiting) {
	    display_status (   
	     "mailbox_input: output still waiting for transmit \n"
	     "-> I don't accept new input for now.");
	    return;
	}
/*
 *  first copy data into our own buffer, 
 *  so that the original data is not 'toupperized' in case we use the 
 *  call detection machine. 
 */	
	if (datalen >= MAILBOX_IN_DATA_LEN) {
	    display_status (   
	     "mailbox_input: buffer too short. Will cut end of data.");
	}
	memset(mailbox_in_data, 0, sizeof (mailbox_in_data));
	for (i = 0;  i < datalen && i < MAILBOX_IN_DATA_LEN; i++) {
	    mailbox_in_data[i] = (char)data[i];
	}

/*
 *  to make the check easier, if our call is called, toupperize input 
 */	
	if (! mailbox_connected) {
	    /*
	    display_status (
		//"Mailbox not yet connected.\n"
		"Capitalizing input to simplify check for my call...\n"       		
		"%s", mailbox_in_data);
	    */
	    for (i = 0; i < datalen && i < BUF_SIZE - buf_hf2box_avail; i++) {
		if (islower(mailbox_in_data[i])) {
		    c = toupper(mailbox_in_data[i]);
		    mailbox_in_data[i] = c;
		}
	    }
	} 
	/*
	else {
	    display_status 
	        ( "Mailbox is connected. I received data:\n"       		
		"%s", mailbox_in_data);
	}
	*/

/*
 * write received mailbox_in_data to port-buffer:
 */	

	sprintf(buf_hf2box + buf_hf2box_avail, "%s", mailbox_in_data);	
	in_oldavail = buf_hf2box_avail; 
	buf_hf2box_avail+= datalen;
/*
	display_status 
	    ( "New mailbox_in_data now in mailbox input buffer:\n"       		
	    "%s", buf_hf2box + in_oldavail);
	display_status (   
	     "All data in mailbox input buffer: %s ", buf_hf2box);       		
*/

/*
 * special case: first round of new pactor connect:
 */	
	if (lastrxmsg == HFAPP_MSG_STATE_PACTOR_ARQ_SLAVECONNECT &&
	    mailbox_connected == 0
	    && port == 6300 ) {
/*
	    display_status ( 
	        "mailbox_input: Pactor ARQ slave connect! "
		"Will remove rubbish and the first char of the user's msg "
		"(which should be the pactor level) "
		"and then check mailbox.");       		
*/
/*
 * the following cleans remote user's call from rubbish 
 * before and after it, 
 * and removes also the first char of the user's first 
 * message at pactor connect, which is the pactor level (1)
 */
	    sprintf(call, "%s", buf_hf2box + in_oldavail + 1);
	    mailbox_clear_inbuf();	    
	    sprintf(buf_hf2box, "%s\n", call); 	 //	"%s", call);
	    buf_hf2box_avail = strlen(call) + 1; //	;
	    display_status (   
		 "mailbox_input: pactor remote call: " 
		 "%s", buf_hf2box);      		
	    in_oldavail = 0;
	    goto invoke_mailbox_check_io;	
	}

/*
 * special case: new amtor connect:
 */	
	else if (lastrxmsg == HFAPP_MSG_STATE_AMTOR_ARQ_SLAVECONNECT &&
	    mailbox_connected == 0) {
	    display_status ( 
	        "mailbox_input: Amtor ARQ slave connect! "
		"Will check mailbox.");       		
	    goto invoke_mailbox_check_io;	
	}

/*
 * case of FEC-signal, last sign is cr or nl:
 */	
	if (buf_hf2box[buf_hf2box_avail -1 ] == '\r' ) {	
/*
	    display_status ( 
	        "Last received char is carriage return -> "
		"will check mailbox.");       		
*/
	    goto invoke_mailbox_check_io;
	} 
	else if (buf_hf2box[buf_hf2box_avail -1 ] == '\n' ) {	
	    // this happens usually 
/*
	    display_status ( 
	        "Last received char is newline -> "
		"will check mailbox.");  
*/
	    goto invoke_mailbox_check_io;		     		
	} 
	else 
	    return;
	
invoke_mailbox_check_io:
/*
 *  now we block ourselves:
 */	
	mailbox_input_waiting = 1;
	mailbox_check_io();
	return;
}
	    
/* --------------------------------------------------------------------- */

void mailbox_check_io() {
/*
 * Central mailbox control.
 * This function is invoked by 
 * the function mailbox_input in this file, 
 * if a carriage-return or a newline is last char of 
 * the input buffer buf_hf2box.
 *
 * It is also invoked by an amtor slave connect (see msg.c)
 * because there might be no newline in the input at beginning.
 */
	int i = 0;
	
	if (mailbox_on == 0) {
	    display_status 
	        ("mailbox_check_io: you did not activate the mailbox.");	
	    mailbox_clear_allbufs();
	    return;
	}
	if (mailbox_connected) {
	// the 'usual case' is:
	// mailbox was already connected before, so read -> write
	//    display_status 
	//        ("mailbox_check_io: Will send input buf to mailbox...");	
	    if ( mailbox_test_port() ) {
		mailbox_inbuf_2_port();
	//    display_status 
	//        ("mailbox_check_io: Waiting for answer from mailbox...");	
		mailbox_port_2_outbuf();
		return;
	    } else {
	    display_status 
	        ("mailbox_check_io: port to mailbox is closed.");	
	    return;
	    }
	}

	// now, if on but not connected, see if someone calls the mailbox.
	// depends on mode....
	switch (lastrxmsg) {
	    case HFAPP_MSG_STATE_PACTOR_ARQ_SLAVECONNECT:
		display_status 
		    ("mailbox_check_io: we have been connected via Pactor Slave"); 
		if (port == 6300) {
		    // very special case: Pactor 1 connect and
		    // port == 6300 which means F6FBB.
		    // Very special treatment.		
		    display_status 
			("by: %s", buf_hf2box); // + buf_hf2box_written); 
		    display_status 
			("I will try to open the mailbox....");
		    if ( (i = mailbox_open_port()) < 0) {
			display_status 
		    	    ("mailbox_check_io: mailbox could not be opened.\n"
		    	    "Please see if xfbbd is started and\n"
		    	    "port and ipadress options are given correctly.");
			    mailbox_on = 0;		
			    mailbox_clear_allbufs();
			    display_status 
				("I switched the mailbox mode off for now.");
		        return;
		    }		    
		    mailbox_loggedin();
		    mailbox_clear_outbuf();
		    mailbox_port_2_outbuf();
//			special case for f6fbb & pactor 1: discard the answer....
		    mailbox_clear_outbuf();
//			display_status (  
//			"mailbox_check_io: "
//			"buf_hf2box: %s", buf_hf2box);       
			display_status 
		    ("mailbox_check_io: special case: pactor login, F6FBB.\n"
		    "We got the remote user's call already.\n"
		    "-> we discard F6FBB's greeting and call question.\n");
		// user's call should stand in rx buffer now!!
		    mailbox_inbuf_2_port(); 
		    mailbox_port_2_outbuf();
    		// now the box got user's call and asks for password 
		    return;
		} else {
		// other case: port is not 6300, we do not use F6FBB.
		    goto arq_connect;
		    break;
		}
		break;

	    case HFAPP_MSG_STATE_GTOR_ARQ_SLAVECONNECT:
		display_status 
		    ("mailbox_check_io: we have been connected via Gtor ARQ"); 
		    goto arq_connect;
		    break;
		    
	    case HFAPP_MSG_STATE_AMTOR_ARQ_SLAVECONNECT:
		display_status 
		    ("mailbox_check_io: we have been connected via Amtor ARQ"); 
		    goto arq_connect;
		    break;

	    
	    case HFAPP_MSG_STATE_GTOR_ARQ_DISCONNECT:
	    case HFAPP_MSG_STATE_PACTOR_ARQ_DISCONNECT:
	    case HFAPP_MSG_STATE_AMTOR_FEC_DISCONNECT:
		display_status 
		    ("mailbox_check_io: disconnect by remote station"); 
		mailbox_loggedout();
		return;
	    
	    case HFAPP_MSG_STATE_RTTY_RX:
		display_status 
		    ("mailbox_check_io: QRZ in RTTY ?"); 
		goto fec_connect;
		break;	

	    case HFAPP_MSG_START_PACTOR_STANDBY: 
		display_status 
		    ("mailbox_check_io: QRZ in Pactor FEC?"); 
		goto fec_connect;
		break;
	
	    case HFAPP_MSG_START_STANDBY: 
		display_status 
		    ("mailbox_check_io: QRZ in Pactor/Amtor/Gtor FEC ?"); 
		goto fec_connect;
		break;
	
	    case HFAPP_MSG_STATE_PACTOR_FEC_CONNECT:
		display_status 
		    ("mailbox_check_io: QRZ in Pactor-FEC ?"); 
		goto fec_connect;
		break;
	
	    case HFAPP_MSG_STATE_AMTOR_FEC_CONNECT:
	    case HFAPP_MSG_STATE_AMTOR_SELFEC_RX:
	    case HFAPP_MSG_STATE_AMTOR_COLFEC_RX:
		display_status 
		    ("mailbox_check_io: QRZ in Amtor-FEC ?"); 
		goto fec_connect;
		break;	

	    case HFAPP_MSG_STATE_MT63_RX:
		display_status 
		    ("mailbox_check_io: QRZ in MT63 ?"); 
		goto fec_connect;
		break;	

	    default:
		display_status("mailbox_check_io: QRZ ? "); 
		goto fec_connect;
	    break;
	}

arq_connect:
	display_status 
	    ("ARQ connect -> I will try to open the mailbox....");
	if ( (i = mailbox_open_port()) < 0) {
	/*
	    display_status 
	        ("mailbox_check_io: mailbox could not be opened.\n"
	        "See if port and ipadress are given correctly.\n"
		"Test what is waiting on the other side of the port,\n"
		"e.g. by `telnet <host> <port> ` ... !");
	*/
	    mailbox_on = 0;		
	    mailbox_clear_allbufs();
	
	    display_status 
		("I switch the mailbox mode off for now.");
	    mailbox_clear_allbufs();
		return;
	}		    
	mailbox_loggedin();
	mailbox_clear_outbuf();
	mailbox_port_2_outbuf();
	return;
	
fec_connect:
	/*look into the input buffer, check someone calling our call?*/
	if (strstr(buf_hf2box, params.brag.call)) {
	    display_status 
		("mailbox_check_io: Was my call. Try to open port...");
	    if ( (i = mailbox_open_port()) < 0) {
		display_status 
		("... could not open port to mailbox.\n"
	        "See if port and ipadress are given correctly.\n"
		"Test what program is waiting at the port,\n"
		"e.g. by `telnet <host> <port> ` ... !");
		mailbox_clear_allbufs();
		return;
	    }		    
	    mailbox_loggedin();
	    display_status 
		("mailbox_check_io: New connect.....");
	    mailbox_clear_allbufs();
	    // first listen, like every ham should do... 
	    mailbox_port_2_outbuf();
	    return;
	}
	else {
	    display_status 
	        ("mailbox_check_io: This was not my call.");
	    mailbox_clear_allbufs();
	    return;
	}
}

/* --------------------------------------------------------------------- */

void mailbox_loggedin() {
	mailbox_connected = 1;
	beacon_suspend();
//	display_status 
//	    ("Beacon is suspended for time if mailbox connect");
}

/* --------------------------------------------------------------------- */

void mailbox_clear_allbufs() {
	mailbox_clear_inbuf();
	mailbox_clear_outbuf();
//	display_status ( "cleared mailbox input & output buffers.");
	return;
}
	    
/* --------------------------------------------------------------------- */

void mailbox_clear_inbuf() {
	buf_hf2box_written = buf_hf2box_avail = in_oldavail = 0;
	memset (buf_hf2box, 0, BUF_SIZE);
	mailbox_input_waiting = 0;
//	display_status ( "cleared mailbox input buffer.");
	return;
}
	    
/* --------------------------------------------------------------------- */

void mailbox_clear_outbuf() {
	buf_box2hf_written = buf_box2hf_avail = out_oldavail = 0;
	memset (buf_box2hf, 0, BUF_SIZE);
	txwaitbytes = 0;
	mailbox_output_waiting = 0;
//	display_status ( "cleared mailbox output buffer.");
	return;
}
	    
/* --------------------------------------------------------------------- */

void mailbox_output(){
/*
 * this function will be called from process_msg in msg.c
 * after the next "keepalive" message
 * so that there is some delay
 * so that the remote user has time to switch from tx to rx 
 * and can get the message
 *
 * in MT63 there is delay anyway from rempote's tx to our rx
 * and: the keepalive msg does not come, 
 * as there is rubbish received all the time
 * so i call this function directly for MT63
 *
 */
 	GtkText *txt;
	int l = 0, w = 0;
 	char v;

	// setting flag to automatic rx after tx-ing mailbox output:
        txt = GTK_TEXT(gtk_object_get_data(GTK_OBJECT(wmain), 
	    "textedit"));
//	display_status (   "transmitting mailbox output: %s",
//	    buf_box2hf + buf_box2hf_written);       		
	// moving forward "written" mark in buffer 
	w = min(txwaitbytes, BUF_SIZE - buf_box2hf_written);
	autorx_on = 1; 
	autotx();
	edit_newline();
//	gtk_text_insert(txt, NULL, NULL, NULL, 
//	    buf_box2hf + buf_box2hf_written, w);
	for(l = 0; l < w; l++) {
	    v = buf_box2hf[buf_box2hf_written + l];
	    if (v == 10) edit_newline(); // cr instead of lf
	    else edit_addchar(v);
	}	
	edit_newline();
	buf_box2hf_written += w;
	txwaitbytes -= w;
//      /* check if write data has caught read data  */
	if (buf_box2hf_written == buf_box2hf_avail) {
//    	    mailbox_clear_outbuf();
    	    mailbox_clear_allbufs();	    
	} else {
	    // error without change of procedure
	    // just start again for this version...
	    display_status (
		"mailbox_output:\n "
		"I remove overflow of %d bytes of tx",
	         buf_box2hf_avail - buf_box2hf_written);
	}
	mailbox_clear_allbufs();	    
//	    see if other side (f6fbb program) closed port
	mailbox_test_port();
	return;
}

/* --------------------------------------------------------------------- */

int listen_socket (int listen_port) {
    struct sockaddr_in a;
    int s;
    int yes;
    if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
               display_status ( 
	       "listen_port: error: socket at port %d", listen_port);
               return -1;
     }
    yes = 1;
    if (setsockopt
               (s, SOL_SOCKET, SO_REUSEADDR,
                (char *) &yes, sizeof (yes)) < 0) {
               display_status ( 
	       "listen_port: error: setsockopt at port %d", listen_port);
               close (s);
               return -1;
    }
    memset (&a, 0, sizeof (a));
    a.sin_port = htons (listen_port);
    a.sin_family = AF_INET;
    if (bind
               (s, (struct sockaddr *) &a, sizeof (a)) < 0) {
               display_status (  
	       "listen_port: error: bind at port %d", listen_port);
               close (s);
               return -1;
    }
    display_status ( "accepting connections on port %d",
                   (int) listen_port);
    listen (s, 10);
    return s;
}

/* --------------------------------------------------------------------- */

int connect_socket (int connect_port,
                                  char *address) {
           struct sockaddr_in a;
           int s;
           if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
               display_status ( "error connect_socket: socket");
               close (s);
               return -1;
           }

           memset (&a, 0, sizeof (a));
           a.sin_port = htons (connect_port);
           a.sin_family = AF_INET;

           if (!inet_aton
               (address,
                (struct in_addr *) &a.sin_addr.s_addr)) {
               display_status ( "error connect_socket: bad IP address format");
               close (s);
               return -1;
           }

           if (connect
               (s, (struct sockaddr *) &a,
                sizeof (a)) < 0) {
               display_status ( 
	       "error connect_socket: connect()");
               shutdown (s, SHUT_RDWR);
               close (s);
               return -1;
           }
           return s;
       }

/* --------------------------------------------------------------------- */

int mailbox_open_port() {

	if (mailbox_fd > 0) {
//	    display_status (  
//		"mailbox_open: port connecting to mailbox already open.");
	    return(0);
	}

	ipaddress = params.mailbox.host;
	if (ipaddress == NULL) {
	    display_status (  
		"mailbox_open: You did not specify option '-h' \n"
		"(IP address of mailbox computer) for hfterm.\n"
		"Will use '127.0.0.1' (localhost) as default.");
	    ipaddress = "127.0.0.1";
	}

	port = params.mailbox.port;
	if (port == 0) {
	    display_status (  
		"mailbox_open: You did not specify option '-p'\n"
		"(port at which the mailbox can be connected),\n"
		"Will use 6300, which is default for F6FBB.");
	    port = 6300;
	}

	mailbox_fd = connect_socket (port, ipaddress);
	if (mailbox_fd < 0) {
	    display_status 
		( "mailbox_open: Error on opening port %s:%d,\n"
		"mailbox could not be opened.\n"
		"Please see if xfbbd is started and\n"
		"port and ipadress options are given correctly.",	
		ipaddress, port);
	    mailbox_on = 0;		
	    mailbox_clear_allbufs();
	    display_status 
		( "I cleared buffers and switched mailbox mode off.");
	    return(-1);
	} else {
	    display_status 
		( "mailbox_open: ready port %s:%d", 
		ipaddress, port);	
	    return(0);
	}
}

/* --------------------------------------------------------------------- */

int mailbox_test_port () {
// test if still something could be written to mailbox connected via telnet
// but without writing
// (to test if other side has not closed in between)
	int n = 0, r = 0;
       	fd_set rd, wr, er; 
	char c;

	if (mailbox_fd < 0) {
	    display_status (  
		"mailbox_test_port: Sorry, port %s:%d was not opened by hfterm.", 
		ipaddress, port);
	    return 0;
	}

//	if (mailbox_fd > 0) {
//	    display_status (  
//		"mailbox_test_port: Will see if box is really running at "
//		"ip-address %s, port %d...", ipaddress, port);
//	}	

	signal (SIGPIPE, SIG_IGN);
	
	FD_ZERO (&rd);
	FD_ZERO (&wr);
	FD_ZERO (&er);
	FD_SET (mailbox_fd, &rd);
	FD_SET (mailbox_fd, &wr);
	FD_SET (mailbox_fd, &er);
	
	n = max (n, mailbox_fd);

	r = select(n + 1, &rd, &wr, &er, 0); // &tm	
	if (r == -1 && errno == EINTR) {
	    display_status ("mailbox_test_port: error: select(...) == -1 && errno == EINTR");
	    errprintf(SEV_WARNING, "mailbox_test_port: error: select(...) == -1 && errno == EINTR");
	    mailbox_loggedout();
	    return 0;
	}
	if (r < 0) {
	    display_status ("mailbox_test_port: error: select(...) < 0");
	    errprintf(SEV_WARNING, "mailbox_test_port: error: select(...) < 0");
   	    mailbox_loggedout();
    	    return 0;
	}
	if (FD_ISSET (mailbox_fd, &er)) {
	    errno = 0;
	    r = recv(mailbox_fd, &c, 1, MSG_OOB);
	    if ( r < 1 ) {
//		display_status ("mailbox_test_port: "
//		"error in receiving exception message: %c", c );       
		mailbox_loggedout();
		return 0;
	    }
//	    display_status ("mailbox_test_port: "
//	    "box sends exception message: %c", c );       
	}		    
	if (FD_ISSET (mailbox_fd, &rd)) {
	    /*
	    display_status ("mailbox_test_port: port ready for read.\n"
	    "where there should be nothing to read. Bug of F6FBB, I think.\n"
	    "So I quit the connection now!");       
	    */
	    /*  a   f o r   n o t   t o    c o n f u s e   u s e r : */
	    display_status ("mailbox has closed port.");       
	    /*
	    BUG OF F&FBB: no clean disconnect of the port !!!
	    this result of select appears at the end of f6fbb connect only, 
	    after the " 73 <name>,".
	    If i then call here "mailbox_port_2_outbuf();"
	    that function says "nothing to read", so an endless loop occurs,
	    and the pactor connect stays forever.
	    So i tried just simply: 
	    */
	// test
	    r = recv(mailbox_fd, &c, 1, MSG_OOB);
	    if ( r < 1 ) {
//		display_status ("mailbox_test_port: "
//		"error in receiving exception message: %c", c );       
		//mailbox_loggedout();
		//return 0;
	    }
//	    display_status ("mailbox_test_port: "
//	    "box sends exception message: %c", c );       
	
	    mailbox_loggedout();
	    return 0;
	    /*
	    and THAT WORKS WITH F6FBB, now auto-qrt when box quits.
	    Was lot of work for me !!!! Günther 
	    */
	}		    
    	if (FD_ISSET (mailbox_fd, &wr)) {
	    //display_status ("mailbox_test_port: port ready to write to box.");       
	    return 1;
	}	
     	display_status ( "mailbox_test_port: unknown error.");       
            mailbox_loggedout();
	    return 0;
         
}

/* --------------------------------------------------------------------- */

void mailbox_inbuf_2_port () {
// write received data from the buffer hf2box to f6fbb via telnet
	static int n = 0;
	int w = 0, r = 0;
       	fd_set wr; 
	struct timeval tm;

	if (mailbox_fd < 0) {
	    display_status (  
		"mailbox_inbuf_2_port: Sorry, not reached port %s:%d.", 
		ipaddress, port);
	    return;
	}

//	if (mailbox_fd > 0) {
//	    display_status (  
//		"mailbox_inbuf_2_port: box running at "
//		"ip-address %s, port %d", ipaddress, port);
//	}	

	signal (SIGPIPE, SIG_IGN);
	// select block
	{
	    FD_ZERO (&wr);
	    FD_SET (mailbox_fd, &wr);
	    n = max (n, mailbox_fd);
            if (mailbox_fd > 0 && buf_hf2box_avail - buf_hf2box_written > 0) {
//	       display_status 
//	           ("mailbox_inbuf_2_port: port to mailbox ready for write");
               FD_SET (mailbox_fd, &wr);
               n = max (n, mailbox_fd);
    	    }
    	    tm.tv_sec = 0;
	    tm.tv_usec = 300000; //100000;
	    r = select(n + 1, NULL, &wr, NULL, &tm);
	 
	    if (r == -1 && errno == EINTR) {
		display_status ( "mailbox_inbuf_2_port: error: select(...) == -1 && errno == EINTR");
		errprintf(SEV_WARNING, "mailbox_inbuf_2_port: error: select(...) == -1 && errno == EINTR");
		mailbox_loggedout();
		return;
	    }
	    if (r < 0) {
		display_status ( "mailbox_inbuf_2_port: error: select(...) < 0");
		errprintf(SEV_WARNING, "mailbox_inbuf_2_port: error: select(...) < 0");
   		mailbox_loggedout();
    		return;
	    }
// WRITE FROM BUFFER TO BOX
	    if (FD_ISSET (mailbox_fd, &wr)) {
//		display_status (  "mailbox_inbuf_2_port: try write to mailbox");       
//		display_status (  
//			"mailbox_inbuf_2_port: before write: "
//			"buf_hf2box: %s", buf_hf2box);       
		w = write (mailbox_fd, buf_hf2box + buf_hf2box_written,
		    buf_hf2box_avail - buf_hf2box_written);
        	if (w == 0) {
     		    display_status (  
			"mailbox_inbuf_2_port: nothing could be written to mailbox");       
		    return;
    		} else if (w < 0) {
     		    display_status (  
			"mailbox_inbuf_2_port: error while writing to mailbox."
			"I will close it.");       
            	    mailbox_loggedout();
		    return;
        	} else {
//		    display_status (  
//			"mailbox_inbuf_2_port: %d bytes written to mailbox: %s", 
//			w, buf_hf2box + buf_hf2box_written);       	    
//		    display_status (  
//			"mailbox_inbuf_2_port: total buf_hf2box: %s", 
//			buf_hf2box);       	    
            	    buf_hf2box_written += w;
        	}
	    }
//        /* check if write data has caught read data  */
     	    if (buf_hf2box_written == buf_hf2box_avail) {
		mailbox_clear_inbuf();
	    } else {
	    // error without change of procedure
	    // just start again for this version...
	    	display_status 
		    ("mailbox_inbuf_2_port: I remove overflow of %d bytes of rx",
		    buf_hf2box_avail - buf_hf2box_written);
		    mailbox_clear_inbuf();
	    }
	}
	return;
}

/* --------------------------------------------------------------------- */

void mailbox_port_2_outbuf () {
/*
 * get f6fbb's answer via telnet and write it to my buffer box2hf
 */ 
	static int n = 0;
	int  r = 0, newtxbytes = 0;
       	fd_set rd; 
	struct timeval tm;

	if (mailbox_fd < 0) {
	    display_status (  
            	"mailbox_port_2_outbuf: Sorry, port %s:%d unreachable.", 
		ipaddress, port);
	    mailbox_loggedout();
	    return;
	}
//	if (mailbox_fd > 0) {
//	    display_status (  
//		"mailbox_port_2_outbuf: box running at "
//		"ip-address %s, port %d", ipaddress, port);
//	}	
	millisleep (350); //70
//	display_status("slept 350 ms to give F6FBB time to think");
	signal (SIGPIPE, SIG_IGN);
	FD_ZERO (&rd);
	FD_SET (mailbox_fd, &rd);
	n = max (n, mailbox_fd);

	if (mailbox_fd > 0 && buf_box2hf_avail < BUF_SIZE) {
//     	display_status (  "mailbox_port_2_outbuf: port from mailbox ready");
    	    FD_SET (mailbox_fd, &rd);
	    n = max (n, mailbox_fd);
	}
    	tm.tv_sec = 0;
	tm.tv_usec = 300000; // 150000; //100000;
	r = select(n + 1, &rd, NULL, NULL, &tm);
//	display_status ( "mailbox_port_2_outbuf: start of select() wait");
	if (r == -1 && errno == EINTR) {
	    display_status 
		( "mailbox_port_2_outbuf: error: select(...) == -1 && errno == EINTR");
		errprintf(SEV_WARNING, "mailbox_port_2_outbuf: error: select(...) == -1 && errno == EINTR");
		mailbox_loggedout();
		return;
	}
	if (r < 0) {
	    display_status ( "mailbox_port_2_outbuf: error: select(...) < 0");
		errprintf(SEV_WARNING, "mailbox_port_2_outbuf: error: select(...) < 0");
   		mailbox_loggedout();
    		return;
	}
// READ FROM BOX & WRITE INTO BUFFER
	if (FD_ISSET (mailbox_fd, &rd)) {
//	        display_status (  
//		     "mailbox_port_2_outbuf: try to get & transmit mailbox output");       
//		out_oldavail = buf_box2hf_avail;
	    newtxbytes = read (mailbox_fd, buf_box2hf + buf_box2hf_avail, 
		BUF_SIZE - buf_box2hf_avail);
	    if (newtxbytes == 0) {
		display_status 
		    ( "mailbox_port_2_outbuf: nothing to read from port.");       
		mailbox_input_waiting = 0;
		return;
    	    } else if (newtxbytes < 0) {
		display_status 
	    	    ("mailbox_port_2_outbuf: error while reading from mailbox.");
		mailbox_loggedout();
	        return;
	    } else { // read is positive 
    		/*
		display_status 
		    ("mailbox_port_2_outbuf: read from mailbox %d new chars", 
		    newtxbytes);       
		display_status 
		    ("mailbox_port_2_outbuf: these new data are waiting for tx: %s", 
		    buf_box2hf + buf_box2hf_avail);       		
		display_status    
		    ("mailbox_port_2_outbuf: all data now waiting for tx: %s", 
		    buf_box2hf);       		
		*/

// 			first 2 bytes are rubbish but only at greeting
//			but seems not to be transmitted 
//			memset (buf_box2hf + buf_box2hf_avail, 0, 2);
//			display_status    
//			    ("mailbox_port_2_outbuf: all data, cleared, now waiting for tx: %s", 
//			    buf_box2hf);       		
// 			moving forward "available" mark in buffer 
		txwaitbytes += newtxbytes;
		buf_box2hf_avail += newtxbytes;

		// switch from input block to output-waiting-flag 
		// to make debug easier
		mailbox_output_waiting = 1;
		mailbox_input_waiting = 0;

		// from msg.c after some time ( = after a keepalive-msg)
		// mailbox_output() will be called 
		// if mailbox_output_waiting = 1
		
		/*
		if (lastrxmsg == HFAPP_MSG_STATE_MT63_RX) {
		    display_status("mailbox_output() called directly for mt63");
		    mailbox_output();
		}
		*/
		// mailbox_output() will be called 
		// directly for mt63
	    } 
	} /* end of  "if (FD_ISSET (mailbox_fd, &rd))" block  */ 
}

/* --------------------------------------------------------------------- */

void mailbox_loggedout() {
	mailbox_clear_allbufs();
	SHUT_mailbox_fd 
	display_status ( "Mailbox reset & standby ....");
	mailbox_connected = 0;
	beacon_restore();
	mailbox_on = 1;
//	mailbox_on stays 1, means still standby of automatic mailbox!!!
//	can be set to 0 only by menu via function on_mailbox_activate 
//	in callbacks.c.
//	transmit("Mailbox closed -> disconnect! 73 de hfterm!\n");
	qrt(); // automatic qrt, this function is in rxtx.c
	return;
}

/* --------------------------------------------------------------------- */
