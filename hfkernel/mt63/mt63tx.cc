/*
 *    mt63tx.cc  --  The MT63ASC transmitter in C++ for LINUX,
 *                   written to be compatible with the
 *                   MT63ASC.ASM modem for the EVM56K/DSPCARD4.
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

// Date: 08-NOV-1999

#include <stdio.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#ifdef __FreeBSD__ 
#include <unistd.h>
#endif

#include "dsp.h"
#include "mt63.h"
#include "sound.h"

#include "stdinr.h"

// ============================================================================

char DevName[32]=""; 	// the name of the sound device
char UseDevice=0;

int ReqRate=8000;	// we request 8000 Hz sampling
float DevRate=0;	// the device gives us a possibly different rate
float UserRate=0.0;	// user provided rate (overrides the DevRate)

int SaveToFile=0;	// save audio to a file
char *SaveFile=NULL;	// file to save audio

int SendTextFile=0;	// send text from a file
char *TextFile=NULL;

int Bandwidth=1000;	// 500, 1000 or 2000 Hz bandwidth
int Interleave=0;	// short/long interleave

char *CW_ID=NULL;	// CW identification to be transmitted along the digital signal

// ============================================================================

MT63tx Tx;		// MT63 encoder

RateConvBL RateConv;	// rate converter to adjust the sampling rate
int UseRateConv=1;

s16_buff OutBuff;	// S16 buffer

SoundDevice TxDev;	// sound device to output audio

// ============================================================================

int main(int argc, char *argv[])
{ int len,err,i,arg; char ch;
  float ValueF; int ValueI;
  int End; int InpFile;

printf("\n\
Multitone MT63 modem for Linux - transmitter, (c) 1999 Pawel Jalocha, SP9VRC\n\
Made to be compatible with the MT63ASC for the EVM56K/DSPCARD4\n\
\n");

if(argc<=1)
{ printf("\
Usage: mt63tx <options>\n\
 the options:\n\
	 -d<device> the output dsp device number or name [default = none]\n\
		    -d => /dev/dsp, -d1 => /dev/dsp1, etc.\n\
		    or: -d/dev/dsp, -d/dev/dsp1, etc.\n\
	 -s<file>   save audio to a file\n\
	 -r<rate>   request given sampling rate from device [8000 Hz]\n\
	 -R<rate>   the true sampling rate (if you know it already)\n\
	 -t<file>   send text from <file> [default = send from keyboard]\n\
	 -B<band>   select bandwidth: 500, 1000 or 2000 Hz [1000Hz]\n\
	 -i         use the long interleave [default = short]\n\
	 -C<CW_ID>  transmit the <CW_ID> identifier along the signal\n\
                    put underscores into the message instead of spaces\n\
Examples:\n\
  mt63tx -d0      => default settings, send audio to /dev/dsp0\n\
  mt63tx -d0 -i   => use the double interleave (64 symbols)\n\
  mt63tx -tmsg.txt -smsg.raw => transmit message from msg.txt but save audio\n\
				to a file in a raw signed 16-bit format\n\
");
goto Error; }

for(err=0,arg=1; arg<argc; arg++)
{ if(argv[arg][0]!='-')
  { printf("Unknown option %s\n",argv[arg]); err+=1; continue; }
  switch(argv[arg][1])
  {
  case 'd':
    if(argv[arg][2]=='\0')
    { strcpy(DevName,"/dev/dsp"); UseDevice=1; }
    else if(isdigit(argv[arg][2]))
    { strcpy(DevName,"/dev/dsp"); strcat(DevName,argv[arg]+2); UseDevice=1; }
    else if(argv[arg][2]=='/')
    { strcpy(DevName,argv[arg]+2); UseDevice=1; }
    else
    { printf("Bad device number/name in %s\n",argv[arg]); err+=1; }
    break;
  case 'r':
    if(sscanf(argv[arg]+2,"%d",&ValueI)!=1)
      { printf("Invalid decimal number in %s\n",argv[arg]); err+=1; }
    else ReqRate=ValueI;
    break;
  case 'R':
    if(sscanf(argv[arg]+2,"%f",&ValueF)!=1)
      { printf("Invalid floating point number in %s\n",argv[arg]); err+=1; }
    else UserRate=ValueF;
    break;
  case 's':
    SaveToFile=1; SaveFile=argv[arg]+2; break;
  case 't':
    SendTextFile=1; TextFile=argv[arg]+2; break;
  case 'C':
    CW_ID=argv[arg]+2; break;
  case 'i':
    Interleave=1;
    break;
  case 'B':
    if(sscanf(argv[arg]+2,"%d",&ValueI)!=1)
      { printf("Invalid decimal number in %s\n",argv[arg]); err+=1; }
    else
    { switch(ValueI)
      { case 500:
	case 1000:
	case 2000:
	  Bandwidth=ValueI; break;
	default:
	  { printf("The modem bandwidth can only be 500, 1000 or 2000 Hz\n"); err+=1; }
      }
    } break;
  default:
    printf("Unknown option %s\n",argv[arg]); err+=1;
  }
}

if(err) goto Error;

if((UseDevice==0)&&(SaveToFile==0))
{ printf("No audio device selected nor a file to save audio\n"); goto Error; }

if(UseDevice) printf("Audio device for output is %s\n",DevName);
else printf("No audio device selected\n");

if(SaveToFile)
{ if(UseDevice)
  { err=TxDev.OpenFileForWrite(SaveFile,ReqRate,DevName);
    if(err<0)
    { printf("Can't open %s or %s to write audio to, errno => %s\n",
	     SaveFile,DevName,strerror(errno)); goto Error; }
    printf("We save the audio to %s and play it via %s\n",
	    SaveFile,DevName);
  } else
  { err=TxDev.OpenFileForWrite(SaveFile,ReqRate);
    if(err<0)
    { printf("Can't open %s to write audio to, errno => %s\n",
	     SaveFile,strerror(errno)); goto Error; }
    printf("We write the audio to %s\n",SaveFile);
  }
} else
{ err=TxDev.OpenForWrite(DevName,ReqRate);
  if(err<0) { printf("Can't open %s to write audio to: errno => %s\n",
		   DevName,strerror(errno)); goto Error; }
}
DevRate=TxDev.Rate;
printf("Requested rate = %d Hz, device reported rate = %1.0f Hz\n",
       ReqRate,DevRate);
if(UserRate>0.0)
{ printf("You say the true device rate is %4.2f Hz.\n",UserRate);
  DevRate=UserRate; }
UseRateConv=(DevRate!=8000.0);

if(UseRateConv)
{ err=RateConv.Preset(16,(float**)NULL,64);
  if(err)
  { printf("Can't preset the rate converter - not enough RAM ?!\n"); goto Error; }
  err=RateConv.ComputeShape(0.0,0.75*M_PI,WindowBlackman2);
  if(err)
  { printf("Can't compute the shape for the rate converter - not enough RAM ?!\n"); goto Error; }
  RateConv.SetOutVsInp(DevRate/8000.0);
}
printf("Modem bandwidth is %d Hz with %s interleave\n",
	Bandwidth,Interleave ? "DOUBLE (64)" : "SINGLE (32)");

if(CW_ID!=NULL)
{ for(i=0; CW_ID[i]; i++) if(CW_ID[i]=='_') CW_ID[i]=' ';
  printf("CW message \"%s\" is transmitted along the digital signal\n",CW_ID); }

err=Tx.Preset(Bandwidth,Interleave,CW_ID);
if(err)
{ printf("Can't preset the MT63tx module - not enough RAM ?!\n"); goto Error; }

if(SendTextFile)
{
  InpFile=open(TextFile,O_RDONLY,0);
  if(InpFile<0) { printf("Can't open %s to read text from\n",TextFile); goto Stop; }
  printf("MT63 transmitting text from %s ...\n",TextFile);
// the following code is to debug the Tx.SendTune()
/*  for(i=0; i<20; i++) // send tuning, just to test the SendTune()
  { Tx.SendTune();
    if(UseRateConv)
    { RateConv.ProcessLinI(&Tx.Comb.Output);
      ConvFloatToS16(&RateConv.Output,&OutBuff);
    } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
    len=TxDev.Write(OutBuff.Data,OutBuff.Len);
    printf("tuning...");
    if(len!=OutBuff.Len)
    { printf("Error while sending out audio, errno => %s\n",strerror(errno));
      goto Stop; }
  }
*/
  for(End=0; !End; )
  { err=read(InpFile,&ch,1); if(err!=1) { close(InpFile); break; }
    printf("%c",ch); fflush(stdout);
    if(ch=='\n') ch='\r'; Tx.SendChar(ch);
    if(UseRateConv)
    { RateConv.ProcessLinI(&Tx.Comb.Output);
      ConvFloatToS16(&RateConv.Output,&OutBuff);
    } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);

    len=TxDev.Write(OutBuff.Data,OutBuff.Len);
    if(len!=OutBuff.Len)
    { printf("Error while sending out audio, errno => %s\n",strerror(errno));
      close(InpFile); goto Stop; }
  } close(InpFile);
}
else
{ printf("MT63 transmiting: type your text, press Ctrl-D to stop\n");

  err=StdinSetRaw();
  if(err) { printf("Can't set STDIN into the raw mode\n"); goto Error; }
  for(End=0; !End; )
  { ch=StdinRead(); Tx.SendChar(ch); if(ch==0x04) End=1;
    if(ch) { printf("%c",ch);
	   if(ch=='\r') printf("\n"); fflush(stdout); }

    if(UseRateConv)
    { RateConv.ProcessLinI(&Tx.Comb.Output);
      ConvFloatToS16(&RateConv.Output,&OutBuff);
    } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);

    len=TxDev.Write(OutBuff.Data,OutBuff.Len);
    if(len!=OutBuff.Len)
    { printf("Error while sending out audio, errno => %s\n",strerror(errno));
      StdinRestore(); goto Stop; }
  } StdinRestore(); 
}

