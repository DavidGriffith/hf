/*
 *    sound.cc  --  Sound interface for LINUX
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

#include <stdio.h>	// for debug only

#include "sound.h"

#ifdef __linux__
#include <unistd.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#endif

#ifdef __FreeBSD__ 
#include <unistd.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef __MSDOS__
#include <io.h>
#endif

// ============================================================================

SoundDevice::SoundDevice() { DevNo=0; FileNo=0; }

SoundDevice::~SoundDevice()
{ if(DevNo>0) close(DevNo); if(FileNo>0) close(FileNo); }

// Open the sound device for read,write or both: Dir=O_RDONLY/O_WRONLY/O_RDWR.
// For the moment we always open in 16-bit mono mode.
// ReqRate is the rate we want.
int SoundDevice::Open(char *Device, int ReqRate, int Dir, int NonBlock)
{ int err; int param;

#ifdef __LINUX_SOUND__
 if(NonBlock) DevNo=open(Device,Dir|O_NONBLOCK,0);
	 else DevNo=open(Device,Dir,0);
 if(DevNo<=0) goto Error;
// printf("open() OK.\n");

 param=AFMT_S16_LE; // set sample format to 16-bit, signed, little endian
 err=ioctl(DevNo,SNDCTL_DSP_SETFMT,&param);
 if((err)||(param!=AFMT_S16_LE)) goto Error;
// printf("Format selection OK.\n");

 param=0; // set mono mode
 err=ioctl(DevNo,SNDCTL_DSP_STEREO,&param);
 if((err)||(param!=0)) goto Error;
// printf("Mono mode OK.\n");

 param=ReqRate; // request the sampling rate
 err=ioctl(DevNo,SNDCTL_DSP_SPEED,&param);
 if(err) goto Error;
 Rate=param; // here the device tells us the actuall sampling rate
// printf("Rate selection OK.\n");

 if(NonBlock)
 { param=1; // set non-blocking mode - O_NONBLOCK at open() is not enough !
   err=ioctl(DevNo,SNDCTL_DSP_NONBLOCK,&param);
   if((err)||(param!=1)) goto Error; }
// printf("Nonblock OK.\n");

 return DevNo;

Error: if(DevNo>0) close(DevNo); DevNo=0; return -1;
#else
// for MSDOS we simply return an error, because we have no sound support
 return -1;
#endif

} // returns the file handle or -1 if there are problems

// open to read the sound
int SoundDevice::OpenForRead(char *Device, int ReqRate, int NonBlock)
{ int err;
  Close(); err=Open(Device, ReqRate, O_RDONLY, NonBlock);
  TotalWritten=TotalRead=0;
  return err; } // returns the device file handle (>0) or error (<0)

// open to write the sound
int SoundDevice::OpenForWrite(char *Device, int ReqRate, int NonBlock)
{ int err;
  Close(); err=Open(Device, ReqRate, O_WRONLY, NonBlock);
  TotalWritten=TotalRead=0;
  return err; }

// open for reading and writing at same time (never tried...)
int SoundDevice::OpenForReadWrite(char *Device, int ReqRate, int NonBlock)
{ int err;
  Close(); err=Open(Device, ReqRate, O_RDWR, NonBlock);
  TotalWritten=TotalRead=0;
  return err; }

// fake open for read: takes data from a file
// optionally monitors the data on a sound device
int SoundDevice::OpenFileForRead(char *FileName, int FileRate, char *MonDevice)
{ int err;
  Close();
#ifdef __MSDOS__
  FileNo=open(FileName,O_RDONLY|O_BINARY,0); if(FileNo<=0) return -1;
#else
  FileNo=open(FileName,O_RDONLY); if(FileNo<=0) return -1;
#endif
  if(MonDevice)
  { err=Open(MonDevice,FileRate,O_WRONLY,0); if(err<0) { Close(); return -1; } }
  Rate=FileRate;
  TotalWritten=TotalRead=0;
  return 0; }

int SoundDevice::OpenFileForWrite(char *FileName, int FileRate, char *MonDevice)
{ int err;
  Close();
#ifdef __MSDOS__
  FileNo=open(FileName,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,S_IREAD|S_IWRITE);
#else
  FileNo=open(FileName,O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
#endif
  if(FileNo<=0) return -1;
  if(MonDevice)
  { err=Open(MonDevice,FileRate,O_WRONLY,0); if(err<0) { Close(); return -1; } }
  Rate=FileRate;
  TotalWritten=TotalRead=0;
  return 0; }

// read samples into Buff, no more than MaxSamples
int SoundDevice::Read(short int *Buff, int MaxSamples)
{ int len,wr;
  if(FileNo>0)
  { len=read(FileNo,Buff,MaxSamples*2);
    if(len==0) return 0;
    if(len<0) return -1;
    if(DevNo>0)
    { wr=write(DevNo,Buff,len); /* if(wr!=len) return -1; */ }
  } else
  { len=read(DevNo,Buff,MaxSamples*2);
    if(len==0) return 0;
    if(len<0) return (len==EAGAIN)||(len==EBUSY) ? 0 : -1;
  } TotalRead+=len/2; return len/2;
} // returns the number of samples actually read or -1 when problems.

