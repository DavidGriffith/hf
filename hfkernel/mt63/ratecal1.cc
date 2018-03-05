/*
 *    ratecal1.cc  --  Sound card sampling rate calibrator (for LINUX)
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

// file: ratecal1.cc, modified on 29-AUG-1999
// to compile you need as well these files:
// dsp.h, dsp.cc, sound.h, sound.cc, stdinr.h, stdinr.cc
//
// The purpose of this program is to measure the sampling rate
// of a soundcard. I find the sampling rate may be off by
// more than one percent which is not acceptable for
// my DSP experiments.
// To give an example: when I request 8000 Hz from my SoundMan (PAS16)
// it tells me it can do 8007 Hz instead, but in reality it does
// 7910.3 Hz => so a 90 Hz error.
// I think this is a "bug" in the LiNUX PAS16 device driver
// which sets the divisor to N while it should set it to N-1,
// but the corrected driver makes still an error of 0.2%
// so one needs the rate calibration anyway.
//
// The program has been tuned to work with the HF timing standards
// which transmit short pulses each second. I used the signals on
// 4996.0, 9996.0 and 14996.0 kHz. I set the HF receiver in the USB mode
// and 1.0 kHz below the frequency to get pulses at 1000 Hz audio.
// In principle any signal having a periodic envelope
// with a known period can be used, so an AMTOR/SITOR station is OK,
// if you change the reference period to 0.450 sec with -T0.450 option.
// What counts is the envelope of the signal around the frequency
// given by -F and within the bandwidth given by -B.
// The calibrator will try to match the delay which gives the smallest
// differential error. This procedure ussually gives you the rate
// within 0.5 Hz (at 8000 Hz sampling).
//
// For more accuracy multiply the repetition time: for example
// give -T4.0 thus 4 second reference period; a signal with period
// of T is as well periodic over time = N*T where N is an integer.
// If your sampling rate error is large (like with my card)
// tell the program the (about) true rate with the -R option.
// for example for my card I run the more precise test:
//
// ratecal1 -r8000 -R7910 -T4.0 -I40 -B200
//
// The most precise test I have ever done was by typing:
//
// ratecal1 -r8000 -R7910.3 -T300 -I60.0 -B500 -D4
//
// and leaving it for 10 minutes on the 14995.0 USB.
//
// After the calibration is done put down the two values:
// the rate you request (-r) and the true rate you get.
// You may try different requested rates (-r) so you get
// the real rate close to what you like.
// For example with my card, I need to request 8100 Hz to get
// real rate of 8018.2 Hz which is closest to the 8000 Hz
// which I want for my applications.
//
// Timing signals on 4996, 9996 and 14996 kHz are not ideally periodic
// because some pulses are longer, some are doubled, etc.
// This has certainly negative effect on the calibration result.
// The pulses there are not transmitted all the time but according
// to certain schedule thus we can not apply too long integration times too.
// Still rate measurement with an error about 10^-5 is possible.
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef __linux__
#include <unistd.h> // for usleep()
#endif

#ifdef __FreeBSD__ 
#include <unistd.h>
#endif

// #ifdef __MSDOS__
// #include <dos.h> // for delay()
// #endif

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "stdinr.h" // raw keyboard mode
#include "sound.h" // sound input/output
#include "dsp.h"   // sound processing

// ============================================================================
/*
void PrintFcmpxBuff(fcmpx_buff *Buff)
{ int i,len; fcmpx *data;
  len=Buff->Len; data=Buff->Data;
  for(i=0; i<len; i++)
  { printf("%+7.4f %+7.4f  %6.4f %+6.3f\n",
	   data[i].re, data[i].im,
	   sqrt(Power(data[i])), Phase(data[i])); }
}

void PrintFloatBuff(float_buff *Buff)
{ int i,len,col; float *data;
  len=Buff->Len; data=Buff->Data;
  for(col=0,i=0; i<len; i++)
  { col+=printf(" %6.4f",data[i]);
    if(col>72) { printf("\n"); col=0; }
  } if(col) printf("\n");
}
*/
// ============================================================================
// the parameters

char DevName[32]=""; 	// the name of the sound device
char UseDevice=0;

