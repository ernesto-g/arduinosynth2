#include <Arduino.h>
#include "VCOs.h"
#include "Lfo.h"

#define EG_MAX_VALUE  16

static volatile unsigned short n[VCOS_LEN];
static volatile unsigned short freqMultiplier[VCOS_LEN];
static volatile unsigned short freqMultiplierHalf[VCOS_LEN];
volatile unsigned char lfoValueForVCA; // 128 max

// EG management
#define EG_STATE_IDLE             0
#define EG_STATE_START_ATTACK     1
#define EG_STATE_RUNNING_ATTACK   2
#define EG_STATE_AT_MAX           3
#define EG_STATE_START_RELEASE    4
#define EG_STATE_RUNNING_RELEASE  5
static volatile unsigned short eg[VCOS_LEN];
static volatile unsigned char egState[VCOS_LEN];
static volatile unsigned short egDividers[VCOS_LEN];
volatile unsigned int egCounter; // affected by lfo interrupt
//______________

// Values set by front panel potenciometers
static volatile unsigned short eg1Attack=40/8;
static volatile unsigned short eg2Attack=512;
static volatile unsigned short eg1Release=40/8;
static volatile unsigned short eg2Release=512;
volatile unsigned char lfoValueForVCAInPanel; // 127 max
//_________________________________________

void vcos_init(void)
{
  int i;
  for(i=0; i<VCOS_LEN;i++)
  {
    n[i]=0;
    freqMultiplier[i] = 0;
    freqMultiplierHalf[i] = 0;
    eg[i] = EG_MAX_VALUE;
    egState[i] = EG_STATE_IDLE;
  }
  lfoValueForVCA = 0;
  lfoValueForVCAInPanel=0;
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
    OCR1B = (((acc*lfoValueForVCA)/128)*lfoValueForVCAInPanel)/128 + (((acc))*(128-lfoValueForVCAInPanel))/128 + (3*EG_MAX_VALUE);

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
    OCR1A = (((acc*lfoValueForVCA)/128)*lfoValueForVCAInPanel)/128 + (((acc))*(128-lfoValueForVCAInPanel))/128 + (3*EG_MAX_VALUE);

    egCounter++;

    digitalWrite(13,LOW);
}

void vcos_setFrqVCO(unsigned char vcoIndex,unsigned short val)
{
    freqMultiplier[vcoIndex] = val;
    freqMultiplierHalf[vcoIndex] = val/2;
    egState[vcoIndex] = EG_STATE_START_ATTACK;
}

void vcos_turnOff(unsigned char vcoIndex)
{
    egState[vcoIndex] = EG_STATE_START_RELEASE;
}


static unsigned char indexStateMachine=0;

static unsigned char allEgsIdle(void)
{
  unsigned char i;
  for(i=0; i<VCOS_LEN; i++)
  {
    if(egState[i]!=EG_STATE_IDLE)
      return 0;
  }
  return 1;
}
void vcos_egStateMachine(void)
{
  unsigned char i = indexStateMachine;
  
  //for(i=0; i<VCOS_LEN; i++)
  {
    switch(egState[i])
    {
      case EG_STATE_IDLE:
      {
        break;    
      }
      case EG_STATE_START_ATTACK:
      {
        egDividers[i] = eg1Attack;
        egState[i] = EG_STATE_RUNNING_ATTACK;
        eg[i] = 0;
        egCounter=0;
        break;
      }
      case EG_STATE_RUNNING_ATTACK:
      {
        if(egCounter>24) // 1ms
        {
          egCounter=0;
          if(egDividers[i]>0)
            egDividers[i]--;
          if(egDividers[i]==0)
          {
            egDividers[i] = eg1Attack;
            eg[i]++;
            if(eg[i]==EG_MAX_VALUE)
              egState[i] = EG_STATE_AT_MAX;
          }
        }
        break;
      }
      case EG_STATE_AT_MAX:
      {
        break;
      }
      case EG_STATE_START_RELEASE:
      {
        egDividers[i] = eg1Release;
        egState[i] = EG_STATE_RUNNING_RELEASE;
        egCounter=0;        
        break;
      }
      case EG_STATE_RUNNING_RELEASE:
      {
        if(egCounter>24) // 1ms
        {
          egCounter=0;
          if(egDividers[i]>0)
            egDividers[i]--;
          if(egDividers[i]==0)
          {
            egDividers[i] = eg1Release;
            eg[i]--;
            if(eg[i]==0)
              egState[i] = EG_STATE_IDLE;
          }
        }        
        break;
      }
    }
  }

  indexStateMachine++;
  if(indexStateMachine>=VCOS_LEN)
  {
    indexStateMachine=0;
    if(allEgsIdle())
    {
      lfo_outOff(); // no pressed keys, turn the lfo off (avoid low freq noise)
    }
  }
}

void vcos_setEg1Attack(byte value)
{
  eg1Attack = value;
}
void vcos_setEg2Attack(byte value)
{
  eg2Attack = value;
}
void vcos_setEg1Release(byte value)
{
  eg1Release = value;
}
void vcos_setEg2Release(byte value)
{
  eg2Release = value;
}

void vcos_setLfoForVCAModulation(byte value)
{
  lfoValueForVCAInPanel = value;  
}

