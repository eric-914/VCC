#include "library/MC6809.h"
#include "library/MMU.h"

void MC6809Reset(void)
{
  char index;

  MC6809State* mc6809State = GetMC6809State();

  for (index = 0; index <= 5; index++) {		//Set all register to 0 except V
    PXF(index) = 0;
  }

  for (index = 0; index <= 7; index++) {
    PUR(index) = 0;
  }

  CC_E = 0;
  CC_F = 1;
  CC_H = 0;
  CC_I = 1;
  CC_N = 0;
  CC_Z = 0;
  CC_V = 0;
  CC_C = 0;

  DP_REG = 0;

  mc6809State->SyncWaiting = 0;

  PC_REG = MemRead16(VRESET);	//PC gets its reset vector

  SetMapType(0);
}
