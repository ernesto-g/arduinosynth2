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
#include <avr/pgmspace.h>
#include "Config.h"
#include "AnalogIns.h"
#include "Lfo.h"
#include "MidiManager.h"
#include "FrontPanel.h"

volatile unsigned int btnPressedCounter; // incremented in lfo interrupt timer

static unsigned char state;
static unsigned char controlIndex;
static unsigned char stateButton;

static unsigned char getDiscrete4ValuesFromSwitchSelector(uint16_t analogValue);
static unsigned char getDiscrete5ValuesFromSwitchSelector(uint16_t analogValue);
static void setValueToManager(unsigned char controlIndex);

void frontp_init(void)
{
    state = FRONTPANEL_STATE_IDLE;
    stateButton = FRONTPANEL_BTN_STATE_IDLE;    
}

void frontp_state_machine(void)
{
    switch(state)
    {
        case FRONTPANEL_STATE_IDLE:
        {
          state = FRONTPANEL_STATE_UPDATE_GLISS;
          break;
        }
        case FRONTPANEL_STATE_UPDATE_GLISS:
        {
          // update gliss switch value
          if(digitalRead(PIN_GLISS_SWITCH)==HIGH)
            midi_setGlissOn(1);
          else
            midi_setGlissOn(0);
          //__________________________  
          state = FRONTPANEL_STATE_UPDATE_LFO_WAVEFORM;
          break;
        }
        case FRONTPANEL_STATE_UPDATE_LFO_WAVEFORM:
        {
          // update gliss switch value
          if(digitalRead(PIN_LFO_WAVEFORM_SWITCH)==HIGH)
            lfo_setWaveType(LFO_WAVE_TYPE_SINE);
          else
            lfo_setWaveType(LFO_WAVE_TYPE_EXP);
          //__________________________  
          state = FRONTPANEL_STATE_IDLE;          
          break;
        }
    }

    switch(stateButton)
    {
        case FRONTPANEL_BTN_STATE_IDLE:
        {
            if(digitalRead(PIN_BUTTON)==HIGH)
            {
                stateButton=FRONTPANEL_BTN_WAIT_RELEASE;
                btnPressedCounter=0; // this counter is modified in lfo interrupt at 25ms rate
            }
            break;
        }
        case FRONTPANEL_BTN_WAIT_RELEASE:
        {
          if(digitalRead(PIN_BUTTON)==LOW)
          {
              stateButton=FRONTPANEL_BTN_END_RELEASE;
          }
          break;
        }
        case FRONTPANEL_BTN_END_RELEASE:
        {
          if(btnPressedCounter>100) // 2500 ms
          {
            // valid press
            midi_buttonPressedLongCallback();
          }
          else if(btnPressedCounter>5) // 125 ms
          {
            // valid press
            midi_buttonPressedShortCallback();            
          }
          stateButton=FRONTPANEL_BTN_STATE_IDLE;
          break;
        }
    }
}



