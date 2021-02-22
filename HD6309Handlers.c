#include "library/hd6309defs.h"
#include "library/hd6309state.h"
#include "library/MMU.h"

#include "hd6309_cc.h"
#include "hd6309_md.h"

void ErrorVector(void);

void InvalidInsHandler(void)
{
  HD6309State* hd63096State = GetHD6309State();

  MD_ILLEGAL = 1;
  hd63096State->mdbits = getmd();

  ErrorVector();
}

void DivbyZero(void)
{
  HD6309State* hd63096State = GetHD6309State();

  MD_ZERODIV = 1;
  hd63096State->mdbits = getmd();

  ErrorVector();
}

void ErrorVector(void)
{
  HD6309State* hd63096State = GetHD6309State();

  CC_E = 1;

  MemWrite8(PC_L, --S_REG);
  MemWrite8(PC_H, --S_REG);
  MemWrite8(U_L, --S_REG);
  MemWrite8(U_H, --S_REG);
  MemWrite8(Y_L, --S_REG);
  MemWrite8(Y_H, --S_REG);
  MemWrite8(X_L, --S_REG);
  MemWrite8(X_H, --S_REG);
  MemWrite8(DPA, --S_REG);

  if (MD_NATIVE6309)
  {
    MemWrite8((F_REG), --S_REG);
    MemWrite8((E_REG), --S_REG);

    hd63096State->CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(getcc(), --S_REG);

  PC_REG = MemRead16(VTRAP);

  hd63096State->CycleCounter += (12 + hd63096State->NatEmuCycles54);	//One for each byte +overhead? Guessing from PSHS
}
