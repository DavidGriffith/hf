/*
 *    peakrms.c  --  simple tools to print Peak/RMS/other characteristics
 *                   of an audio file made primarily to check files made
 *                   by the MT63 modem.
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

// (c) Pawel Jalocha, SP9VRC

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

FILE *InpFile=NULL;

#define BuffSize 8192
short InpBuff[BuffSize];

int main(int argc, char *argv[])
{ int len,i; long Total;
  float InpScale,Sig,Max,Min,Peak;
  double SX,SXX; double Aver,RMS;
  float T1,T2,T3; long C1,C2,C3; double P,P1,P2,P3;

  InpScale=1.0/32768.0;

  if(argc<2)
  { printf("Usage: peakrms <input.raw>\n");
    return 1; }
  InpFile=fopen(argv[1],"rb");
  if(InpFile==NULL)
  { printf("Can't open %s for input\n",argv[1]); return 1; }

  SX=0.0; SXX=0.0; Min=1.0; Max=(-1.0);
  for(Total=0; ; )
  { len=fread(InpBuff,2,BuffSize,InpFile);
    if(len<=0) break;
    for(i=0; i<len; i++)
    { Sig=InpBuff[i]*InpScale; SX+=Sig; SXX+=Sig*Sig;
      if(Sig>Max) Max=Sig; else if(Sig<Min) Min=Sig; }
    Total+=len;
  }
  if(Total==0) goto Stop;
  printf("%s: 1st pass, %ld samples\n",argv[1],Total);
  Aver=SX/Total; RMS=sqrt(SXX/Total-Aver*Aver);
  Peak=Max-Aver; if((Aver-Min)>Peak) Peak=Min-Aver;
  printf("DC = %+6.3f, RMS = %5.3f, Peak = %+6.3f, Peak/RMS = %4.1fdB\n",
	 Aver,RMS,Peak,20*log(fabs(Peak)/RMS)/log(10.0));

  fseek(InpFile,0L,SEEK_SET);

  T1=RMS; T2=2*RMS; T3=3*RMS;
  C1=0;   C2=0;     C3=0;
  P1=0.0; P2=0.0;   P3=0;

  for(Total=0; ; )
  { len=fread(InpBuff,2,BuffSize,InpFile);
    if(len<=0) break;
    for(i=0; i<len; i++)
    { Sig=InpBuff[i]*InpScale; Sig=fabs(Sig-Aver);
	   if(Sig>T3) C3++;
      else if(Sig>T2) C2++;
      else if(Sig>T1) C1++;
      if(Sig>T1) P1+=Sig*Sig-T1*T1;
      if(Sig>T2) P2+=Sig*Sig-T2*T2;
      if(Sig>T3) P3+=Sig*Sig-T3*T3;
    }
    Total+=len;
  }
  printf("%s: 2nd pass, %ld samples\n",argv[1],Total);

  C2+=C3; C1+=C2;

  printf("Samples above RMS: %5.2f%%, above 2*RMS: %4.2f%%, above 3*RMS: %4.2f%%\n",
	 (100.0*C1)/Total,(100.0*C2)/Total,(100.0*C3)/Total);

  P=RMS*RMS*Total;
  printf("Power above RMS: %5.2f%%, above 2*RMS: %4.2f%%, above 3*RMS: %4.2f%%\n",
	 100.0*P1/P,100.0*P2/P,100.0*P3/P);

Stop:
  fclose(InpFile);

  return 0;
}
