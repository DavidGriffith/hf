/*
 *    sound.h  --  Sound interface for LINUX
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

#ifndef __SOUND_H__

#define __SOUND_H__

#ifdef __linux__
#include <unistd.h>
#include <sys/fcntl.h>
#define __LINUX_SOUND__
#endif

#ifdef __MSDOS__
// #include <io.h>
// #include <dos.h>
#endif

// a simple sound device interface, to read or write sound at given rate
class SoundDevice
{ public:
   SoundDevice();
   ~SoundDevice();
     // open Device for read at given rate, optional non-blocking option
   int OpenForRead(char *Device, int ReqRate, int NonBlock=0);
     // open Device for write at given rate
   int OpenForWrite(char *Device, int ReqRate, int NonBlock=0);
     // open (a full duplex) Device for read/write at given rate
   int OpenForReadWrite(char *Device, int ReqRate, int NonBlock=0);
     // open a raw sample file for read, optionally monitor through MonDevice
   int OpenFileForRead(char *FileName, int Rate, char *MonDevice=0);
     // open a raw sample file for write, optionally monitor through MonDevice
   int OpenFileForWrite(char *FileName, int Rate, char *MonDevice=0);
     // close an open device or a file
   int Close();
     // read audio samples
   int Read(short int *Buff, int MaxSamples);
     // how many samples are there ready to be read ?
   int ReadReady();
     // write audio samples
   int Write(short int *Buff, int MaxSamples);
     // how many sample we can write into the device ?
   int WriteReady();
  public:
   int Rate;
   int DevNo;
   int FileNo;
   long int TotalRead;
   long int TotalWritten;
  private:
   int Open(char *Device, int ReqRate, int Dir, int NonBlock=1);
} ;

// the mixer stuff doesn't really work yet...

/*
#ifdef __linux__

class MixerDevice
{ public:
   MixerDevice();
   ~MixerDevice();
   int Open(char *Device);
   int Close() { int err=close(DevNo); return err ? -1 : 0; }
   int SetChanLev(int Chan, int Level);
//   int SetRecSource(int Source);

   int RecFromLine();
   int RecFromMic();
   int SetRecLevel(int Level);

   int SetMicLevel(int Level);
   int SetLineLevel(int Level);
   int SetGenRecLevel(int Level);
   int SetDigOutLevel(int Level);
   int SetVolumeLevel(int Level);
   int SetMonitorLevel(int Level);

  private:
   int DevNo;
   int RecSource; // channel we record from
} ;

#endif // __linux__
*/

#endif // of __SOUND_H__