// see how much data is in the input buffer
int SoundDevice::ReadReady()
{
#ifdef __LINUX_SOUND__
  int err; audio_buf_info info;
  err=ioctl(DevNo,SNDCTL_DSP_GETISPACE,&info);
  return err ? -1 : info.bytes/2;
#else
  return 0;
#endif
} // returns the number of samples we can immediately read

// read 8-bit unsigned and CONVERT to 16-bit signed
// this is a "quick fix" so people with older cards are not out
/*
int SoundRead8bit(int SoundFile, short int *Buff, int MaxSamples)
{ int len,idx;
  len=read(SoundFile,Buff,MaxSamples*2);
  if(len==0) return 0;
  if(len<0) return (len==EAGAIN)||(len==EBUSY) ? 0 : -1;
  for(idx=len-1; idx; idx--) // here we convert
    { Buff[idx] = (int)((((char*)Buff)[idx])^((char)0x80)) ; }
  return len;
} // returns the number of samples actually read or -1 when problems.
*/

// write 16-bit samples into the device
int SoundDevice::Write(short int *Buff, int MaxSamples)
{ int len,wr;
  if(DevNo>0)
  { len=write(DevNo,Buff,MaxSamples*2);
    if(len==0) return 0;
    if(len<0) return (len==EAGAIN)||(len==EBUSY) ? 0 : -1;
    // if(len<0) return -1;
  } else len=MaxSamples*2;
  if(FileNo>0)
  { wr=write(FileNo,Buff,len); if(wr!=len) return -1; }
  TotalWritten+=len/2; return len/2;
} // returns the number of samples actually written or -1 when problems.

// see how much space is in the output buffer
int SoundDevice::WriteReady()
{
#ifdef __LINUX_SOUND__
  int err; audio_buf_info info;
  err=ioctl(DevNo,SNDCTL_DSP_GETOSPACE,&info);
  return err ? -1 : info.bytes/2;
#else
  return 0;
#endif
} // returns the number of samples we can immediately write in

// close the sound device (will wait until all written data is played)
int SoundDevice::Close()
{ int err1,err2;
  if(FileNo>0) { err1=close(FileNo); FileNo=0; }
          else err1=0;
  if(DevNo>0)  { err2=close(DevNo);  
                  DevNo=0;
		  printf("finally closed audio.\n"); }
          else err2=0;
  return (err1<0)||(err2<0) ? -1 : 0; }

/*
int SoundSync(int SoundFile)
{ int err;
 err=ioctl(DevNo,SNDCTL_DSP_SYNC,0);
 return err ? -1 : 0;
}
*/

// ============================================================================
/*
#ifdef __linux__

MixerDevice::MixerDevice() { DevNo=0; }

MixerDevice::~MixerDevice() { if(DevNo>0) close(DevNo); }

int MixerDevice::Open(char *Device)
{ int err,mask;

  DevNo=open(Device,0,0); if(DevNo<=0) return -1;
// The following is only to test if the mixer is really there...
//  err=ioctl(DevNo,SOUND_MIXER_READ_DEVMASK, &mask);
//  if(err) { close(DevNo); return -1; }

  return DevNo;
}

int MixerDevice::SetChanLev(int Chan, int Level)
{ return ioctl(DevNo,MIXER_WRITE(Chan),&Level); }

// int MixerDevice::SetChanLevLR(int Chan, int Left, int Right)
// { int param= ((Right&0xFF)<<8) | (Left&0xFF);
//  return ioctl(DevNo,MIXER_WRITE(Chan),&param); }


int MixerDevice::RecFromLine()
{ int param=1<<SOUND_MIXER_LINE;
  RecSource=SOUND_MIXER_LINE;
  return ioctl(DevNo,SOUND_MIXER_WRITE_RECSRC,&param); }

int MixerDevice::RecFromMic()
{ int param=1<<SOUND_MIXER_MIC;
  RecSource=SOUND_MIXER_MIC;
  return ioctl(DevNo,SOUND_MIXER_WRITE_RECSRC,&param); }

int MixerDevice::SetRecLevel(int Level)
{ int param=Level;
  return ioctl(DevNo,MIXER_WRITE(RecSource),&param); }


int MixerDevice::SetLineLevel(int Level)
{ int param=Level;
  return ioctl(DevNo,SOUND_MIXER_WRITE_LINE,&param); }

int MixerDevice::SetMicLevel(int Level)
{ int param=Level;
  return ioctl(DevNo,SOUND_MIXER_WRITE_MIC,&param); }

int MixerDevice::SetDigOutLevel(int Level)
{ int param=Level;
  return ioctl(DevNo,SOUND_MIXER_WRITE_PCM,&param); }

int MixerDevice::SetGenRecLevel(int Level)
{ int param=Level;
  return ioctl(DevNo,SOUND_MIXER_WRITE_RECLEV,&param); }

int MixerDevice::SetVolumeLevel(int Level)
{ int param=Level;
  return ioctl(DevNo,SOUND_MIXER_WRITE_VOLUME,&param); }

int MixerDevice::SetMonitorLevel(int Level)
{ int param=Level;
  return ioctl(DevNo,SOUND_MIXER_WRITE_IMIX,&param); }

#endif // __linux__
*/
// ============================================================================

