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
#include <HardwareSerial.h>
#include <Arduino.h>
#include "Config.h"
#include "MidiManager.h"
#include "AnalogIns.h"
#include "Lfo.h"
#include "SequencerManager.h"
#include "VCOs.h"



const unsigned short NOTES_TABLE_PWM[61] = {294
,278
,262
,247
,233
,220
,208
,196
,185
,175
,165
,156
,147
,139
,131
,124
,117
,110
,104
,98
,93
,87
,83
,78
,74
,69
,65
,62
,58
,55
,52
,49
,46
,44
,41
,39
,37
,35
,33
,31
,29
,28
,26
,25
,23
,22
,21
,19
,18
,17
,16
,15
,15
,14
,13
,12
,12
,11
,10
,10
,9};


MidiInfo midiInfo;
byte midiStateMachine=MIDI_STATE_IDLE;
unsigned char voicesMode;

void midi_analizeMidiInfo(MidiInfo * pMidiInfo);
static void showMode(void);
static byte saveKey(byte note);
static byte getIndexOfPressedKey(byte note);
static byte deleteKey(byte note);
static byte thereAreNoKeysPressed(void);
static byte getTheLowestKeyPressed(void);
static byte getTheHighestKeyPressed(void);
static unsigned char setVCOs(byte note);
static byte getNextKeyForRepeat(void);
static void setMidiControl(byte control, byte value);
static void setVCOforGliss(byte note);
static void stopVCOforGliss(void);

static unsigned int currentRepeatValue;
static unsigned char repeatRunning;
static unsigned char repeatOn;
static unsigned char repeatKeyIndex;

volatile unsigned int repeatCounter; // incremented in lfo interrupt

static KeyPressedInfo keysPressed[KEYS_PRESSED_LEN];

// Poliphonic management
#define VCOS_KEYS_LEN       VCOS_LEN
static KeyPressedInfo vcosKeys[VCOS_KEYS_LEN];
static byte deleteVCOKey(byte note);




void midi_init(void)
{
  byte i;
  midiStateMachine=MIDI_STATE_IDLE;

  for(i=0; i<KEYS_PRESSED_LEN; i++)
    keysPressed[i].flagFree=1;

  for(i=0; i<VCOS_KEYS_LEN; i++)
    vcosKeys[i].flagFree=1;

  repeatKeyIndex=0;
  voicesMode = MIDI_MODE_0;
  

  repeatCounter = 0;
  repeatRunning=0;
  currentRepeatValue=0;
  repeatOn=0;
  showMode();

  vcos_setEg1Attack(1); // max: 32. min: 0
  vcos_setEg2Attack(1);
  vcos_setEg1Release(5); // max : 64: min : 0
  vcos_setEg2Release(5);
  vcos_setLfoForVCAModulation(0);
  vcos_setLfoForVCFModulation(0);
}


void midi_analizeMidiInfo(MidiInfo * pMidiInfo)
{
    if(pMidiInfo->channel==MIDI_CURRENT_CHANNEL)
    {
        if(pMidiInfo->cmd==MIDI_CMD_NOTE_ON)
        {
            // NOTE ON 
            if(pMidiInfo->note>=36 && pMidiInfo->note<=96)
            { 

              saveKey(pMidiInfo->note);
             
              if(repeatOn==1)
                return; // repeat is playing, save key to repeat later, but ignore current key hit

              lfo_reset();
              lfo_outOn();

              if(seq_isRecording())
                seq_startRecordNote(pMidiInfo->note);

              setVCOs(pMidiInfo->note);              
            }
        }
        else if(pMidiInfo->cmd==MIDI_CMD_NOTE_OFF)
        {
            // NOTE OFF
            if(repeatOn==0)
            {
              byte vcoIndexToTurnOff = deleteVCOKey(pMidiInfo->note);
              vcos_turnOff(vcoIndexToTurnOff);
            }
            
            if(deleteKey(pMidiInfo->note))
            {           
              if(seq_isRecording())
                seq_endRecordNote();
            }
        }
        else if(pMidiInfo->cmd==MIDI_CMD_CONTROL_CHANGE)
        {
            byte controlNumber = pMidiInfo->note;
            byte controlValue = pMidiInfo->vel;
            //printHex(controlNumber);
            //printHex(controlValue);     
            setMidiControl(controlNumber, controlValue);
        }
        else
        {
          // unsuported command
          
        }
    }     
}


