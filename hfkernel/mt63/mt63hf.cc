/*
 *	mt63hf.cc  --  by Gnther Montag DL4MGE
 *	"glue" - links Pawel`s MT63 code to
 *	Tom Sailer's hf code		
 *	for integration of MT63 into hf
 *		
 *      MT63 is the MT63ASC receiver in C++ for LINUX,
 *      written to be compatible with the
 *      MT63ASC.ASM modem for the EVM56K/DSPCARD4.
 *
 *	Derived from mt63rx and -tx code 
 *	(Copyright (C) 1999-2004 Pawel Jalocha, SP9VRC)
 *
 *    This file is part of hf, and part of its libmt63.a
 *
 *    hf and MT63 is free software; you can redistribute it and/or modify
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
#include "mt63hf.h"

// ============================================================================

/*DEBUG*/
#ifndef DEBUG
#define DEBUG printf("%s: function %s still running at line %d...\n", \
__FILE__, __FUNCTION__,  __LINE__);

#define D DEBUG
#endif /*DEBUG*/

#define BuffLen 512
s16 InpBuff[BuffLen];	// Signed 16-bit audio input buffer
float_buff InpFloat;
LevelMonitor InpLevel;
MT63rx Rx;
MT63tx Tx;		// MT63 encoder
s16_buff OutBuff;	// S16 buffer
int ReqRate=8000;	// we request 8000 Hz sampling
float DevRate=8000.0;	// was 0, RxDev.Rate here not possible
			// because I want hfkernel running all the time
			// and rely on the user's calibration. Günther
			// (in the orig code the device gave a possibly 
			// different rate via RxDev.Rate)
float UserRate=0.0;	// user provided rate (overrides the DevRate)
int Bandwidth=1000;	// 500, 1000 or 2000 Hz bandwidth
int Interleave=0;	// short/long interleave
int IntegLen=32;        // integration period for sync./data tracking
float sound_corr = 1.0;
char *CW_ID=NULL;	// CW identification to be transmitted along the digital signal
RateConvBL RateConv;	// rate converter to adjust the sampling rate
int UseRateConv=1;
SoundDevice RxDev;	// sound device to read audio
SoundDevice TxDev;	// sound device to output audio
char code[16];

// ============================================================================

int mt63_rx_start(unsigned int bandwidth, unsigned int doubleinterleave, 
    unsigned int integration,  float soundcorr)
{ 
  int err;
  printf("Start of Multitone MT63 modem receiver, (c) 1999 Pawel Jalocha\n");

  if(!integration) {
      printf("mt63_rx_start: Invalid option integration %d",
        integration);
  } else IntegLen=integration;
  if(soundcorr < 0.9 || soundcorr > 1.1) {
      printf("mt63_rx_start: Invalid float option soundcorr %f",
      soundcorr); 
      UserRate = (float)ReqRate;
  }
  else { 
    UserRate = ReqRate / soundcorr;
    sound_corr = soundcorr;
  }
  if(doubleinterleave) Interleave = 1;
  switch(bandwidth) { 
    case 500:
    case 1000:
    case 2000:
	break;
    default: 
	printf("mt63_rx: invalid bandwidth option %d\n",
	bandwidth);
	exit (1);
  }
  Bandwidth = bandwidth; 
  if(UserRate != 8000){ 
    printf("mt63_rx_start: Soundcard sample rate corrected to %4.2f Hz.\n",
      UserRate);
    DevRate=UserRate; 
  }
  UseRateConv = (DevRate != 8000.0);
  if(UseRateConv) { 
//    err=RateConv.Preset(16,NULL,64);
    err=RateConv.Preset(16,(float**)NULL,64);
      if(err) { 
        printf
          ("Can't preset the rate converter - not enough RAM ?!\n"); 
        goto Error; }
      err=RateConv.ComputeShape(0.0,0.75*M_PI,WindowBlackman2);
      if(err) { 
        printf
          ("Can't compute the shape for the rate converter - not enough RAM ?!\n"); 
      goto Error; }
    RateConv.SetOutVsInp(8000.0/DevRate);
  }
  printf("Modem bandwidth is %d Hz with %s interleave\n",
	Bandwidth,Interleave ? "DOUBLE (64)" : "SINGLE (32)");
  printf("The time/frequency synchronizer integrates over %d symbols\n",
    IntegLen);

  err=Rx.Preset(Bandwidth,Interleave,IntegLen);
  if(err) { 
    printf("Can't preset the MT63 receiver - not enough RAM ?\n");
    goto Error; 
  }
return  0;
Error:
 exit (1);
}

// ============================================================================