int ReqRate=8000;	// we request 8000 Hz sampling
float DevRate=0;	// the device gives us a possibly different rate
float UserRate=0.0;	// user provided rate (overrides the DevRate)

int ReadFromFile=0;	// read real audio or playback from a file
char *PlaybackFile=NULL; // file to read audio from in playback mode
int SaveToFile=0;	// save audio to a file
char *SaveFile=NULL;	// file to save audio

int DetectEnvelope=1;	 // insert envelope detector
float CenterFreq=1000.0; // tone centered on 700 Hz
float BandWidth=1000.0;	 // signal bandwidth 500 Hz
int FilterLen=64;        // input filter length
int DecimRate=2;	 // decimation rate

float RepTime=1.00;	// 1.0 second between pulses in our timing reference
float IntegTime=10.0;	// integrate over 10 seconds
int ScanRange=128;      // we check 128 different delays
			// to see the best periodic match
int FitRange=2;		// parabole fit range around the best match
int Correlate=0;	// 0 => minimize differences, 1 => maximize correlation
int CorrelPos=0;        // correlate only rising edges of the envelope

// ============================================================================

SoundDevice AudioInput; // our sound input

#define BuffLen   512 // we process data in blocks

short int InpBuff[BuffLen];	// the input buffer
float_buff InpFloat;

QuadrSplit InpSplit;	// input filter, I/Q splitter, decimator

float_buff EnvBuff;

DelayLine<float> EnvLine;

double *IntegMid=NULL;
double *IntegOut=NULL;
float W1,W2,W5;

// ============================================================================

