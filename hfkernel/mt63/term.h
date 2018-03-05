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

#ifdef __MSDOS__
#include <conio.h>
#else
#include <curses.h>
#endif

class SplitTerm
{ public:
   SplitTerm(); ~SplitTerm();
   int Preset(int TxLines=0);
   void Close(void);
   int UserInp(int &key);
   void RxOut(char ch);
   void RxStr(char *str);
   void TxOut(char ch);
   void Status(int Stat, char *Str);
   inline void RxStatUpp(char *Str) { Status(0,Str); }
   inline void RxStatLow(char *Str) { Status(1,Str); }
   inline void TxStatUpp(char *Str) { Status(2,Str); }
   inline void TxStatLow(char *Str) { Status(3,Str); }

  private:
   int Init;
   int Width,Height;

   int RxPos, RxLen;
   int RxCurX, RxCurY, RxAct;

   int TxPos, TxLen;
   int TxCurX, TxCurY, TxAct;

   int StatPos[4];

   void ChOut(char ch);
} ;
