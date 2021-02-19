#include "library/hd6309defs.h"
#include "library/hd6309state.h"
#include "hd6309_md.h"

#include "MemRead16.h"
#include "library/MmuAccessors.h"

void HD6309Reset(void)
{
  HD6309State* hd63096State = GetHD6309State();

  for (char index = 0; index <= 6; index++) {		//Set all register to 0 except V
    PXF(index) = 0;
  }

  for (char index = 0; index <= 7; index++) {
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

  MD_NATIVE6309 = 0;
  MD_FIRQMODE = 0;
  MD_UNDEFINED2 = 0;  //UNDEFINED
  MD_UNDEFINED3 = 0;  //UNDEFINED
  MD_UNDEFINED4 = 0;  //UNDEFINED
  MD_UNDEFINED5 = 0;  //UNDEFINED
  MD_ILLEGAL = 0;
  MD_ZERODIV = 0;

  hd63096State->mdbits = getmd();

  hd63096State->SyncWaiting = 0;

  DP_REG = 0;
  PC_REG = MemRead16(VRESET);	//PC gets its reset vector

  SetMapType(0);	//shouldn't be here
}
