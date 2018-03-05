/*     portecho.c
 * 
 *  by Günther Montag dl4mge
 *  test program for telnet port, it uses "toupper" 
 *  and just echoes the input in capitals.
 *  derived from tfwd.c, which is taken from the 
 *  very interesting select_tut man page (see man select_tut).
 *
 *  This is part of hf package, GPL. Have fun!
*/

 /* ----------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

#define SHUT_FD1 {                      \
               if (fd1 >= 0) {                 \
                   shutdown (fd1, SHUT_RDWR);  \
                   close (fd1);                \
                   fd1 = -1;                   \
               }                               \
           }

#define BUF_SIZE 1024

int quit = 0;
char* greeting = "Connected to portecho!\nQuit me by #.\n";
char* byebye   = "Bye ! \n";
  
 /* ----------------------------------------------------------------- */

static int listen_socket (int listen_port) {
    struct sockaddr_in a;
    int s;
    int yes;
    if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
               perror ("socket");
               return -1;
     }
    yes = 1;
    if (setsockopt
               (s, SOL_SOCKET, SO_REUSEADDR,
                (char *) &yes, sizeof (yes)) < 0) {
               perror ("setsockopt");
               close (s);
               return -1;
    }
    memset (&a, 0, sizeof (a));
    a.sin_port = htons (listen_port);
    a.sin_family = AF_INET;
    if (bind
               (s, (struct sockaddr *) &a, sizeof (a)) < 0) {
               perror ("bind");
               close (s);
               return -1;
    }
    printf ("accepting connections on port %d\n",
                   (int) listen_port);
    listen (s, 10);
    return s;
}

 /* ----------------------------------------------------------------- */

int main (int argc, char **argv) {
   int h, i, fd1 = -1;
   char buf1[BUF_SIZE], buf2[BUF_SIZE];
   // buf1 what port reads from extern prog = tx
   // buf2 what port sends to extern prog   = rx 
   int buf1_avail = 0, buf1_written = 0;
   int buf2_avail = 0, buf2_written = 0;

// vars of my test prog
    char zeichen;
    char quitsignal = '#';

   if (argc != 2) {
       fprintf (stderr,
           "\n\t* * *  dl4mge 's PORTECHO. Little TCP test program  * * *\n"
	     "\t* * *  for tests with hf's mailbox function.        * * *\n"
	     "\n"
             "\tUsage: portecho  <port>, e.g. 'portecho 3333'.\n"
	     "\tQuit me with '#'. \n\n");
//		<forward-to-port> <forward-to-ip-address>\n");
       exit (1);
   }
       fprintf (stdout,
           "\n\t* * *  dl4mge 's PORTECHO. Little TCP test program  * * *\n"
	     "\t* * *  for tests with hf's mailbox function.        * * *\n"
	     "\n"
	     "\tTest me on another console by 'telnet localhost <port>'.\n"
	     "\tQuit me with '#'. \n\n");


   signal (SIGPIPE, SIG_IGN);
   h = listen_socket (atoi (argv[1]));
   if (h < 0) {
       fprintf (stderr, "error on opening port\n");
       exit (1);
    }
    for (;;) {
      for (;;) {
       int r, n = 0;
       fd_set rd, wr, er;
       FD_ZERO (&rd);
       FD_ZERO (&wr);
       FD_ZERO (&er);
       FD_SET (h, &rd);
       n = max (n, h);
       if (fd1 > 0 && buf1_avail < BUF_SIZE) {
       	   fprintf (stderr, "testport ready for read\n");
           FD_SET (fd1, &rd);
           n = max (n, fd1);
       }
       if (fd1 > 0
           && buf2_avail - buf2_written > 0) {
	   fprintf (stderr, "testport ready for write\n");
           FD_SET (fd1, &wr);
           n = max (n, fd1);
       }
       if (fd1 > 0) {
	   fprintf (stderr, "testport ready for err\n");
           FD_SET (fd1, &er);
           n = max (n, fd1);
       }
        r = select (n + 1, &rd, &wr, &er, NULL);
	if (r == -1 && errno == EINTR)
           continue;
	if (r < 0) {
           perror ("select()");
           exit (1);
	}
	if (FD_ISSET (h, &rd)) {
	    unsigned int l;
    	    struct sockaddr_in client_address;
    	    fprintf (stderr, "something tries to access port.\n");  
	    memset (&client_address, 0, l =sizeof (client_address));
    	    r = accept (h, (struct sockaddr *) &client_address, &l);
    	    fprintf (stderr, "I will try to accept peer port\n");  
    	    if (r < 0) {
    		perror ("accept() peer port");
    	    } else { 
    		fprintf (stderr, "I accepted peer port\n");  
        	fprintf (stderr, "r = %d\n",r);  	       
        	SHUT_FD1;
                buf1_avail = buf1_written = 0;
	        buf2_avail = buf2_written = 0;
    	        fd1 = r;
		sprintf(buf2, "%s", greeting);
		buf2_avail = strlen(greeting);
	    }
	}
//       /* NB: read oob data before normal reads 
        if (fd1 > 0)
    	    if (FD_ISSET (fd1, &er)) {
               char c;
               errno = 0;
               r = recv (fd1, &c, 1, MSG_OOB);
               if (r < 1) {
                   SHUT_FD1;
		} else
            	    fprintf (stderr, "error: %s, errno %d\n", &c, errno);  
		}
// READ
	if (fd1 > 0) {
           if (FD_ISSET (fd1, &rd)) {
	   	fprintf (stderr, "try read\n");       
		r =  read (fd1, buf1 + buf1_avail, BUF_SIZE - buf1_avail);
		if (r < 1) {
                   SHUT_FD1;
                } else {
		    buf1_avail += r;
            	    fprintf (stderr, "read %d chars\n", r);       
		    fprintf (stderr, "buf1: %s\n", buf1);       		
		}
	    
// PROCESS
		for (i = 0; i < buf1_avail && i < (BUF_SIZE - buf2_avail); i++) {
		    zeichen = buf1[buf1_written++];
			if (zeichen != quitsignal) {
			    buf2[buf2_avail++] = toupper(zeichen);			    
			}
			if (zeichen == quitsignal) {
		    	    fprintf (stderr, 
				"read: Signal to quit: %c !\n", zeichen);  
			    quit = 1;
		    	    sprintf(buf2, "%s", byebye);
			    buf2_avail = strlen(byebye);
			    //exit(0);
			}
		    }
		fprintf (stderr, "buf2: %s\n", buf2);       		
	    }
	}
// WRITE
        if (fd1 > 0) {
           if (FD_ISSET (fd1, &wr)) {
		fprintf (stderr, "write\n");       
                r = write (fd1, buf2 + buf2_written,
		  buf2_avail - buf2_written);
                if (r < 1) {
                   SHUT_FD1;
                } else {
                   buf2_written += r;
                }
	    }
	}
//       /* check if write data has caught read data 
        if (buf1_written == buf1_avail) {
    	    buf1_written = buf1_avail = 0;
	    memset(buf1, 0, sizeof(buf1));
	}
    	if (buf2_written == buf2_avail) {
	    buf2_written = buf2_avail = 0;
	    memset(buf2, 0, sizeof(buf2));
	    if (quit) {
		SHUT_FD1
		quit = 0;
		break;
	    }
	}
    
      }	
    }    
  return 0;
}

