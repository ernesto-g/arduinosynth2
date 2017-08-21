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
void vcos_setEGmode(byte mode);