int main(int argc, char *argv[])
{ int arg; int ValueI; float ValueF;
  int err,wr,Level,InpLen;
  int i,s,delay;
  int PulsePeriod,MaxEnvLineDelay;
  FILE *DataLog;
  long LogTime,LogPeriod;
  float Diff,Diff1,Diff2;
  int InpFileNo,OutFileNo;

printf("\n\
Sound card sampling rate calibrator, (c) 1999 Pawel Jalocha\n\
Made for time stations running on 4996, 9996 and 14996 kHz\n\
but in principle any station sending periodic signals should do.\n\
\n");
if(argc<=1)
{ printf("\
Usage: ratecal1 <options>\n\
 the options:\n\
	 -d<device> input/output dsp device number or name [default = none]\n\
		    -d => /dev/dsp, -d1 => /dev/dsp1, etc.\n\
		    or: -d/dev/dsp, -d/dev/dsp1, etc.\n\
	 -p<file>   playback audio from a file & monitor it on -d device\n\
	 -s<file>   save audio to a file\n\
	 -r<rate>   request given sampling rate from device [8000 Hz]\n\
	 -R<rate>   the true sampling rate (if you know it already)\n\
	 -T<time>   repetition time of input pulses [1.0 sec]\n\
	 -I<time>   data integration time [10.0 sec]\n\
	 -E     (*) DO NOT detect envelope - take the signal directly\n\
	 -F<freq>   center pulse frequency [1000.0 Hz]\n\
	 -B<band>   pulse bandwidth at -3dB [1000.0 Hz]\n\
	 -f<length> FIR filter and complex splitter length [64]\n\
	 -D<decim.> decimation rate after the filter/splitter [2]\n\
	 -S<range>  number of different delays to find a match [128]\n\
	 -C[+]  (*) maximize correlation [default = minimize the difference]\n\
	 -P<+/-> (*) range for precise peak fit around the match [2 samples]\n\
(*) = option not implemented, not working or experimental\n\
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
  case 'p':
    ReadFromFile=1; PlaybackFile=argv[arg]+2; break;
  case 's':
    SaveToFile=1; SaveFile=argv[arg]+2; break;
  case 'I':
    if(sscanf(argv[arg]+2,"%f",&ValueF)!=1)
      { printf("Invalid floating point number in %s\n",argv[arg]); err+=1; }
    else IntegTime=ValueF;
    break;
  case 'S':
    if(sscanf(argv[arg]+2,"%d",&ValueI)!=1)
      { printf("Invalid decimal number in %s\n",argv[arg]); err+=1; }
    else ScanRange=ValueI;
    break;
  case 'f':
    if(sscanf(argv[arg]+2,"%d",&ValueI)!=1)
      { printf("Invalid decimal number in %s\n",argv[arg]); err+=1; }
    else FilterLen=ValueI;
    break;
  case 'D':
    if(sscanf(argv[arg]+2,"%d",&ValueI)!=1)
      { printf("Invalid decimal number in %s\n",argv[arg]); err+=1; }
    else DecimRate=ValueI;
    break;
  case 'T':
    if(sscanf(argv[arg]+2,"%f",&ValueF)!=1)
      { printf("Invalid floating point number in %s\n",argv[arg]); err+=1; }
    else RepTime=ValueF;
    break;
  case 'R':
    if(sscanf(argv[arg]+2,"%f",&ValueF)!=1)
      { printf("Invalid floating point number in %s\n",argv[arg]); err+=1; }
    else UserRate=ValueF;
    break;
  case 'E':
    DetectEnvelope=0;
    break;
  case 'F':
    if(sscanf(argv[arg]+2,"%f",&ValueF)!=1)
      { printf("Invalid floating point number in %s\n",argv[arg]); err+=1; }
    else CenterFreq=ValueF;
    break;
  case 'C':
    Correlate=1;
    if(argv[arg][2]=='+') CorrelPos=1;
    break;
  case 'P':
    if(sscanf(argv[arg]+2,"%d",&ValueI)!=1)
      { printf("Invalid decimal number in %s\n",argv[arg]); err+=1; }
    else FitRange=ValueI;
    break;
  case 'B':
    if(sscanf(argv[arg]+2,"%f",&ValueF)!=1)
      { printf("Invalid floating point number in %s\n",argv[arg]); err+=1; }
    else BandWidth=ValueF;
    break;
  default:
    printf("Unknown option %s\n",argv[arg]); err+=1;
  }
}

if(err) goto Error;

/*
Notes:\n\
- incrementing -S allows for more rate error and takes proportionally more CPU.
- increasing -D cuts down the CPU and RAM usage and allows for more rate error
  but you must reduce the bandwidth of the input filter to avoid aliasing.
- multiplying the period matched with -T increases accuracy but reduces
  the allowed rate error - use -R then, to give a more precise starting rate.
*/

if((UseDevice==0)&&(ReadFromFile==0))
{ printf("No audio device selected nor a file to playback\n"); goto Error; }

if(UseDevice) printf("Input/output audio device = %s\n",DevName);
else printf("No audio device selected\n");

if(ReadFromFile)
{ if(UseDevice)
  { err=AudioInput.OpenFileForRead(PlaybackFile,ReqRate,DevName);
    if(err<0)
    { printf("Can't open %s or %s to read audio, errno => %s\n",
	     PlaybackFile,DevName,strerror(errno)); goto Error; }
    printf("Playback mode: we read audio from %s and play it via %s\n",
	   PlaybackFile,DevName);
  } else
  { err=AudioInput.OpenFileForRead(PlaybackFile,ReqRate);
    if(err<0)
    { printf("Can't open %s to read audio, err => %s\n",
	     PlaybackFile,strerror(errno)); goto Error; }
    printf("Playback mode: we read audio from %s\n",PlaybackFile);
  }
} else
{ err=AudioInput.OpenForRead(DevName,ReqRate);
  if(err<0) { printf("Can't open %s to read audio: errno => %s\n",
		   DevName,strerror(errno)); goto Error; }
}
DevRate=AudioInput.Rate;
printf("Requested rate = %d Hz, device reported rate = %1.0f Hz\n",
       ReqRate,DevRate);
if(UserRate>0.0)
{ printf("You say, when we request %d Hz the true device rate is %4.2f Hz.\n",
	 ReqRate,UserRate);
  printf("We take this as a starting point for the calibration.\n");
  DevRate=UserRate; }

PulsePeriod=(int)floor((DevRate*RepTime)/DecimRate+0.5);
MaxEnvLineDelay=PulsePeriod+ScanRange/2;
err=EnvLine.Preset(MaxEnvLineDelay+8,2*MaxEnvLineDelay+BuffLen/DecimRate);
if(err) { printf("Can't preset the delay line - low memory ?\n"); goto Error; }

IntegMid=(double*)malloc(ScanRange*sizeof(double));
if(IntegMid==NULL) { printf("Can't allocate integrators - low memory ?\n"); goto Error; }
IntegOut=(double*)malloc(ScanRange*sizeof(double));
if(IntegOut==NULL) { printf("Can't allocate integrators - low memory ?\n"); goto Error; }
for(s=0; s<ScanRange; s++) { IntegMid[s]=0.0; IntegOut[s]=0.0; }
LowPass2Coeff(IntegTime*DevRate/DecimRate,W1,W2,W5);

printf("Input pulse repetition time = %5.3f sec = %3.1f samples\n",
       RepTime,DevRate*RepTime);
printf("We will scan repetition times from %d to %d samples (after decim.)\n",
       MaxEnvLineDelay-(ScanRange-1),MaxEnvLineDelay);
printf("and integrate the differences over %3.1f sec.\n",IntegTime);

err=InpSplit.Preset(FilterLen,(float*)NULL,(float*)NULL,DecimRate);
if(err) { printf("Can't preset the I/Q splitter - low memory ?\n"); goto Error; }
err=InpSplit.ComputeShape(2*M_PI*(CenterFreq-BandWidth/2)/DevRate,
			  2*M_PI*(CenterFreq+BandWidth/2)/DevRate,
			  WindowHanning);
if(err) { printf("Can't preset the I/Q splitter - low memory ?\n"); goto Error; }

printf("We expect pulses at %3.1f Hz, within %3.1f Hz (-3dB) bandwidth\n",
       CenterFreq,BandWidth);
printf("Filter (FIR) length is %d, decimation rate is %d\n",
	FilterLen,DecimRate);
if(DetectEnvelope==0) printf("Envelope detector is disabled\n");

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

// DataLog=fopen("ratecal1.out","w");
LogTime=0; LogPeriod=(long)floor((1.0*DevRate)/DecimRate+0.5);
printf("\nPress RETURN to stop\n\n");
while(StdinReady()==0)
{ InpLen=AudioInput.Read(InpBuff,BuffLen);
  if(InpLen<0)
  { printf("Error while reading audio from %s, errno => %s\n",
	   DevName,strerror(errno)); break; }
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

  ConvS16toFloat(InpBuff,&InpFloat,InpLen,1.0/256/128);

  printf("Input:%5.3f/%04.1f%%, ",RMS(&InpFloat),
    100.0*(InpFloat.Len-CountInRange(&InpFloat,-0.5,0.5))/InpFloat.Len);

  InpSplit.Process(&InpFloat);

//  printf("at I/Q = %6.4f, ",RMS(&InpSplit.Output));

  if(DetectEnvelope)
    ConvCmpxToPower(&InpSplit.Output,&EnvBuff);
  else
  { InpLen=InpSplit.Output.Len;
    EnvBuff.EnsureSpace(InpLen);
    for(i=0; i<InpLen; i++) EnvBuff.Data[i]=InpSplit.Output.Data[i].re;
    EnvBuff.Len=InpLen; }

  InpLen=EnvBuff.Len;
  EnvLine.Process(&EnvBuff);
  // fprintf(DataLog,"%3d: [%4d] ",InpLen,EnvLine.Zero-EnvLine.Line);
  for(s=0; s<ScanRange; s++)
  { delay=MaxEnvLineDelay-s;
    if(Correlate)
      for(i=0; i<InpLen; i++)
      { Diff1=EnvLine.InpPtr[i]-EnvLine.InpPtr[i-1];
	Diff2=EnvLine.InpPtr[i-delay]-EnvLine.InpPtr[i-delay-1];
	if((!CorrelPos)||(Diff1>0.0))
	  LowPass2(Diff1*Diff2,IntegMid[s],IntegOut[s],W1,W2,W5); }
    else
      for(i=0; i<InpLen; i++)
      { Diff=EnvLine.InpPtr[i]-EnvLine.InpPtr[i-delay];
	LowPass2(Power(Diff),IntegMid[s],IntegOut[s],W1,W2,W5); }
  }

  int MinPos,MaxPos; double MinVal,MaxVal; double A,B,C;
  float Deep; double Delta; float Width;

  LogTime+=InpLen;
  if(LogTime>LogPeriod)
  { if(Correlate)
    { MaxVal=FindMax(IntegOut,ScanRange,MaxPos);
//    fprintf(DataLog,"Min. %10.8f at %d, aver. = %10.8f\n",
//                    MinVal,MinPos,Aver);
//    for(s=0; s<ScanRange; s++)
//      fprintf(DataLog," %10.8f",IntegOut[s]);
//    fprintf(DataLog,"\n");
      if(MaxPos<FitRange)
	printf(  "Match on the edge: period>%d rate>%5.3f Hz       ",
	  MaxEnvLineDelay-MaxPos,DecimRate*(MaxEnvLineDelay-MaxPos)/RepTime);
      else if(MaxPos>=(ScanRange-FitRange))
	printf(  "Match on the edge: period<%d rate<%5.3f Hz       ",
	  MaxEnvLineDelay-MaxPos,DecimRate*(MaxEnvLineDelay-MaxPos)/RepTime);
      else
      { FitPoly2(IntegOut+MaxPos-FitRange,2*FitRange+1,A,B,C);
//        Delta=B*B-4*A*C;
//        if(Delta>0.0) Width=(-sqrt(Delta)/A); else Width=0.0;
	printf(  "Match: period=%5.2f rate=%5.3f Hz         ",
		 MaxEnvLineDelay-(MaxPos-FitRange-B/A/2),
	  DecimRate*(MaxEnvLineDelay-(MaxPos-FitRange-B/A/2))/RepTime); }
    } else // of Correlate
    { MinVal=FindMin(IntegOut,ScanRange,MinPos);
      MaxVal=FindMax(IntegOut,ScanRange,MaxPos);
//      fprintf(DataLog,"Min. %10.8f at %d, aver. = %10.8f\n",
//		       MinVal,MinPos,Aver);
//    for(s=0; s<ScanRange; s++)
//      fprintf(DataLog," %10.8f",IntegOut[s]);
//    fprintf(DataLog,"\n");
      if(MinPos<FitRange)
	printf(  "Match on the edge: period>%d rate>%5.3f Hz       ",
	 MaxEnvLineDelay-MinPos, DecimRate*(MaxEnvLineDelay-MinPos)/RepTime);
      else if(MinPos>=(ScanRange-FitRange))
	printf(  "Match on the edge: period<%d rate<%5.3f Hz       ",
	   MaxEnvLineDelay-MinPos,DecimRate*(MaxEnvLineDelay-MinPos)/RepTime);
      else
      { FitPoly2(IntegOut+MinPos-FitRange,2*FitRange+1,A,B,C);
	if(MaxVal>0.0) Deep=(MaxVal-MinVal)/MaxVal; else Deep=0.0;
//        if(A>0.0) Width=2*sqrt((MaxVal-MinVal)/A); else Width=0.0;
	printf(  "Match: period=%5.2f rate=%5.3f Hz [%3.1f%%]      ",
		 MaxEnvLineDelay-(MinPos-FitRange-B/A/2),
		 DecimRate*(MaxEnvLineDelay-(MinPos-FitRange-B/A/2))/RepTime,
		 100.0*Deep); }
//        fprintf(DataLog,"Poly2 fit pos=%6.2f, A=%f, B=%f, C=%f\n",
//                        MinPos-3-B/A/2,A,B,C);
//      for(s=MinPos-3; s<=(MinPos+3); s++)
//        fprintf(DataLog," %10.8f",IntegOut[s]);
//      fprintf(DataLog,"\n");
    }
//    else printf("Not enough data, no signal or rate error too big   ");
    LogTime-=LogPeriod; }

//  for(i=0; i<InpLen; i++)
//    fprintf(DataLog,"%3d: %6.4f\n",i,EnvLine.Zero[i]);

  printf("\r"); fflush(stdout);
}
printf("\n");
// fclose(DataLog);

// printf("Last I/Q splitter output: I,Q,Ampl,Phase\n");
// PrintFcmpxBuff(&InpSplit.Output);

// printf("Last envelope buffer:\n");
// PrintFloatBuff(&EnvBuff);

 printf("%ld samples processed = %3.1f sec\n",
     AudioInput.TotalRead,AudioInput.TotalRead/DevRate);
Stop:
 free(IntegMid); free(IntegOut);
 AudioInput.Close();
 if(OutFileNo>0) close(OutFileNo);
 return 0;

Error:
 AudioInput.Close();
 if(OutFileNo>0) close(OutFileNo);
 return 1;
}
