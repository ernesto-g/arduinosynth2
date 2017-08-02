#include <Arduino.h>

static volatile unsigned short n[6];
static volatile unsigned short freqMultiplier[6];
static volatile unsigned short freqMultiplierHalf[6];
static volatile unsigned short eg[6]; // 255 max
static volatile unsigned char lfoValueForVCA; // 127 max

inline static signed short fnSawTooth(unsigned short n)
{
    //return (n / 256) - 78;
    return (n / 512) - 39;
}

inline static signed short fnSquare(unsigned short n)
{
    if(n<20000)
      return 41;
    return -41;
}

inline static signed short fnTriangle(unsigned short n)
{
    if(n<20000)
    {
        return (n / 512) - 39;
    }    
    return 0 - ( (n-20000) / 512);
}

void vcos_init(void)
{
  int i;
  for(i=0; i<6;i++)
  {
    n[i]=0;
    freqMultiplier[i] = 10;
    freqMultiplierHalf[i] = 5;
    eg[i] = 32;
  }
  lfoValueForVCA = 127;
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
        acc-= eg[i]/8 ;
      }
      else if(n[i]<freqMultiplier[i])
      {
        //acc+= ((32*eg[i])/256) ;        
        acc+= eg[i]/8 ;        
      }
      else
      {
        n[i]=0;
      }           
    }
    OCR1B = ((acc*lfoValueForVCA)/128) + 96;

    acc = 0;
    for(i=3; i<6;i++)
    {
      n[i]++;
      if(n[i]<freqMultiplierHalf[i])
      {
        acc-= ((32*eg[i])/256) ;
      }
      else if(n[i]<freqMultiplier[i])
      {
        acc+= ((32*eg[i])/256) ;        
      }
      else
      {
        n[i]=0;
      }           
    }
    OCR1A = ((acc*lfoValueForVCA)/128) + 96;


    
    digitalWrite(13,LOW);
}

void vocs_setFrqVCO1(unsigned short val)
{
    freqMultiplier[0] = val;
    freqMultiplier[1] = val;
    freqMultiplier[2] = 0;
}
void vocs_setFrqVCO2(unsigned short val)
{
    freqMultiplier[3] = val;
    freqMultiplier[4] = val;
    freqMultiplier[5] = 0;
}


