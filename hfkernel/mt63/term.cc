/*
 *    term.cc  --  simple split mode terminal
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

#include <stdio.h>
#include <string.h>

#define NODELAY_HACK 1 // nodelay() does not work in ncurses of Linux ?!
                       // we hack it with a routine from stdinr.cc

#ifdef __MSDOS__
#include <conio.h>
#else
#include <curses.h>
#ifdef NODELAY_HACK
#include "stdinr.h"
#endif
#endif

#include "term.h"

#ifdef __MSDOS__

SplitTerm::SplitTerm() { Init=0; }

SplitTerm::~SplitTerm() { }

void SplitTerm::Close(void)
{ textcolor(LIGHTGRAY); textbackground(BLACK);
  _setcursortype(_NORMALCURSOR);
  window(1,1,Width,Height); clrscr(); }

int SplitTerm::Preset(int TxLines)
{
  Width=80; Height=25;
  TxLen=TxLines;
  if(TxLen==0) { RxLen=Height-TxLen-2; }
	  else { RxLen=Height-TxLen-4; }
  RxPos=2;
  TxPos=RxPos+RxLen+2;

  StatPos[0]=1;
  StatPos[1]=RxPos+RxLen;
  StatPos[2]=RxPos+RxLen+1;
  StatPos[3]=TxPos+TxLen;

  window(1,1,Width,Height); clrscr();
  RxCurX=1; RxCurY=1; RxAct=0;
  TxCurX=1; TxCurY=1; TxAct=0;

  return 0;
}

int SplitTerm::UserInp(int &key)
{ if(kbhit()) { key=getch(); return 1; }
  return 0; }

void SplitTerm::RxOut(char ch)
{ if(!RxAct)
  { window(1,RxPos,Width,RxPos+RxLen-1);
    gotoxy(RxCurX,RxCurY); RxAct=1; TxAct=0; _setcursortype(_NORMALCURSOR); }
  ChOut(ch); RxCurX=wherex(); RxCurY=wherey(); }

void SplitTerm::TxOut(char ch)
{ if(!TxAct)
  { window(1,TxPos,Width,TxPos+TxLen-1);
    gotoxy(TxCurX,TxCurY); TxAct=1; RxAct=0; _setcursortype(_NORMALCURSOR); }
  ChOut(ch); TxCurX=wherex(); TxCurY=wherey(); }

void SplitTerm::ChOut(char ch)
{ if(ch=='\0') return;
  textcolor(LIGHTGRAY); textbackground(BLACK);
  if((ch&0x80)||(ch>=' ')||(ch==0x08)||(ch==0x09)) putch(ch);
  else if(ch=='\n') { if(wherey()>1) putch(ch); }
  else if(ch=='\r') { putch(ch); putch('\n'); }
  else { textcolor(BLACK); textbackground(LIGHTGRAY); putch(ch+'@'); }
}

void SplitTerm::Status(int Stat, char *Str)
{ int i,y=StatPos[Stat];
  RxAct=0; TxAct=0; window(1,y,Width,y);
  textcolor(BLACK); textbackground(LIGHTGRAY); _setcursortype(_NOCURSOR);
  cprintf("%s",Str); for(i=strlen(Str); i<Width-1; i++) putch(' '); }

#else // Linux code with ncurses

SplitTerm::SplitTerm()
{ Init=0; }

SplitTerm::~SplitTerm()
{ if(Init) { erase(); refresh(); endwin(); } }

void SplitTerm::Close(void)
{ if(Init) { erase(); refresh(); endwin(); Init=0; } }

int SplitTerm::Preset(int TxLines)
{
  if(Init) endwin();

  initscr(); cbreak(); noecho(); nonl(); // raw();
  // nodelay(stdscr,TRUE); // nodelay() has no effect, what's wrong ?!
  // halfdelay(1); // half-delay seem to work, but it waits for too long...
  intrflush(stdscr,FALSE); keypad(stdscr,TRUE);
  scrollok(stdscr,TRUE);
  erase();
  Init=1;

  Width=COLS; Height=LINES;
  TxLen=TxLines;
  if(TxLen==0) { RxLen=Height-TxLen-2; }
	  else { RxLen=Height-TxLen-4; }
  RxPos=1;
  TxPos=RxPos+RxLen+2;

  StatPos[0]=0;
  StatPos[1]=RxPos+RxLen;
  StatPos[2]=RxPos+RxLen+1;
  StatPos[3]=TxPos+TxLen;

  RxCurX=0; RxCurY=RxPos; RxAct=0;
  TxCurX=0; TxCurY=TxPos; TxAct=0;

  refresh();

  return 0;
}

#ifdef NODELAY_HACK
int SplitTerm::UserInp(int &key)
{ if(StdinReady()) { key=getch(); return 1; }
             else  return 0; }
#else
int SplitTerm::UserInp(int &key)
{ key=getch(); return key==ERR ? 0 : 1; }
#endif

void SplitTerm::ChOut(char ch)
{ int x,y;
  if(ch=='\0') return;
  attrset(A_NORMAL);
  if((ch&0x80)||(ch>=' ')||(ch==0x08)||(ch==0x09)) addch(ch);
  else if(ch=='\n') { getyx(stdscr,y,x); if(x>0) addch(ch); }
  else if(ch=='\r') { addch('\n'); }
  else { attrset(A_REVERSE); addch(ch+'@'); }
}

void SplitTerm::RxOut(char ch)
{ if(!RxAct)
  { setscrreg(RxPos,RxPos+RxLen-1);
    move(RxCurY,RxCurX); RxAct=1; TxAct=0; }
  ChOut(ch); getyx(stdscr,RxCurY,RxCurX); refresh(); }

void SplitTerm::RxStr(char *str)
{ if(!RxAct)
  { setscrreg(RxPos,RxPos+RxLen-1);
    move(RxCurY,RxCurX); RxAct=1; TxAct=0; }
  attrset(A_NORMAL); addstr(str);
  getyx(stdscr,RxCurY,RxCurX); refresh(); }

void SplitTerm::TxOut(char ch)
{ if(!TxAct)
  { setscrreg(TxPos,TxPos+TxLen-1);
    move(TxCurY,TxCurX); TxAct=1; RxAct=0; }
  ChOut(ch); getyx(stdscr,TxCurY,TxCurX); refresh(); }

void SplitTerm::Status(int Stat, char *Str)
{ int i,y=StatPos[Stat];
  RxAct=0; TxAct=0; move(y,0);
  attrset(A_REVERSE);
  for(i=0; Str[i]; i++) addch(Str[i]);
  for(   ; i<Width-1; i++) addch(' ');
  refresh(); }

#endif
