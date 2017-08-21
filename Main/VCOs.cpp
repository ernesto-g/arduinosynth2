#include <Arduino.h>
#include "VCOs.h"
#include "Lfo.h"
#include "Config.h"

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
volatile unsigned short eg[EGS_LEN];
static volatile unsigned char egState[EGS_LEN];
static volatile unsigned short egDividers[EGS_LEN];
unsigned int egCounter[EGS_LEN]; 
volatile unsigned int egCounterInt; // affected by lfo interrupt
//______________

// Values set by front panel potenciometers
static volatile unsigned short eg1Attack;
static volatile unsigned short eg2Attack;
static volatile unsigned short eg1Release;
static volatile unsigned short eg2Release;
volatile unsigned char lfoValueForVCAInPanel; // 127 max
volatile unsigned short lfoEg2BalanceValueForVCFInPanel; // 127 max
//_________________________________________

static unsigned char waveformType=0;


void vcos_init(void)
{
  int i;
  for(i=0; i<VCOS_LEN;i++)
  {
    n[i]=0;
    freqMultiplier[i] = 0;
    freqMultiplierHalf[i] = 0;
  }
  for(i=0; i<EGS_LEN;i++)
  {
    eg[i] = EG1_MAX_VALUE;
    egState[i] = EG_STATE_IDLE;
  }  
  eg[EG2_INDEX] = 0;

  lfoValueForVCA = 0;
  lfoValueForVCAInPanel=0;
  lfoEg2BalanceValueForVCFInPanel = 0;
}

void vcos_calculateOuts(void)
{
    signed short acc;
    unsigned char i;
    //digitalWrite(13,HIGH);

    acc = 0;
    for(i=0; i<3;i++)
    {
      n[i]++;
      if(n[i]<freqMultiplierHalf[i])
      {
        acc-= eg[i] ;
      }
      else if(n[i]<freqMultiplier[i])
      {
        acc+= eg[i] ;        
      }
      else
      {
        n[i]=0;
      }           
    }
    OCR1B = (((acc*lfoValueForVCA)/128)*lfoValueForVCAInPanel)/128 + (((acc))*(128-lfoValueForVCAInPanel))/128 + (3*EG1_MAX_VALUE);

    acc = 0;
    for(i=3; i<6;i++)
    {
      n[i]++;
      if(n[i]<freqMultiplierHalf[i])
      {
        acc-= eg[i] ;
      }
      else if(n[i]<freqMultiplier[i])
      {
        acc+= eg[i];        
      }
      else
      {
        n[i]=0;
      }           
    }
    OCR1A = (((acc*lfoValueForVCA)/128)*lfoValueForVCAInPanel)/128 + (((acc))*(128-lfoValueForVCAInPanel))/128 + (3*EG1_MAX_VALUE);

    egCounterInt++;

    //digitalWrite(13,LOW);
}

void vcos_setFrqVCO(unsigned char vcoIndex,unsigned short val)
{
    freqMultiplier[vcoIndex] = val;
    if(waveformType==0)
      freqMultiplierHalf[vcoIndex] = val/2;
    else
      freqMultiplierHalf[vcoIndex] = val/5;
      
    egState[vcoIndex] = EG_STATE_START_ATTACK;
    egState[EG2_INDEX] = EG_STATE_START_ATTACK;
}

void vcos_turnOff(unsigned char vcoIndex)
{
    egState[vcoIndex] = EG_STATE_START_RELEASE;

    if(egState[EG2_INDEX]!=EG_STATE_IDLE && egState[EG2_INDEX]!=EG_STATE_RUNNING_RELEASE)
      egState[EG2_INDEX] = EG_STATE_START_RELEASE;
}

void vcos_setWaveForm(unsigned char val)
{
    waveformType = val;
}

static unsigned char indexStateMachine=0;

static unsigned char allEgsIdle(void)
{
  unsigned char i;
  for(i=0; i<(EGS_LEN-1); i++)
  {
    if(egState[i]!=EG_STATE_IDLE)
      return 0;
  }
  return 1;
}
void vcos_egStateMachine(void)
{
  unsigned char i = indexStateMachine;
  
  unsigned char timeout=24;  
  if(i==EG2_INDEX)
    timeout = 1;

  if(egCounterInt>=2)
  {
      egCounterInt=0;
      unsigned char j;
      for(j=0; j<EGS_LEN; j++)
        egCounter[j]++;
  }  
    
    switch(egState[i])
    {
      case EG_STATE_IDLE:
      {
        break;    
      }
      case EG_STATE_START_ATTACK:
      {
        if(i==EG2_INDEX)
          egDividers[i] = eg2Attack;
        else
          egDividers[i] = eg1Attack;
        
        egState[i] = EG_STATE_RUNNING_ATTACK;
        eg[i] = 0;
        egCounter[i]=0;
        break;
      }
      case EG_STATE_RUNNING_ATTACK:
      {
        if(egCounter[i]>timeout) // 1ms
        {
          egCounter[i]=0;
          if(egDividers[i]>0)
            egDividers[i]--;
          if(egDividers[i]==0)
          {
            eg[i]+=1;
            if(i==EG2_INDEX)
            {
              egDividers[i] = eg2Attack;
              if(eg[i]>=EG2_MAX_VALUE)
                egState[i] = EG_STATE_AT_MAX;              
            }
            else
            {
              egDividers[i] = eg1Attack;
              if(eg[i]>=EG1_MAX_VALUE)
                egState[i] = EG_STATE_AT_MAX;              
            } 
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
        if(i==EG2_INDEX)
            egDividers[i] = eg2Release;
        else
            egDividers[i] = eg1Release;

        egState[i] = EG_STATE_RUNNING_RELEASE;
        egCounter[i]=0;        
        break;
      }
      case EG_STATE_RUNNING_RELEASE:
      {
        if(egCounter[i]>timeout) // 1ms
        {
          egCounter[i]=0;
          if(egDividers[i]>0)
            egDividers[i]--;
          if(egDividers[i]==0)
          {
            if(i==EG2_INDEX)
            {
                egDividers[i] = eg2Release;    
            }
            else
            {
                egDividers[i] = eg1Release;
            }   
            eg[i]--;
            //printHex(eg[i]);
            if(eg[i]==0)
              egState[i] = EG_STATE_IDLE;
          }
        }        
        break;
      }
    }

  indexStateMachine++;
  if(indexStateMachine>=EGS_LEN)
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
  if(value==0)
    value=1;
  eg1Attack = value;
}
void vcos_setEg2Attack(byte value)
{
  if(value==0)
    value=1;
  eg2Attack = value;
}
void vcos_setEg1Release(byte value)
{
  if(value==0)
    value=1;
  eg1Release = value;
}
void vcos_setEg2Release(byte value)
{
  if(value==0)
    value=1;
  eg2Release = value;
}

void vcos_setLfoForVCAModulation(byte value)
{
  lfoValueForVCAInPanel = value;  
}

void vcos_setLfoForVCFModulation(byte value)
{
  lfoEg2BalanceValueForVCFInPanel =value;
}
