

#define VCOS_LEN  6

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


