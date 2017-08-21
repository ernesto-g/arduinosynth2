

#define VCOS_LEN  6
#define EG1_MAX_VALUE  16
#define EG2_MAX_VALUE 255
#define EGS_LEN       VCOS_LEN+1
#define EG2_INDEX     EGS_LEN-1 // last eg in array is EG2


void vcos_init(void);
void vcos_calculateOuts(void);

void vcos_setFrqVCO(unsigned char vcoIndex,unsigned short val);
void vcos_turnOff(unsigned char vcoIndex);
void vcos_egStateMachine(void);

void vcos_setEg1Attack(byte value);
void vcos_setEg2Attack(byte value);
void vcos_setEg1Release(byte value);
void vcos_setEg2Release(byte value);
void vcos_setLfoForVCAModulation(byte value);
void vcos_setLfoForVCFModulation(byte value);
void vcos_setWaveForm(unsigned char val);