void midi_stateMachine(byte midiByte)
{
  switch(midiStateMachine)
  {
      case MIDI_STATE_IDLE:
        {
        // read status byte 
        midiInfo.channel = midiByte & B00001111;
        midiInfo.cmd = midiByte & B11110000;
        midiStateMachine = MIDI_STATE_RVC_DATA1;
        break;
      }
      case MIDI_STATE_RVC_DATA1:
      {
        // read note
        midiInfo.note = midiByte;
        midiStateMachine = MIDI_STATE_RVC_DATA2;        
        break;
      }
      case MIDI_STATE_RVC_DATA2:
      {
        // read velocity
        midiInfo.vel = midiByte;
        midiStateMachine = MIDI_STATE_IDLE;
        midi_analizeMidiInfo(&midiInfo);
        break;
      }
  }
  
}

void midi_startNote(unsigned char midiNoteNumber)
{
    MidiInfo mi;
    mi.cmd = MIDI_CMD_NOTE_ON;
    mi.note = midiNoteNumber;
    mi.channel = MIDI_CURRENT_CHANNEL;
    midi_analizeMidiInfo(&mi);
}
void midi_stopNote(unsigned char midiNoteNumber)
{
    MidiInfo mi;
    mi.cmd = MIDI_CMD_NOTE_OFF;
    mi.note = midiNoteNumber;
    mi.channel = MIDI_CURRENT_CHANNEL;
    midi_analizeMidiInfo(&mi);  
}

static byte note2PlayForRepeat;
void midi_repeatManager(void)
{
  if(currentRepeatValue>0)
  {
    repeatOn=1;    
    if(repeatCounter>=currentRepeatValue)
    {
        repeatCounter=0;
        note2PlayForRepeat = getNextKeyForRepeat();
        if(note2PlayForRepeat!=0xFF)
        {
          lfo_reset();
          lfo_outOn();
          if(setVCOs(note2PlayForRepeat)==0)
          {
            // repeat is too fast for declared EG parameters
            note2PlayForRepeat = 0xFF;
          }
        }
        digitalWrite(PIN_LED_REPEAT,HIGH);      
        repeatRunning=1;
    }
    else
    {
      if(repeatRunning==1 && repeatCounter>=(4 + (currentRepeatValue/8) ) ) // wait (100ms + a proportional time) to disable trigger and gate
      {
        digitalWrite(PIN_LED_REPEAT,LOW);            
        repeatRunning=0;

        if(note2PlayForRepeat!=0xFF)
        {
          byte vcoIndexToTurnOff = deleteVCOKey(note2PlayForRepeat);
          vcos_turnOff(vcoIndexToTurnOff);
        }            
      }
    }
  }
  else 
  {
    repeatOn=0;
    if(repeatRunning==1)
    {
        digitalWrite(PIN_LED_REPEAT,LOW);      
        repeatRunning=0;
        byte vcoIndexToTurnOff = deleteVCOKey(note2PlayForRepeat);
        vcos_turnOff(vcoIndexToTurnOff);
    }
  }
}


void midi_setRepeatValue(unsigned int repeatVal)
{
//  if(voicesMode==MIDI_MODE_SECUENCER)
//  {
//    seq_setSpeed((1023-repeatVal)/12);
//    currentRepeatValue=0;
//  }
//  else
  {      
      if(repeatVal<64)
      {
          currentRepeatValue=0;  
      }
      else
      {   
          repeatVal-=64;
                    
          repeatVal = 1024 - repeatVal; // invert value
          currentRepeatValue = (repeatVal+30)/12 ; // currentRepeatValue between 2 and 158 (50ms to 1.9s)
      }
  }
}




void midi_buttonPressedLongCallback(void)
{
  /*
    if(voicesMode==MIDI_MODE_SECUENCER)
    {
        if(seq_isRecording()==0)
        {
          // start recording mode
          seq_startRecord();
        }
        else
        {
          // play mode
          seq_startPlay();
        }
    }
    */
}

void midi_buttonPressedShortCallback(void)
{    
    voicesMode++;
    if(voicesMode>=MIDI_MODES_LEN)
      voicesMode=0;

    showMode();

    vcos_setEGmode(voicesMode);

//    if(voicesMode==MIDI_MODE_SECUENCER)
//      seq_startPlay();
//    else
    {
        byte i;
        for(i=0; i<KEYS_PRESSED_LEN; i++)
          keysPressed[i].flagFree=1;
      seq_stopPlay();
    }
}


static void showMode(void)
{
    digitalWrite(PIN_LED_MODE0,LOW);
    digitalWrite(PIN_LED_MODE1,LOW);
  
    switch(voicesMode)
    {
      case MIDI_MODE_0:
        digitalWrite(PIN_LED_MODE0,HIGH);
        break;
      case MIDI_MODE_1:
        digitalWrite(PIN_LED_MODE1,HIGH);
        break;
    }  
}


