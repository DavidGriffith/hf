
/*
 *    addnoise.c  --  add noise to a sound file (signed 16-bit format).
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
#include <stdlib.h>
#include <math.h>

static double UniformNoise(void)
{ return ((double)rand()+1.0)/((double)RAND_MAX+1.0); }

void WhiteNoise(double *I, double *Q)
{ double Power,Phase;
  Power=sqrt(-2*log(UniformNoise()));
  Phase=2*M_PI*UniformNoise();
  (*I)=Power*cos(Phase);
  (*Q)=Power*sin(Phase);
}

FILE *InpFile=NULL;
FILE *OutFile=NULL;

#define BuffSize 8192
short InpBuff[BuffSize];

int main(int argc, char *argv[])
{ int len,i; long Total; float RMS;
  double I,Q;

  if(argc<4)
  { printf("Usage: addnoise <input.sw> <output.sw> <noise RMS>\n");
    return 1; }
  InpFile=fopen(argv[1],"rb");
  if(InpFile==NULL)
  { printf("Can't open %s for input\n",argv[1]); return 1; }
  OutFile=fopen(argv[2],"wb");
  if(OutFile==NULL)
  { printf("Can't open %s for input\n",argv[2]); return 1; }
  len=sscanf(argv[3],"%f",&RMS);
  if(len!=1)
  { printf("Invalid floating point number: %s\n",argv[3]); return 1; }

  for(Total=0; ; )
  { len=fread(InpBuff,2,BuffSize,InpFile);
    if(len<=0) break;
    for(i=0; i<len; i++)
    { WhiteNoise(&I,&Q);
      InpBuff[i]+=(short)floor(I*RMS*32768.0+0.5); }
    fwrite(InpBuff,2,len,OutFile); Total+=len;
  }
  printf("Done, %ld samples processed\n",Total);
  fclose(OutFile); fclose(InpFile);
  return 0;
}
