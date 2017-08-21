/**
 *  Arduino Digital Synth with analog filter
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
//#include <HardwareSerial.h>
#include <Arduino.h>
#include "Config.h"

#define USART_BAUDRATE 31250
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
 
void config_init(void)
{
  
  // Configure serial port for MIDI input
  //Serial.begin(31250); 
    UBRR0L = (uint8_t)(BAUD_PRESCALE & 0xff);
    UBRR0H = (uint8_t)(BAUD_PRESCALE >> 8);
    UCSR0B =
        /* interrupt on receive */
        (1 << RXCIE0) | (1 << TXEN0) |
        (1 << RXEN0);
    UCSR0C =
        /* no parity bit */
        (0 << UPM01) |
        (0 << UPM00) |
        /* asyncrounous USART */
        (0 << UMSEL01) |
        (0 << UMSEL00) |
        /* one stop bit */
        (0 << USBS0) |
        /* 8-bits of data */
        (1 << UCSZ01) |
        (1 << UCSZ00);

    DDRD = 0x00;
    DDRB = 0x20;  
  //_____________________________________


  // Configure TIMER1 for PWM (VCO1 and VCO2 Control Voltage)
  DDRB |= (1 << DDB1) | (1 << DDB2);
  TCCR1A =
      (1 << COM1A1) | (1 << COM1B1) |
      // Fast PWM mode.
      (1 << WGM11);
  TCCR1B =
      // Fast PWM mode.
      (1 << WGM12) | (1 << WGM13) |
      // No clock prescaling (fastest possible
      // freq).
      (1 << CS10);
  OCR1A = 0;
  OCR1B = 0;
  //ICR1 = 512;//31khz
  //ICR1 = 384; // 41Khz
  //ICR1 = 192;
  //ICR1 = 164; // 96Khz
  ICR1 = 96; // OK
  //ICR1 = 144; // OK
  //__________________________


  // Configure TIMER2 for PWM (LFO output)
  DDRD |= (1 << DDD3);
  TCCR2A =
      (1 << COM2B1) |
      // Fast PWM mode.
      (1 << WGM20) | (1 << WGM21);
  TCCR2B =
      // No clock prescaling (fastest possible
      // freq).
      (1 << CS20);
  OCR2A = 0;
  OCR2B = 0;
  //ICR2 = 255;
  //__________________________
  
  // Configure TIMER0 for DDS interrupt
  cli();//stop interrupts
  // each 133uS 
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments

  OCR0A = 12; // 52uS 
  //OCR0A = 9; // 42uS // falla recepcion uart

  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei();//allow interrupts
  //___________________________________
  

  // Init Pins
  pinMode(PIN_LED_MODE0,OUTPUT);
  digitalWrite(PIN_LED_MODE0,LOW);

  pinMode(PIN_LED_MODE1,OUTPUT);
  digitalWrite(PIN_LED_MODE1,LOW);

  pinMode(PIN_LED_REPEAT,OUTPUT);
  digitalWrite(PIN_LED_REPEAT,LOW);

  pinMode(PIN_BUTTON,INPUT);
  pinMode(PIN_VCO_WAVEFORM_SWITCH,INPUT);
  pinMode(PIN_LFO_WAVEFORM_SWITCH,INPUT);

  //__________
   
 }


void USART_Transmit( unsigned char data )
{
 /* Wait for empty transmit buffer */
 while ( !( UCSR0A & (1<<5)) );
 /* Put data into buffer, sends the data */
 UDR0 = data;
}
void printHex(byte val)
{
  byte h = val>>4;
  if(h<=9)
    h = h + '0';
  else
    h = (h-0x0A) + 'A';
    
  byte l = val&0x0F;
  if(l<=9)
    l = l + '0';
  else
    l = (l-0x0A) + 'A';

  USART_Transmit('0');
  USART_Transmit('x');
  USART_Transmit(h);
  USART_Transmit(l);
  USART_Transmit(' ');
}