int mt63_tx_start(unsigned int bandwidth, unsigned int doubleinterleave, 
    float soundcorr, unsigned char* call)
{ 
  int err, len, i;
  printf("Start of Multitone MT63 modem transmitter, (c) 1999 Pawel Jalocha\n");

  if(soundcorr < 0.9 || soundcorr > 1.1) {
      printf("mt63_tx_start: Invalid float option soundcorr %f",
      soundcorr); 
      UserRate = (float)ReqRate;
  }
  else { 
    UserRate = ReqRate / soundcorr;
    sound_corr = soundcorr;
  }
  if(doubleinterleave) Interleave = 1;
  switch(bandwidth) { 
    case 500:
    case 1000:
    case 2000:
	break;
    default: 
	printf("mt63_tx_start: invalid bandwidth option %d\n",
	bandwidth);
	exit (1);
  }
  Bandwidth = (int)bandwidth; 
  if(UserRate != 8000){ 
    printf("mt63_tx_start: Soundcard sample rate corrected to %4.2f Hz.\n",
      UserRate);
    DevRate=UserRate; 
  }
  UseRateConv = (DevRate != 8000.0);
  if(UseRateConv) { 
    err=RateConv.Preset(16,(float**)NULL,64);
      if(err) { 
        printf
          ("Can't preset the rate converter - not enough RAM ?!\n"); 
        goto Error; }
      err=RateConv.ComputeShape(0.0,0.75*M_PI,WindowBlackman2);
      if(err) { 
        printf
          ("Can't compute the shape for the rate converter - not enough RAM ?!\n"); 
      goto Error; }
    RateConv.SetOutVsInp(8000.0/DevRate);
  }
  printf("Modem bandwidth is %d Hz with %s interleave\n",
	Bandwidth,Interleave ? "DOUBLE (64)" : "SINGLE (32)");
  CW_ID = (char*)call; 
  if(strlen(CW_ID) == 0) {
      CW_ID = NULL;
      printf("no CW message.\n");
  }
  if(CW_ID!=NULL) { 
      for (i=0; CW_ID[i]; i++) {
          if(CW_ID[i]=='_') CW_ID[i]=' ';
	  //printf("%c <%d>\n", CW_ID[i], (int)CW_ID[i]);
      }
      printf("CW message \"%s\" is transmitted along the digital signal\n",
          CW_ID); 
  }
  err=Tx.Preset(Bandwidth,Interleave,CW_ID);
  if(err) { 
    printf("Can't preset the MT63 transmitter - not enough RAM ?\n");
    goto Error; 
  }
return 0;
Error:
 exit (1);
}

// ============================================================================

int open_audio_input(char *DevName) {
  int err;

    printf("Audio device for input is %s\n", DevName);
     err = RxDev.OpenForRead(DevName,(int)UserRate);
     if(err<0) { 
	  printf("Can't open %s to read audio from: errno => %s\n",
	    DevName,strerror(errno)); 
	exit(1); 
      }
    printf("Audio device opened for input: DevNo = %d\n", err);
    
// for test only
/*
  { err=RxDev.OpenFileForRead("/home/monday/mtf",8000 ); //(int)UserRate);
    if(err<0)
    { printf("Can't open %s to read audio from file\n"); exit (1); }
    printf("We read the audio from /home/monday/mtf\n");
  }
 */
  return(0);
}


// ============================================================================

int open_audio_output(char *DevName) {
  int err;

  printf("Audio device for output is %s\n", DevName);
  err = TxDev.OpenForWrite(DevName,(int)UserRate);
  if(err<0) { 
      printf("Can't open %s to write audio to: errno => %s\n",
	DevName,strerror(errno)); 
	exit(1); 
  }
  printf("Audio device opened for output: DevNo = %d\n", err);
  return(0);
}

// ============================================================================

void mt63_decode(short *samples, int count) 
{ int i, InpLen; 
    memset(code, 0, sizeof(code));
    
  //printf("decoding mt63:\n");
  for(i = 0; i < count && i < BuffLen; i++) {
    InpBuff[i] = (s16)samples[i];
  }
  InpLen = i;
  ConvS16toFloat(InpBuff,&InpFloat,InpLen);
  //printf("converted from S 16 to float: %d samples\n", count);
  InpLevel.Process(&InpFloat);
  if(UseRateConv)
  { RateConv.ProcessLinI(&InpFloat);
    Rx.Process(&RateConv.Output);
    //printf("UseRateConv: Rx.Output.Len: %d chars\n", Rx.Output.Len);
  } else Rx.Process(&InpFloat);
  //printf("Rx.Output.Len: %d chars\n", Rx.Output.Len);
  for(i=0; i<Rx.Output.Len; i++) { 
    code[i]=Rx.Output.Data[i];
/*    
    if((code[i]>=' ')||(code[i]==0x08)||(code[i]==0x09)) 
      printf("%c",code[i]);
    else if(code[i]=='\r') {
      code[i] = '\n';
      printf("%c", code[i]);
    }
    else if(code[i]!='\0') {
      //printf("<%02X>",code[i]); //orig !!
     printf("%c",code[i]); //test  !!
    }
    fflush(stdout);
*/    
  }
}

