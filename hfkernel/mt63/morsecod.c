/*
 *    morsecod.c  --  Morse table generator, from a human readable form
 *		      into a computer readable form.
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

#define TableSize 128
long CodeTable[TableSize];

#define MaxLineLen 128
char LineBuff[MaxLineLen];

FILE *InpFile;
FILE *OutFile;

int main(int argc, char *argv[])
{ int ch,c,len,Line,p; long code;

if(argc!=3)
{ printf("Usage: morsecod <alphabet> <data file>\n"); return 1; }

for(ch=0; ch<TableSize; ch++) CodeTable[ch]=0x00000004L;

InpFile=fopen(argv[1],"rt");
if(InpFile==NULL)
{ printf("Can't open %s to read the alphabet\n",argv[1]); return 1; }

for(Line=0; !feof(InpFile); )
{ if(fgets(LineBuff,MaxLineLen,InpFile)==NULL) break;
  Line+=1;
  len=strlen(LineBuff);
  if(LineBuff[len-1]!='\n')      // line too long
  { printf("Line #%d too long in file %s\n",Line,argv[1]); break; }
  if(len<4) { printf("Line #%d assumed empty\n",Line); continue; }           // empty line
  if(LineBuff[1]!=' ') { printf("Line #%d assumed a comment\n",Line); continue; } // not a single character plus a space
  ch=LineBuff[0]; if(ch>=TableSize) continue; // code too high
  for(code=0x4L,c=3,p=len-2; p>1; p--)
  { c+=1;
    switch(LineBuff[p])
    { case '-':
      case '_': code<<=4; code|=0xEL; break;
      case '.': code<<=2; code|=0x2L; break;
      default: printf("Bad code specified at line #%d\n",Line);
    }
    if(c>31) printf("Code at line #%d has too many dots/dashes\n",Line);
    CodeTable[ch]=code;
  }
}
fclose(InpFile);

OutFile=fopen(argv[2],"wt");
if(OutFile==NULL)
{ printf("Can't open %s to write the code table\n",argv[2]); return 1; }

fprintf(OutFile,"\
// computer readable Morse code table\n\
//\n\
// For a given character you should pick up a 31-bit code from the table.\n\
// Bits should be taken starting from the LSB.\n\
// Bit equal 1 means carrier ON, bit 0 means carrier off\n\
// Each code includes one quiet dot at the start and two at the end.\n\
// The code should be read until the last '1',\n\
// but this last '1' must not be transmitted\n\
//\n\
");

fprintf(OutFile,"const int MorseTableSize=%d;\n",TableSize);
fprintf(OutFile,"long MorseTable[MorseTableSize] = {\n");

for(ch=0; ch<' '; ch++)
  fprintf(OutFile," 0x%08lXL , // 0x%02X\n",CodeTable[ch], ch);
for(    ; ch<TableSize-1; ch++)
  fprintf(OutFile," 0x%08lXL , // 0x%02X = '%c'\n",CodeTable[ch], ch, ch);
fprintf(OutFile,  " 0x%08lXL   // 0x%02X = '%c'\n} ;\n",CodeTable[ch], ch, ch);

fclose(OutFile);

return 0;
}
