/**
 *  Arduino Analog-Digital Synth
    Copyright (C) <2017>  Ernesto Gigliotti <ernestogigliotti@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Config.h"
#include "MidiManager.h"
#include "Lfo.h"
#include "AnalogIns.h"
#include "FrontPanel.h"
#include "Outs.h"
#include "SequencerManager.h"
#include "VCOs.h"

volatile unsigned char bufferRx[5][3];
volatile unsigned char bufferRxIndex[5]={0,0,0,0,0};
volatile unsigned char bufferRxNewPacket[5]={0,0,0,0,0};
volatile unsigned char currentBufferIndex=0;

ISR(USART_RX_vect)
{
    if(bufferRxNewPacket[currentBufferIndex]==0)
    {
      bufferRx[currentBufferIndex][bufferRxIndex[currentBufferIndex]] = UDR0;
      bufferRxIndex[currentBufferIndex]++;
      if(bufferRxIndex[currentBufferIndex]>=3)
      {
        bufferRxNewPacket[currentBufferIndex] = 1;
        currentBufferIndex++;
        if(currentBufferIndex>=5)
          currentBufferIndex=0;
      }
    }
    else
    {
      uint8_t r = UDR0;
      r = r; // silence compiler warnings  
    }
}

void setup() {
  config_init();    
  outs_init();
  midi_init();
  vcos_init();
  lfo_init();
  ain_init();
  frontp_init();
  seq_init();
}



void loop() {

  // prueba pin de led como salida
  pinMode(13,OUTPUT);
  OCR1B = 512;    
  OCR1A = 512;     
  USART_Transmit('h');         
  USART_Transmit('o');         
  USART_Transmit('l');         
  USART_Transmit('a');         
  printHex(0x5F);
  //______________________________


  while(1)
  {
    // MIDI Reception
    unsigned char index;
    for(index=0; index<5; index++)
    {
      if(bufferRxNewPacket[index])
      {       
          midi_stateMachine(bufferRx[index][0]);
          midi_stateMachine(bufferRx[index][1]);
          midi_stateMachine(bufferRx[index][2]);      
          bufferRxIndex[index]=0;
          bufferRxNewPacket[index]=0;
      }
    }
    //_______________    

    lfo_cooperativeTimer0Interrupt();

    ain_state_machine();

    vcos_egStateMachine();

    midi_repeatManager();
    midi_glissManager();


    /*
    frontp_state_machine();


    outs_stateMachine();

    seq_stateMachine();
    */
    
  }

}