// ============================================================================

void mt63_direct_rx() 
{ 
/* experimental, initializes before open_audio, uses mt63's audio driver,
   first test, works !*/
  int i, err, InpLen; 
  char code[1024];
  
  printf("will initialize mt63 rx...\n");

  mt63_rx_start(1000, 1, 32, 1);
  printf("will open audio for mt63 rx...\n");
  if (! open_audio_input("/dev/dsp"))
    printf("audio opened for mt63.\n");
  
  for(;;) {
    InpLen=RxDev.Read(InpBuff,BuffLen);
    if(InpLen<0){ 
	printf("Error while reading audio from audio, errno => %s\n",
        strerror(errno)); exit(1); 
    }
    if(InpLen==0) { 
	usleep(50000); continue; 
    }
    ConvS16toFloat(InpBuff,&InpFloat,InpLen);
    /* printf("S16tofloat: %d samples\n", InpLen); */
    /* is 512 always */
    InpLevel.Process(&InpFloat);
    if(UseRateConv){ 
	RateConv.ProcessLinI(&InpFloat);
	Rx.Process(&RateConv.Output);
	printf("UseRateConv: Rx.Output.Len: %d chars\n", Rx.Output.Len);
	// is 0 or 1
    } else 
	Rx.Process(&InpFloat);
    /*printf("Rx.Output.Len: %d chars\n", Rx.Output.Len); */
    /* is 0 or 1 always */
    for(i=0; i<Rx.Output.Len; i++) { 
	code[i]=Rx.Output.Data[i];
	if((code[i]>=' ')||(code[i]==0x08)||(code[i]==0x09)) 
          printf("%c",code[i]);
	else if(code[i]=='\r') {
          code[i] = '\n';
	  printf("%c", code[i]);
	}
	else if(code[i]!='\0') {
          printf("<%02X>",code[i]);
	}
    }
    fflush(stdout);
  }
  printf("%s\n", code);
}

// ============================================================================


void mt63_rx() 
{ 
  int i, k, err, InpLen; 
  
    InpLen=RxDev.Read(InpBuff,BuffLen);
    if(InpLen<0){ 
	printf("Error while reading audio from audio, errno => %s\n",
        strerror(errno)); exit(1); 
    }
    if(InpLen==0) { 
	usleep(50000); 
	printf("wait...\n");
    }
    
    // write samples to the buffer for the spectrum
    if (samples_remain > 0) {
    	k = 2*samples_remain;
	if (k > BuffLen)
	    k = BuffLen;
	memcpy(samples_ptr, InpBuff, k);
    	//printf("mt63 rx: samples_remain: %d, samples_ptr: %p, i: %d\n", 
	  //  samples_remain, samples_ptr, k);  			
	samples_ptr += k/2;
	samples_remain -= k/2;
    } // else printf ("mt63rx: no demand of spectrum samples.\n");
    
    ConvS16toFloat(InpBuff,&InpFloat,InpLen);
    /* printf("S16tofloat: %d samples\n", InpLen); */
    /* is 512 always */
    InpLevel.Process(&InpFloat);
    if(UseRateConv){ 
	RateConv.ProcessLinI(&InpFloat);
	Rx.Process(&RateConv.Output);
	printf("UseRateConv: Rx.Output.Len: %d chars\n", Rx.Output.Len);
	// is 0 or 1
    } else 
	Rx.Process(&InpFloat);
    //printf("Rx.Output.Len: %d chars\n", Rx.Output.Len); 
    /* is 0 or 1 always, 
        if 0, I should make the code buffer empty
	to prevent double printed chars 
     */
    if (!Rx.Output.Len) memset(code, 0, sizeof(code));
    for(i=0; i<Rx.Output.Len; i++) { 
	code[i]=Rx.Output.Data[i];
	/*
	if((code[i]>=' ')||(code[i]==0x08)||(code[i]==0x09)) 
          printf("%c2",code[i]);
	
	else if(code[i]=='\r') {
          code[i] = '\n';
	  printf("%c", code[i]);
	}
	else if(code[i]!='\0') {
          printf("<%02X>",code[i]);
	}
	*/
    }
}