printf("\nFlushing data interleaver ...\n");
for(i=0; i<Tx.DataInterleave; i++)
{ Tx.SendChar('\0');
  if(UseRateConv)
  { RateConv.ProcessLinI(&Tx.Comb.Output);
    ConvFloatToS16(&RateConv.Output,&OutBuff);
  } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
  len=TxDev.Write(OutBuff.Data,OutBuff.Len);
  if(len!=OutBuff.Len)
  { printf("Error while sending out audio, errno => %s\n",strerror(errno));
    goto Stop; }
}

printf("Sending jamming waveform ...\n");
for(i=0; i<20; i++)
{ Tx.SendJam();
  if(UseRateConv)
  { RateConv.ProcessLinI(&Tx.Comb.Output);
    ConvFloatToS16(&RateConv.Output,&OutBuff);
  } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
  len=TxDev.Write(OutBuff.Data,OutBuff.Len);
  if(len!=OutBuff.Len)
  { printf("Error while sending out audio, errno => %s\n",strerror(errno));
    goto Stop; }
}

Stop:
 printf("\nClosing audio device ...\n");
 TxDev.Close();
 printf("Stopped OK.\n");
 printf("%ld samples sent out = %3.1f sec\n",
     TxDev.TotalWritten,TxDev.TotalWritten/DevRate);
 return 0;

Error:
 TxDev.Close();
 return 1;

}
