#include <Arduino.h>
#include "VCOs.h"

#define EG_MAX_VALUE  16
static volatile unsigned short n[VCOS_LEN];
static volatile unsigned short freqMultiplier[VCOS_LEN];
static volatile unsigned short freqMultiplierHalf[VCOS_LEN];
static volatile unsigned short eg[VCOS_LEN]; // 32 max
static volatile unsigned char lfoValueForVCA; // 128 max


void vcos_init(void)
{
  int i;
  for(i=0; i<VCOS_LEN;i++)
  {
    n[i]=0;
    freqMultiplier[i] = 0;
    freqMultiplierHalf[i] = 0;
    eg[i] = EG_MAX_VALUE;
  }
  lfoValueForVCA = 128;
}

void vcos_calculateOuts(void)
{
    signed short acc;
    unsigned char i;
    digitalWrite(13,HIGH);
    acc = 0;
    for(i=0; i<3;i++)
    {
      n[i]++;
      if(n[i]<freqMultiplierHalf[i])
      {
        //acc-= ( (32*eg[i])/256) ;
        //acc-= eg[i]/8 ;
        acc-= eg[i] ;
      }
      else if(n[i]<freqMultiplier[i])
      {
        //acc+= ((32*eg[i])/256) ;        
        //acc+= eg[i]/8 ;        
        acc+= eg[i] ;        
      }
      else
      {
        n[i]=0;
      }           
    }
    OCR1B = ((acc*lfoValueForVCA)/128) + (3*EG_MAX_VALUE);

    acc = 0;
    for(i=3; i<6;i++)
    {
      n[i]++;
      if(n[i]<freqMultiplierHalf[i])
      {
        //acc-= ((32*eg[i])/256) ;
        acc-= eg[i] ;
      }
      else if(n[i]<freqMultiplier[i])
      {
        //acc+= ((32*eg[i])/256) ;        
        acc+= eg[i];        
      }
      else
      {
        n[i]=0;
      }           
    }
    OCR1A = ((acc*lfoValueForVCA)/128) + (3*EG_MAX_VALUE);
    
    digitalWrite(13,LOW);
}

void vcos_setFrqVCO(unsigned char vcoIndex,unsigned short val)
{
    freqMultiplier[vcoIndex] = val;
    freqMultiplierHalf[vcoIndex] = val/2;
    eg[vcoIndex] = EG_MAX_VALUE; // sacar!!!!
}

void vcos_turnOff(unsigned char vcoIndex)
{
  eg[vcoIndex] = 0;
}