// ============================================================================

void mt63_finish_rx()  
{ 
  Rx.Free();
}

// ============================================================================

void mt63_direct_tx() 
{ 
/* experimental, initializes before open_audio, uses mt63's audio driver,
   sends test msg, works !*/
  int i, err, len, End; 
  char ch;
  char* testtext = "ich liebe dich von ganzem Herzen mit Schmerzen";  


  mt63_tx_start(1000, 0, 1.0, (unsigned char*)"MT63 ");
  
  printf("will open audio for mt63 tx...\n");
  if (! open_audio_output("/dev/dsp"))
    printf("audio opened for mt63 test-tx.\n");
  
  
  
  
/* the following code is to debug the Tx.SendTune() */
/*
  printf("Tuning ...\n");
  for(i=0; i<200; i++) // send tuning, just to test the SendTune()
  { Tx.SendTune();
    if(UseRateConv)
    { RateConv.ProcessLinI(&Tx.Comb.Output);
      ConvFloatToS16(&RateConv.Output,&OutBuff);
    } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
    len=TxDev.Write(OutBuff.Data,OutBuff.Len);
    printf("written to audio %d bytes", len);
    if(len!=OutBuff.Len)
    { printf("Error while sending out audio, errno => %s\n",strerror(errno));
      goto Error; }
  }
*/
  printf("MT63 transmiting test text:\n");
  for (i= 0; i < strlen(testtext); i++) {
    ch= testtext[i]; Tx.SendChar(ch); if(ch==0x04) End=1;
    if(ch) { 
      printf("%c",ch);
      if(ch=='\r') printf("\n"); 
      fflush(stdout); 
    }
    
    if(UseRateConv)
    { RateConv.ProcessLinI(&Tx.Comb.Output);
      ConvFloatToS16(&RateConv.Output,&OutBuff);
    } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
    len=TxDev.Write(OutBuff.Data,OutBuff.Len);
    //printf("written to audio %d bytes", len);
    if(len!=OutBuff.Len) { 
      printf("Error while sending out audio, errno => %s\n",strerror(errno));
      goto Stop; 
    }
  }

  printf("\nFlushing data interleaver ...\n");
  for(i=0; i<Tx.DataInterleave; i++){ 
    Tx.SendChar('\0');
    if(UseRateConv){ 
	RateConv.ProcessLinI(&Tx.Comb.Output);
	ConvFloatToS16(&RateConv.Output,&OutBuff);  } 
    else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
    len=TxDev.Write(OutBuff.Data,OutBuff.Len);
    //printf("flushing: written to audio %d bytes", len);
    if(len!=OutBuff.Len) { 
	printf("Error while sending out audio, errno => %s\n",strerror(errno));
	goto Stop; 
    }
  }

  printf("Sending jamming waveform ...\n");
  for(i=0; i<20; i++) { 
    Tx.SendJam();
    if(UseRateConv) { 
      RateConv.ProcessLinI(&Tx.Comb.Output);
      ConvFloatToS16(&RateConv.Output,&OutBuff);
    } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
    len=TxDev.Write(OutBuff.Data,OutBuff.Len);
    //printf("jam: written to audio %d bytes", len);
    if(len!=OutBuff.Len)  { 
	printf("Error while sending out audio, errno => %s\n",strerror(errno));
	goto Stop; 
    }
  }

Stop:
  printf("\nmt63hf.cc: Trying to close audio device DevNo ..\n");
  TxDev.Close();
  printf("Stopped OK.\n");
  printf("%ld samples sent out = %3.1f sec\n",
     TxDev.TotalWritten,TxDev.TotalWritten/DevRate);
  return ;

Error:
  TxDev.Close();
  exit (1);
}

// ============================================================================

int mt63_encode(char ch) //, short* samples, int count) 
{ 
    int i, len, End; 
    //printf("encoding for mt63 tx...\n");
    Tx.SendChar(ch); 
    if(ch == 0x04) End=1;
    if(ch) { 
      printf("%c",ch);
      if(ch=='\r') printf("\n"); 
      fflush(stdout); 
    }
    if(UseRateConv)
    { RateConv.ProcessLinI(&Tx.Comb.Output);
      ConvFloatToS16(&RateConv.Output,&OutBuff);
    } else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);

//   len=TxDev.Write(OutBuff.Data,OutBuff.Len);
//   replaced last line, want to use hf's audio driver
//   so i write to hf's tx buffer.
    for(i = 0; i <= OutBuff.Len && i < ENCODEBUFSIZE; i++) {  
	encodebuf[i] = (short)OutBuff.Data[i];
    }
