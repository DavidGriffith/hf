/*
 *    stdinr.h  --  raw access to the keyboard
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

// set the standard input in "raw" mode so we can read char by char
int StdinSetRaw(void);
// OK => return 0; problems => return -1;

// check if STDIN is ready for read
// StdinReady() works as well without calling StdinSetRaw()
// but then it returns 1 only after the user presses RETURN
int StdinReady(void);
// STDIN has got something => 1, nothing to read from STDIN => 0,
//   problems => -1

// read a character from standard input, returns zero if input is empty
int StdinRead(void);
// empty => return 0; key pressed => return key; problems => return -1;

// restore standard input to state as before StdinSetRaw()
int StdinRestore(void);
// for now it always returns 0













