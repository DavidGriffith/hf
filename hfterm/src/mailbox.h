/*****************************************************************************/
/*
 *      mailbox.h  --  mailbox-via-tcp-port feature for hfterm.
 *
 * 	2004 by Günther Montag DL4MGE
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
      
#ifndef _MAILBOX_H
#define _MAILBOX_H

/* --------------------------------------------------------------------- */

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

/* --------------------------------------------------------------------- */

#define BUF_SIZE 8192

/* --------------------------------------------------------------------- */

extern int mailbox_on, mailbox_connected, mailbox_output_waiting, mailbox_fd;
extern int port;
extern char buf_hf2box[BUF_SIZE], buf_box2hf[BUF_SIZE];
extern int buf_box2hf_avail, buf_box2hf_written;
extern int buf_hf2box_avail, buf_hf2box_written;

/* functions in mailbox.c ----------------------------------------------- */

int listen_socket (int listen_port);
int connect_socket (int connect_port, char *address);
void millisleep(int milliseconds);
void mailbox_input(unsigned char* data, int datalen);
void mailbox_output();
void mailbox_check_io(); 
int mailbox_open_port();
int mailbox_test_port();
void mailbox_loggedin();
void mailbox_inbuf_2_port();
void mailbox_port_2_outbuf();
void mailbox_clear_allbufs();
void mailbox_clear_inbuf();
void mailbox_clear_outbuf();
void mailbox_loggedout();


#endif /* _MAILBOX_H */
