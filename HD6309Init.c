#include "hd6309defs.h"
#include "hd6309state.h"
#include "hd6309intstate.h"

void HD6309Init(void)
{	//Call this first or RESET will core!
  // reg pointers for TFR and EXG and LEA ops
  HD6309State* hd63096State = GetHD6309State();
  HD6309IntState* hd6309IntState = GetHD6309IntState();

  hd63096State->xfreg16[0] = &D_REG;
  hd63096State->xfreg16[1] = &X_REG;
  hd63096State->xfreg16[2] = &Y_REG;
  hd63096State->xfreg16[3] = &U_REG;
  hd63096State->xfreg16[4] = &S_REG;
  hd63096State->xfreg16[5] = &PC_REG;
  hd63096State->xfreg16[6] = &W_REG;
  hd63096State->xfreg16[7] = &V_REG;

  hd63096State->ureg8[0] = (unsigned char*)&A_REG;
  hd63096State->ureg8[1] = (unsigned char*)&B_REG;
  hd63096State->ureg8[2] = (unsigned char*)&(hd63096State->ccbits);
  hd63096State->ureg8[3] = (unsigned char*)&DPA;
  hd63096State->ureg8[4] = (unsigned char*)&Z_H;
  hd63096State->ureg8[5] = (unsigned char*)&Z_L;
  hd63096State->ureg8[6] = (unsigned char*)&E_REG;
  hd63096State->ureg8[7] = (unsigned char*)&F_REG;

  //This handles the disparity between 6309 and 6809 Instruction timing
  hd6309IntState->InsCycles[0][M65] = 6;	//6-5
  hd6309IntState->InsCycles[1][M65] = 5;
  hd6309IntState->InsCycles[0][M64] = 6;	//6-4
  hd6309IntState->InsCycles[1][M64] = 4;
  hd6309IntState->InsCycles[0][M32] = 3;	//3-2
  hd6309IntState->InsCycles[1][M32] = 2;
  hd6309IntState->InsCycles[0][M21] = 2;	//2-1
  hd6309IntState->InsCycles[1][M21] = 1;
  hd6309IntState->InsCycles[0][M54] = 5;	//5-4
  hd6309IntState->InsCycles[1][M54] = 4;
  hd6309IntState->InsCycles[0][M97] = 9;	//9-7
  hd6309IntState->InsCycles[1][M97] = 7;
  hd6309IntState->InsCycles[0][M85] = 8;	//8-5
  hd6309IntState->InsCycles[1][M85] = 5;
  hd6309IntState->InsCycles[0][M51] = 5;	//5-1
  hd6309IntState->InsCycles[1][M51] = 1;
  hd6309IntState->InsCycles[0][M31] = 3;	//3-1
  hd6309IntState->InsCycles[1][M31] = 1;
  hd6309IntState->InsCycles[0][M1110] = 11;	//11-10
  hd6309IntState->InsCycles[1][M1110] = 10;
  hd6309IntState->InsCycles[0][M76] = 7;	//7-6
  hd6309IntState->InsCycles[1][M76] = 6;
  hd6309IntState->InsCycles[0][M75] = 7;	//7-5
  hd6309IntState->InsCycles[1][M75] = 5;
  hd6309IntState->InsCycles[0][M43] = 4;	//4-3
  hd6309IntState->InsCycles[1][M43] = 3;
  hd6309IntState->InsCycles[0][M87] = 8;	//8-7
  hd6309IntState->InsCycles[1][M87] = 7;
  hd6309IntState->InsCycles[0][M86] = 8;	//8-6
  hd6309IntState->InsCycles[1][M86] = 6;
  hd6309IntState->InsCycles[0][M98] = 9;	//9-8
  hd6309IntState->InsCycles[1][M98] = 8;
  hd6309IntState->InsCycles[0][M2726] = 27;	//27-26
  hd6309IntState->InsCycles[1][M2726] = 26;
  hd6309IntState->InsCycles[0][M3635] = 36;	//36-25
  hd6309IntState->InsCycles[1][M3635] = 35;
  hd6309IntState->InsCycles[0][M3029] = 30;	//30-29
  hd6309IntState->InsCycles[1][M3029] = 29;
  hd6309IntState->InsCycles[0][M2827] = 28;	//28-27
  hd6309IntState->InsCycles[1][M2827] = 27;
  hd6309IntState->InsCycles[0][M3726] = 37;	//37-26
  hd6309IntState->InsCycles[1][M3726] = 26;
  hd6309IntState->InsCycles[0][M3130] = 31;	//31-30
  hd6309IntState->InsCycles[1][M3130] = 30;
  hd6309IntState->InsCycles[0][M42] = 4;	//4-2
  hd6309IntState->InsCycles[1][M42] = 2;
  hd6309IntState->InsCycles[0][M53] = 5;	//5-3
  hd6309IntState->InsCycles[1][M53] = 3;
}
