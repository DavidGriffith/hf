/*
 *    mt63hf.h 
 *    
 *    by Günther Montag 2004
 *
 *    functions to link the
 *       MT63 transmitter and receiver in C++ for LINUX
 *       by Pawel Jalocha, SP9VRC
 *    to 
 *      hf by Tom Sailer 
 *    This file is part of hf's the libmt63.a
 *
 *    hf and MT63 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    hf and MT63 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with MT63; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef MT63HF_H
#define MT63HF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



int mt63_rx_start(unsigned int bandwidth, unsigned int doubleinterleave, 
    unsigned int integration, float soundcorr);
int mt63_tx_start(unsigned int bandwidth, unsigned int doubleinterleave, 
    float soundcorr, unsigned char* call);

int open_audio_input(char *DevName);
int open_audio_output(char *DevName);
void mt63_decode(short *samples, int count);
int mt63_encode(char ch); 
void mt63_tx(char c);
void mt63_rx();
void mt63_direct_rx(void);
void mt63_direct_tx(void);
void mt63_finish_tx();
void mt63_finish_rx();
void mt63_send_jam(); 
void close_audio_rx();
void close_audio_tx();

#ifdef __cplusplus
}
#endif /* __cplusplus */

extern char code[16];
#define ENCODEBUFSIZE 1602 /* is maximal length, used for bandwidth 500 */
extern short encodebuf[ENCODEBUFSIZE];
extern int mt63_data_waiting;

/* samples for the sprectrum */
extern int samples_remain;
extern int samples_count;
extern short *samples_ptr;

#endif /* MT63HF_H */
