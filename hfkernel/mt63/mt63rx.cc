/*
 *    mt63rx.cc  --  The MT63ASC receiver in C++ for LINUX,
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

// Date: 11-NOV-1999

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

#include "term.h"
#include "stdinr.h"

// ============================================================================

char DevName[32]=""; 	// the name of the sound device
char UseDevice=0;

int ReqRate=8000;	// we request 8000 Hz sampling
float DevRate=0;	// the device gives us a possibly different rate
float UserRate=0.0;	// user provided rate (overrides the DevRate)

int ReadFromFile=0;	// read real audio or playback from a file
char *PlaybackFile=NULL; // file to read audio from in playback mode

int SaveToFile=0;	// save audio to a file
char *SaveFile=NULL;	// file to save audio

/*
int SendTextFile=0;	// send text from a file
char *TextFile=NULL;
*/

int Bandwidth=1000;	// 500, 1000 or 2000 Hz bandwidth
int Interleave=0;	// short/long interleave
int IntegLen=32;        // integration period for sync./data tracking

int UseTerm=1;		// use terminal or output to STDOUT

// ============================================================================

SoundDevice RxDev;	// sound device to read audio

RateConvBL RateConv;	// rate converter to adjust the sampling rate
int UseRateConv=1;

#define BuffLen 512
s16 InpBuff[BuffLen];	// Signed 16-bit audio input buffer
float_buff InpFloat;

LevelMonitor InpLevel;

MT63rx Rx;

SplitTerm RxTerm;

// ============================================================================

/* this code is to debug the spectra waterfall support
void SpectraDisplay(float *SpectraPower, int Len)
{ int i;
  printf("[SpectraDisplay] Len=%d\n",Len);
  for(i=0; i<Len; i++) printf("%3d: %8.5f\n",i,SpectraPower[i]);
  printf("\n");
}
*/

char StatusLine[80];

void RxMode(void)
{ sprintf(StatusLine,"MT63 Rx: %s, %d Hz, %s Intlve, Sync. over %d symb.",
          ReadFromFile ? PlaybackFile:DevName,
          Bandwidth, Interleave ? " LONG":"SHORT", IntegLen);
  RxTerm.RxStatUpp(StatusLine); }

void RxStatus(void)
{ int Conf; float SNR;
  Conf=(int)floor(Rx.SYNC_Confidence()*100.0); if(Conf>99) Conf=99;
  SNR=Rx.FEC_SNR(); if(SNR>99.9) SNR=99.9;
  sprintf(StatusLine,"Inp:%5.3f/%04.1f%% %s:%02d%%/%+05.2f/%4.2f/%4.2f FEC:%4.1f/%+2d %+6.1fHz ",
    InpLevel.RMS, 100.0*InpLevel.OutOfRange,
    Rx.SYNC_LockStatus() ? "Lock" : "Seek",Conf,
    Rx.SYNC_FreqOffset(),Rx.SYNC_FreqDevRMS(),Rx.SYNC_TimeOffset(),
    SNR,Rx.FEC_CarrOffset(), Rx.TotalFreqOffset());
  RxTerm.RxStatLow(StatusLine); }