static byte saveKey(byte note)
{
  byte i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==1)
    {
        keysPressed[i].flagFree=0;
        keysPressed[i].note = note;
        return 0;
    }
  }
  return 1; // no more space
}
static byte getIndexOfPressedKey(byte note)
{
  byte i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==0)
    {
        if(keysPressed[i].note == note)
          return i;
    }
  }
  return 0xFF; 
}
static byte deleteKey(byte note)
{
    byte index = getIndexOfPressedKey(note);
    if(index<KEYS_PRESSED_LEN)
    {
      keysPressed[index].flagFree=1;
      return 1;
    }
    return 0;
}

static byte thereAreNoKeysPressed(void)
{
  byte i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==0)
    {
      return 0;
    }
  }
  return 1; 
}

static byte getTheLowestKeyPressed(void)
{
    byte i;
    byte mi = 0xFF;
    for(i=0; i<KEYS_PRESSED_LEN; i++)
    {
      if(keysPressed[i].flagFree==0)
      {
          if(keysPressed[i].note<mi)
            mi = keysPressed[i].note;
      }
    }
    return mi;
}

static byte getTheHighestKeyPressed(void)
{
    byte i;
    byte max = 0x00;
    for(i=0; i<KEYS_PRESSED_LEN; i++)
    {
      if(keysPressed[i].flagFree==0)
      {
          if(keysPressed[i].note>max)
            max = keysPressed[i].note;
      }
    }
    return max;
}

static byte getNextKeyForRepeat(void)
{
    byte i;
    byte found=0;
    byte ret=0xFF;
    for(i=repeatKeyIndex; i<KEYS_PRESSED_LEN; i++)
    {
        if(keysPressed[i].flagFree==0)
        {
          ret = keysPressed[i].note;
          found = 1;
          break;
        }
    }
    if(found==0)
    {
        repeatKeyIndex = 0;
        for(i=repeatKeyIndex; i<KEYS_PRESSED_LEN; i++)
        {
            if(keysPressed[i].flagFree==0)
            {
              ret = keysPressed[i].note;
              break;
            }
        }        
    }
    
    repeatKeyIndex = i+1;
    if(repeatKeyIndex>=KEYS_PRESSED_LEN)
      repeatKeyIndex = 0;

   return ret;
}



// **************** Poliphonic management

static byte saveVCOKey(byte note)
{
  byte i;
  for(i=0; i<VCOS_KEYS_LEN; i++)
  {
    if(vcosKeys[i].flagFree==1)
    {
        vcosKeys[i].flagFree=0;
        vcosKeys[i].note = note;
        return i;
    }
  }
  return 0xFF; // no more space
}
static byte getIndexOfVCOKey(byte note)
{
  byte i;
  for(i=0; i<VCOS_KEYS_LEN; i++)
  {
    if(vcosKeys[i].flagFree==0)
    {
        if(vcosKeys[i].note == note)
          return i;
    }
  }
  return 0xFF; 
}

static byte deleteVCOKey(byte note)
{
    byte index = getIndexOfVCOKey(note);
    if(index<VCOS_KEYS_LEN)
    {
      vcosKeys[index].flagFree=1;
      return index;
    }
    return 0xFF;
}

static unsigned char setVCOs(byte note)
{
      byte vcoIndex = saveVCOKey(note);
      if(vcoIndex!=0xFF)
      {  
        vcos_setFrqVCO(vcoIndex,NOTES_TABLE_PWM[note-36]); 
        return 1;
      }      
      return 0;
}

static void setVCOforGliss(byte note)
{
    vcos_setFrqVCO(5,NOTES_TABLE_PWM[note-36]); // VCO index=5 (last VCO)
}
static void stopVCOforGliss(void)
{
    vcos_turnOff(5); // VCO index=5 (last VCO)
}



static void setMidiControl(byte control, byte value)
{
  switch(control)
  {
      case MIDI_CONTROL_EG1_ATTACK:
      {
        vcos_setEg1Attack(value/4);
        break;
      }
      case MIDI_CONTROL_EG1_RELEASE:
      {
        vcos_setEg1Release(value/2);
        break;
      }
      case MIDI_CONTROL_EG2_ATTACK:
      {
        vcos_setEg2Attack(value/4);
        break;
      }
      case MIDI_CONTROL_EG2_RELEASE:
      {
        vcos_setEg2Release(value/2);        
        break;
      }
      case MIDI_CONTROL_LFO_SPEED:
      {
        lfo_setFrequencyMultiplier(value*2);
        break;
      }
      case MIDI_CONTROL_VCA_LFO_MODULATION:
      {
        vcos_setLfoForVCAModulation(value);
        break;
      }
      case MIDI_CONTROL_VCF_LFO_MODULATION:
      {
        vcos_setLfoForVCFModulation(value);
        break;
      }
      case MIDI_CONTROL_REPEAT:
      {
        midi_setRepeatValue(((unsigned int)value)*8);
        break;
      }
  }
}


