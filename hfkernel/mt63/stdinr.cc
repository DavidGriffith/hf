/*
 *    stdinr.cc  --  raw access to the keyboard
 *
 *    Copyright (C) 1999-2004 Pawel Jalocha, SP9VRC
 *
 *    This file is part of MT63.
 *
 *    MT63 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    MT63 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with MT63; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifdef __MSDOS__

#include <conio.h>

#include "stdinr.h"

int StdinSetRaw(void) { return 0; }

int StdinRead(void) { return kbhit() ? getch() : 0; }

int StdinReady(void) { return kbhit(); }

int StdinRestore(void) { return 0; }

#else

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>

#include "stdinr.h"

struct termios StdinOrigState; /* saved state of standard input */

/* set the standard input in "raw" mode so we can read char by char */
int StdinSetRaw(void)
{ int err; struct termios StdinState;
  err=isatty(STDIN_FILENO);
   if(err==0) { printf("STDIN is not a terminal ?\n"); return -1; }
  err=fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
   if(err) { printf("Can't set the NONBLOCK option on STDIN\n"); return -1; }
  err=tcgetattr(STDIN_FILENO,&StdinOrigState);
   if(err) { printf("tcgetattr() returns %d on STDIN\n",err); return -1; }
  err=tcgetattr(STDIN_FILENO,&StdinState); if(err) return -1;
   if(err) { printf("tcgetattr() returns %d on STDIN\n",err); return -1; }
  StdinState.c_lflag &= ~(ICANON|ECHO);
  StdinState.c_cc[VMIN] = 1;
  StdinState.c_cc[VTIME] = 0;
  /* cfmakeraw(&StdinState); */
  err=tcsetattr(STDIN_FILENO,TCSANOW,&StdinState);
   if(err) { printf("tcsetattr() returns %d on STDIN\n",err); return -1; }
  return 0;
} /* OK => return 0; problems => return -1; */

/* check if STDIN ir ready for read - does NOT need to call StdinSetRaw() ! */
int StdinReady(void)
{ fd_set InpSet;
  struct timeval Timeout;
  FD_ZERO(&InpSet); FD_SET(STDIN_FILENO,&InpSet);
  Timeout.tv_sec=0; Timeout.tv_usec=0;
  if(select(STDIN_FILENO+1,&InpSet,NULL,NULL,&Timeout)<0) return -1;
  return FD_ISSET(STDIN_FILENO,&InpSet);
} /* STDIN has got something => 1, nothing to read from STDIN => 0,
     problems => -1 */

/* read a character from standard input, returns zero if input is empty */
int StdinRead(void)
{ int err; char key;
  err=read(STDIN_FILENO,&key,1);
  if(err<0) { return errno==EAGAIN ? 0 : -1; }
  if(err==0) return 0;
  return (int)key;
} /* empty => return 0; key pressed => return key; problems => return -1; */

/* restore standard input to state as before StdinSetRaw() */
int StdinRestore(void)
{ int err=tcsetattr(STDIN_FILENO,TCSANOW,&StdinOrigState);
  if(err) { printf("tcsetattr() returns %d on STDIN\n",err); return -1; }
  return 0; }

#endif












