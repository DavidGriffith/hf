/*
 *    mt63trx.cc  --  The MT63ASC transceiver in C++ for LINUX,
 *                    written to be compatible with the
 *                    MT63ASC.ASM modem for the EVM56K/DSPCARD4.
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

#include "dsp.h"
#include "mt63.h"
#include "sound.h"

#include "term.h"

// ============================================================================

char DevName[32]=""; 	// the name of the sound device
char UseDevice=0;

int ReqRate=8000;	// we request 8000 Hz sampling
float DevRate=0;	// the device gives us a possibly different rate
float UserRate=0.0;	// user provided rate (overrides the DevRate)

int Bandwidth=1000;	// 500, 1000 or 2000 Hz bandwidth
int Interleave=0;	// short/long interleave
int IntegLen=32;        // integration period for sync./data tracking

char *CW_ID=NULL;	// CW identification to be transmitted along the digital signal

// ============================================================================

SoundDevice Dev;	// sound device to read/write audio

RateConvBL RateConv;	// rate converter to adjust the sampling rate
int UseRateConv=1;

#define BuffLen 256
s16 InpBuff[BuffLen];	// Signed 16-bit audio input buffer
float_buff InpFloat;

LevelMonitor InpLevel;	// input audio monitor

MT63rx Rx;		// MT63 decoder

SplitTerm Term;		// Rx/Tx split terminal

MT63tx Tx;		// MT63 encoder

s16_buff OutBuff;

char_fifo KeyBuff;

// ============================================================================

int TxON=0;

char StatusLine[80];

int TxSwitch='T'-'@';	// Ctrl-T to turn on the transmitter
int RxSwitch='R'-'@';	// Ctrl-R to turn off the transmitter
int DropSwitch='D'-'@'; // Ctrl-D to drop the receiver lock
int AbortSwitch='A'-'@'; // Ctrl-A to abort the transmitter immediately
int StopSwitch='^'-'@'; // Ctrl-^ to stop the program
int ExitSwitch='X'-'@'; // alternate (pico like) exit

int TxReq=0;
int RxReq=0;
int DropReq=0;
int AbortReq=0;
int StopReq=0;

void ReadKeyboard(void)
{ int key, err;
  while(Term.UserInp(key))
  { if(key==TxSwitch) { TxReq=1; AbortReq=0; continue; }
    // if(key==RxSwitch) { RxReq=1; continue; }
    if(key==DropSwitch) { DropReq=1; continue; }
    if(key==AbortSwitch) { AbortReq=1; continue; }
    if((key==StopSwitch)||(key==ExitSwitch)) { StopReq=1; continue; }
    err=KeyBuff.Inp((char)key);
    if(!err) Term.TxOut(key);
  }
}

void ModeStatus(void)
{ sprintf(StatusLine,"MT63: %s, %d Hz, %s Intlve, Sync. over %d symb.",
	  DevName, Bandwidth, Interleave ? "LONG (64)":"SHORT (32)", IntegLen);
  Term.RxStatUpp(StatusLine); }

// --------------------------------------------------------------------------

int RxStart(void)
{ int err;

  err=Dev.OpenForRead(DevName,ReqRate);
  if(err<0) return -1;
  DevRate=Dev.Rate; if(UserRate>0.0) DevRate=UserRate;
  UseRateConv=(DevRate!=8000.0);

  InpLevel.Preset(64.0,0.75);

  if(UseRateConv)
  { err=RateConv.Preset(32,(float**)NULL,16);
    if(err) return -1;
    err=RateConv.ComputeShape(0.0,M_PI,WindowHamming);
    if(err) return -1;
    RateConv.SetOutVsInp(8000.0/DevRate);
  }
  err=Rx.Preset(Bandwidth,Interleave,IntegLen);
  if(err) return -1;

  return 0;
}

int RxProcess(void)
{ int i,InpLen; char code;
  InpLen=Dev.Read(InpBuff,BuffLen);
  if(InpLen<0) return -1;

  ConvS16toFloat(InpBuff,&InpFloat,InpLen);
  InpLevel.Process(&InpFloat);
  if(UseRateConv)
  { RateConv.ProcessLinI(&InpFloat);
    Rx.Process(&RateConv.Output);
  } else Rx.Process(&InpFloat);

  for(i=0; i<Rx.Output.Len; i++)
  { code=Rx.Output.Data[i]; Term.RxOut(code); }
  return 0;
}

void RxStatus(void)
{ int Conf; float SNR;
  Conf=(int)floor(Rx.SYNC_Confidence()*100.0); if(Conf>99) Conf=99;
  SNR=Rx.FEC_SNR(); if(SNR>99.9) SNR=99.9;
  sprintf(StatusLine,"Inp:%5.3f/%04.1f%% %s:%02d%%/%+05.2f/%4.2f/%4.2f FEC:%4.1f/%+2d %+6.1fHz ",
    InpLevel.RMS, 100.0*InpLevel.OutOfRange,
    Rx.SYNC_LockStatus() ? "Lock" : "Seek",Conf,
    Rx.SYNC_FreqOffset(),Rx.SYNC_FreqDevRMS(),Rx.SYNC_TimeOffset(),
    SNR,Rx.FEC_CarrOffset(), Rx.TotalFreqOffset());
  Term.RxStatLow(StatusLine); }

int RxResync(void) { return Rx.Preset(Bandwidth,Interleave,IntegLen); }

void RxStop(void) { Dev.Close(); Rx.Free(); }

// --------------------------------------------------------------------------

int TxStart(void)
{ int err;

  err=Dev.OpenForWrite(DevName,ReqRate);
  if(err<0) return -1;
  DevRate=Dev.Rate; if(UserRate>0.0) DevRate=UserRate;
  UseRateConv=(DevRate!=8000.0);

  if(UseRateConv)
  { err=RateConv.Preset(32,(float**)NULL,16);
    if(err) return -1;
    err=RateConv.ComputeShape(0.0,M_PI,WindowHamming);
    if(err) return -1;
    RateConv.SetOutVsInp(DevRate/8000.0);
  }
  err=Tx.Preset(Bandwidth,Interleave,CW_ID);
  if(err) return -1;

  return 0;
}

int TxProcess(void)
{ char ch; int err,len;
  if(RxReq)
  { if(RxReq==1) Term.RxStr("\nFlushing Tx ...");
    Tx.SendChar('\0');
    RxReq+=1; if(RxReq>Tx.DataInterleave) { RxReq=0; AbortReq=1; }
  } else
  { err=KeyBuff.Out(ch);
    if(err) ch='\0';
    else { Term.RxOut(ch); if(ch==(char)RxSwitch) RxReq=1; }
    Tx.SendChar(ch); }
  if(UseRateConv)
  { RateConv.Process(&Tx.Comb.Output);
    ConvFloatToS16(&RateConv.Output,&OutBuff);
  } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
  len=Dev.Write(OutBuff.Data,OutBuff.Len);
  if(len!=OutBuff.Len) return -1;
  return 0;
}

void TxStatus(void)
{ Term.RxStatLow("Transmitting ..."); }

void TxStop(void) { Dev.Close(); Tx.Free(); }

// ============================================================================

int main(int argc, char *argv[])
{ int err,arg,InpLen,i; char code;
  float ValueF; int ValueI;
  int stop,key;

printf("\n\
Multitone MT63 modem for Linux, (c) 1999 Pawel Jalocha\n\
Made to be compatible with the MT63ASC for the EVM56K/DSPCARD4\n\
\n");

if(argc<=1)
{ printf("\
Usage: mt63trx <options>\n\
 the options:\n\
	 -d<device> the output dsp device number or name [default = none]\n\
		    -d => /dev/dsp, -d1 => /dev/dsp1, etc.\n\
		    or: -d/dev/dsp, -d/dev/dsp1, etc.\n\
	 -r<rate>   request given sampling rate from device [8000 Hz]\n\
	 -R<rate>   the true sampling rate (if you know it already)\n\
	 -B<band>   select bandwidth: 500, 1000 or 2000 Hz [1000Hz]\n\
	 -i         use the double interleave [default = single]\n\
	 -I<period> set the sync. integration period [32 symbols]\n\
	 -C<CW_ID>  transmit the <CW_ID> identifier along the signal\n\
                    put underscores into the message instead of spaces\n\
\n\
Terminal operation:\n\
 Ctrl-T switches to transmit\n\
 Ctrl-R switches to receive (but waits will all typed text is sent off)\n\
 Ctrl-A aborts the transmition immediately (still needs to flush the soundcard)\n\
 Ctrl-D drops the receiver lock and forces signal reacquisition\n\
 Ctrl-^ or Ctrl-X quits the program (needs some time to flush the soundcard)\n\
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
  case 'C':
    CW_ID=argv[arg]+2; break;
  case 'R':
    if(sscanf(argv[arg]+2,"%f",&ValueF)!=1)
      { printf("Invalid floating point number in %s\n",argv[arg]); err+=1; }
    else UserRate=ValueF;
    break;
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

if(UseDevice==0)
{ printf("No audio device selected !\n"); goto Error; }

if(CW_ID!=NULL)
{ for(i=0; CW_ID[i]; i++) if(CW_ID[i]=='_') CW_ID[i]=' '; }

err=KeyBuff.Preset(1000);
if(err) { printf("Can't preset the keyboard buffer\n"); goto Error; }

err=RxStart();
if(err) { printf("Can't start the receiver on %s, errno => %s\n",
                  DevName,strerror(errno));
          goto Error; }
TxON=0;

Term.Preset(8);

ModeStatus();
Term.RxStatLow("");

if(CW_ID==NULL) Term.TxStatUpp("");
else
{ sprintf(StatusLine,"CW ID: %s",CW_ID);
  Term.TxStatUpp(StatusLine); }

Term.TxStatLow("MT63 TRX for LINUX, (c) Pawel Jalocha, SP9VRC");

while(!StopReq)
{ ReadKeyboard();
  if(TxON)
  { err=TxProcess(); if(err) break;
    if(AbortReq)
    { TxStop(); err=RxStart(); if(err) break;
      Term.RxStr("\nReceiving:\n");
      RxStatus(); AbortReq=0; TxON=0; }
  } else
  { err=RxProcess(); if(err) break;
    RxStatus();
    if(DropReq)
    { Term.RxStr("\nDropping Rx lock, resync. ...\n");
      err=RxResync(); DropReq=0; if(err) break; }
    if(TxReq)
    { RxStop(); err=TxStart(); if(err) break;
      Term.RxStr("\nTransmitting:\n");
      TxStatus(); TxReq=0; TxON=1; }
  }
}

Stop:
 TxStop(); RxStop(); Term.Close();
 return 0;

Error:
 TxStop(); RxStop(); Term.Close();
 return 1;
}
