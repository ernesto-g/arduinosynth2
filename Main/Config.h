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
void config_init(void);


#define PIN_BUTTON                8
#define PIN_GLISS_SWITCH          12
#define PIN_LFO_WAVEFORM_SWITCH   11

#define PIN_LED_MODE0     6
#define PIN_LED_MODE1     7
#define PIN_LED_REPEAT    2


void USART_Transmit( unsigned char data );
void printHex(byte val);   




