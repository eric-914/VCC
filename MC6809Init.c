#include "library/mc6809state.h"

void MC6809Init(void)
{	//Call this first or RESET will core!
  // reg pointers for TFR and EXG and LEA ops
  MC6809State* mc6809State = GetMC6809State();

  mc6809State->xfreg16[0] = &D_REG;
  mc6809State->xfreg16[1] = &X_REG;
  mc6809State->xfreg16[2] = &Y_REG;
  mc6809State->xfreg16[3] = &U_REG;
  mc6809State->xfreg16[4] = &S_REG;
  mc6809State->xfreg16[5] = &PC_REG;

  mc6809State->ureg8[0] = (unsigned char*)(&A_REG);
  mc6809State->ureg8[1] = (unsigned char*)(&B_REG);
  mc6809State->ureg8[2] = (unsigned char*)(&(mc6809State->ccbits));
  mc6809State->ureg8[3] = (unsigned char*)(&DPA);
  mc6809State->ureg8[4] = (unsigned char*)(&DPA);
  mc6809State->ureg8[5] = (unsigned char*)(&DPA);
  mc6809State->ureg8[6] = (unsigned char*)(&DPA);
  mc6809State->ureg8[7] = (unsigned char*)(&DPA);
}