int main(int argc, char *argv[])
{ int err,arg,InpLen,i; char code;
  float ValueF; int ValueI;
  int stop,key;
  int OutFileNo=0; int wr;

printf("\n\
Multitone MT63 modem for Linux - receiver, (c) 1999 Pawel Jalocha\n\
Made to be compatible with the MT63ASC for the EVM56K/DSPCARD4\n\
\n");

if(argc<=1)
{ printf("\
Usage: mt63rx <options>\n\
 the options:\n\
	 -d<device> the output dsp device number or name [default = none]\n\
		    -d => /dev/dsp, -d1 => /dev/dsp1, etc.\n\
		    or: -d/dev/dsp, -d/dev/dsp1, etc.\n\
	 -s<file>   save audio to a file\n\
	 -p<file>   playback audio from a file\n\
	 -r<rate>   request given sampling rate from device [8000 Hz]\n\
	 -R<rate>   the true sampling rate (if you know it already)\n\
	 -B<band>   select bandwidth: 500, 1000 or 2000 Hz [1000Hz]\n\
	 -i         use the double interleave [default = single]\n\
	 -I<period> set the sync. integration period [32 symbols]\n\
Examples:\n\
mt63rx -d0    => read audio from /dev/dsp0, decode with single interleave\n\
mt63rx -d0 -i -I32 => decode with double interleave and longer integration\n\
mt63rx -pmsg.raw => decode the audio file in the raw signed 16-bit format\n\
mt63rx -pmsg.raw -R8010 => decode from msg.raw, the sampling rate was 8010 Hz\n\
mt63rx -d0 -slog.raw -i => decode audio from /dev/dsp0 with double interleave\n\
			   but save it to log.raw for later playback\n\
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
  case 'I':
    if(sscanf(argv[arg]+2,"%d",&ValueI)!=1)
      { printf("Invalid decimal number in %s\n",argv[arg]); err+=1; }
    else IntegLen=ValueI;
    break;
  case 'R':
    if(sscanf(argv[arg]+2,"%f",&ValueF)!=1)
      { printf("Invalid floating point number in %s\n",argv[arg]); err+=1; }
    else UserRate=ValueF;
    break;
  case 's':
    SaveToFile=1; SaveFile=argv[arg]+2; break;
  case 'p':
    ReadFromFile=1; PlaybackFile=argv[arg]+2; break;
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

if((UseDevice==0)&&(ReadFromFile==0))
{ printf("No audio device selected nor a file to read audio\n"); goto Error; }

if(UseDevice) printf("Audio device for input is %s\n",DevName);
else printf("No audio device selected\n");

if(ReadFromFile)
{ if(UseDevice)
  { err=RxDev.OpenFileForRead(PlaybackFile,ReqRate,DevName);
    if(err<0)
    { printf("Can't open %s or %s to read audio from\n",PlaybackFile,DevName); goto Error; }
    printf("We read the audio from %s and play it on %s\n",
	    PlaybackFile,DevName);
  } else
  { err=RxDev.OpenFileForRead(PlaybackFile,ReqRate);
    if(err<0)
    { printf("Can't open %s to read audio from\n",PlaybackFile); goto Error; }
    printf("We read the audio from %s\n",PlaybackFile);
  }
} else
{ err=RxDev.OpenForRead(DevName,ReqRate);
  if(err<0) { printf("Can't open %s to read audio from: errno => %s\n",
		   DevName,strerror(errno)); goto Error; }
}

if(SaveToFile)
{
#ifdef __MSDOS__
  OutFileNo=open(SaveFile,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,S_IREAD|S_IWRITE);
#else
  OutFileNo=open(SaveFile,O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
#endif
  if(OutFileNo<0)
  { printf("Can't open file %s to save audio, errno => %s\n",
	   SaveFile,strerror(errno)); goto Error; }
  printf("We will save the audio to %s for later playback\n",SaveFile);
} else OutFileNo=0;

DevRate=RxDev.Rate;
printf("Requested rate = %d Hz, device reported rate = %1.0f Hz\n",
       ReqRate,DevRate);
if(UserRate>0.0)
{ printf("You say the true device rate is %4.2f Hz.\n",UserRate);
  DevRate=UserRate; }
UseRateConv=(DevRate!=8000.0);

InpLevel.Preset(10.0,0.75);

if(UseRateConv)
{ err=RateConv.Preset(16,(float**)NULL,64);
  if(err)
  { printf("Can't preset the rate converter - not enough RAM ?!\n"); goto Error; }
  err=RateConv.ComputeShape(0.0,0.75*M_PI,WindowBlackman2);
  if(err)
  { printf("Can't compute the shape for the rate converter - not enough RAM ?!\n"); goto Error; }
  RateConv.SetOutVsInp(8000.0/DevRate);
}

printf("Modem bandwidth is %d Hz with %s interleave\n",
	Bandwidth,Interleave ? "DOUBLE (64)" : "SINGLE (32)");
printf("The time/frequency synchronizer integrates over %d symbols\n",IntegLen);

// err=Rx.Preset(Bandwidth,Interleave,IntegLen,SpectraDisplay);
err=Rx.Preset(Bandwidth,Interleave,IntegLen);
if(err) { printf("Can't preset the MT63 receiver - not enough RAM ?\n");
	  goto Error; }

/*
InpFloat.EnsureSpace(BuffLen);
for(i=0; i<BuffLen; i++) InpFloat.Data[i]=0.0;
InpFloat.Len=200;		// timing shift for debug
RateConv.ProcessLinI(&InpFloat);
Rx.Process(&RateConv.Output);
*/
if(UseTerm)
{ printf("\nPress RETURN to continue\n\n"); getchar();
  RxTerm.Preset();
  RxMode(); RxStatus(); }
else printf("\n");

for(stop=0; !stop; )
{ if(UseTerm)
  { err=RxTerm.UserInp(key);
    if(err) stop=1; // { if((key=='q')||(key=='Q')) stop=1; }
  } else
  { if(StdinReady()) stop=1; }

  InpLen=RxDev.Read(InpBuff,BuffLen);
  if(InpLen<0)
  { printf("Error while reading audio from %s, errno => %s\n",
	   DevName,strerror(errno)); break; }
// the following applies when we read audio in non-blocking mode
#ifdef __linux__
  if(InpLen==0) { usleep(50000); continue; }
#endif
#ifdef __MSDOS__
  if(InpLen==0) continue;
#endif
  if(OutFileNo>0)
  { wr=write(OutFileNo,InpBuff,2*InpLen);
    if(wr!=(2*InpLen))
    { printf("Error while writing data to %s\n",SaveFile); break; }
  }

  ConvS16toFloat(InpBuff,&InpFloat,InpLen);
  InpLevel.Process(&InpFloat);
  if(UseRateConv)
  { RateConv.ProcessLinI(&InpFloat);
    Rx.Process(&RateConv.Output);
  } else Rx.Process(&InpFloat);
  if(UseTerm)
  { for(i=0; i<Rx.Output.Len; i++)
    { code=Rx.Output.Data[i]; RxTerm.RxOut(code); }
    RxStatus();
  } else
  { for(i=0; i<Rx.Output.Len; i++)
    { code=Rx.Output.Data[i];
      if((code>=' ')||(code==0x08)||(code==0x09)) printf("%c",code);
      else if(code=='\r') printf("\n");
      else if(code!='\0') printf("<%02X>",code);
      fflush(stdout);
    }
  }
}

Stop:
 if(UseTerm) RxTerm.Close();
 printf("\nClosing audio device ...\n");
 RxDev.Close();
 if(OutFileNo>0) close(OutFileNo);
 printf("Stopped OK.\n");
 printf("%ld samples read = %3.1f sec\n",
     RxDev.TotalRead,RxDev.TotalRead/DevRate);
 return 0;

Error:
 RxDev.Close();
 if(OutFileNo>0) close(OutFileNo);
 return 1;

}