//    printf("mt63_encode: written %d samples to output buffer\n", i);
    if(i < OutBuff.Len) { 
	printf("But sorry, only written %d of %d samples to output buffer\n", 
	    i, OutBuff.Len);
        printf("Error while sending out audio, errno => %s\n",
	    strerror(errno));
    }
    return OutBuff.Len;
}

// ============================================================================

void mt63_finish_tx() 
{ 
  int i, err, len, End; 
  printf("\nFlushing data interleaver ...\n");
  for(i=0; i<Tx.DataInterleave; i++){ 
    Tx.SendChar('\0');
    if(UseRateConv){ 
	RateConv.ProcessLinI(&Tx.Comb.Output);
	ConvFloatToS16(&RateConv.Output,&OutBuff);  } 
    else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
    len=TxDev.Write(OutBuff.Data,OutBuff.Len);
    //printf("flushing: written to audio %d bytes", len);
    if(len!=OutBuff.Len) { 
	printf("Error while sending out audio, errno => %s\n",strerror(errno));
	exit (1); 
    }
  }
}

// ============================================================================

void mt63_tx(char c) 
{ 
 // char* testtext = "ich liebe dich von ganzem Herzen mit Schmerzen";  
 // char ch;
  int i, k, len, End;
/*
      
  printf("MT63 transmiting test text:\n");
  for (i= 0; i < strlen(testtext); i++) {
    ch= testtext[i]; Tx.SendChar(ch); if(ch==0x04) End=1;
    if(ch) { 
      printf("%c",ch);
      if(ch=='\r') printf("\n"); 
      fflush(stdout); 
  }
*/


  //printf("MT63 transmiting character:\n");
     
    Tx.SendChar(c); 
    if(c==0x04) End=1;
    //if(c) { 
      
      if(UseRateConv) {
	RateConv.ProcessLinI(&Tx.Comb.Output);
        ConvFloatToS16(&RateConv.Output,&OutBuff);
      } else 
        ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
      
      len=TxDev.Write(OutBuff.Data,OutBuff.Len);
      //printf("written to audio %d bytes", len);
         
    // write samples to the buffer for the spectrum
    if (samples_remain > 0) {
    	k = 2*samples_remain;
	if (k > BuffLen)
	    k = BuffLen;
	memcpy(samples_ptr, OutBuff.Data, k);
    	//printf("mt63 tx: samples_remain: %d, samples_ptr: %p, i: %d\n", 
	  //samples_remain, samples_ptr, k);  			
	samples_ptr += k/2;
	samples_remain -= k/2;
    }  //else printf ("mt63tx: no demand of spectrum samples.\n");
    

     /*
      printf("%c",c);
      if(c=='\r') printf("\n"); 
      fflush(stdout); 
  */
      if(len!=OutBuff.Len) { 
        printf("Error while sending out audio, errno => %s\n",strerror(errno));
//      goto Stop; 
      }
    //}
}

// ============================================================================

void mt63_send_jam() 
{
	int i;
	Tx.SendJam();
	if(UseRateConv) { 
    	    RateConv.ProcessLinI(&Tx.Comb.Output);
            ConvFloatToS16(&RateConv.Output,&OutBuff);
	} else ConvFloatToS16(&Tx.Comb.Output,&OutBuff);
	//printf("encoded %d samples of jam\n", OutBuff.Len); 
	for(i = 0; i <= OutBuff.Len && i < ENCODEBUFSIZE; i++) {  
	    encodebuf[i] = (short)OutBuff.Data[i];
	}
	printf("written %d samples of jam to output buffer\n", i); 
	if(i < OutBuff.Len) { 
	    printf("But only written %d of %d jam samples to output buffer\n", 
		i, OutBuff.Len);
    	    printf("Error while sending out audio, errno => %s\n",
		strerror(errno));
	}
}

// ============================================================================

void close_audio_rx() 
{
    printf("\nMT63: Closing audio device ...\n");
    if (RxDev.Close()) printf("sound device could not be closed.\n");
    printf("Stopped OK.\n");
    printf("%ld samples sent out = %3.1f sec\n",
     RxDev.TotalWritten,RxDev.TotalWritten/DevRate);
}

// ============================================================================

void close_audio_tx() 
{
    printf("\nMT63: Closing audio device ...\n");
    if (TxDev.Close()) printf("sound device could not be closed.\n");
    printf("Stopped OK.\n");
    printf("%ld samples sent out = %3.1f sec\n",
     TxDev.TotalWritten,TxDev.TotalWritten/DevRate);
}

// ============================================================================

