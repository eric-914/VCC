#include "library/hd6309defs.h"
#include "library/hd6309state.h"
#include "library/hd6309intstate.h"

#include "hd6309_cc.h"

#include "MemWrite8.h"
#include "MemRead16.h"

void HD6309_cpu_firq(void)
{
  HD6309State* hd63096State = GetHD6309State();
  HD6309IntState* hd63096IntState = GetHD6309IntState();

  if (!CC_F)
  {
    hd63096State->InInterrupt = 1; //Flag to indicate FIRQ has been asserted

    switch (MD_FIRQMODE)
    {
    case 0:
      CC_E = 0; // Turn E flag off

      MemWrite8(PC_L, --S_REG);
      MemWrite8(PC_H, --S_REG);
      MemWrite8(getcc(), --S_REG);

      CC_I = 1;
      CC_F = 1;
      PC_REG = MemRead16(VFIRQ);

      break;

    case 1:		//6309
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
      }

      MemWrite8(B_REG, --S_REG);
      MemWrite8(A_REG, --S_REG);
      MemWrite8(getcc(), --S_REG);

      CC_I = 1;
      CC_F = 1;
      PC_REG = MemRead16(VFIRQ);

      break;
    }
  }

  hd63096IntState->PendingInterrupts &= 253;
}

void HD6309_cpu_irq(void)
{
  HD6309State* hd63096State = GetHD6309State();
  HD6309IntState* hd63096IntState = GetHD6309IntState();

  if (hd63096State->InInterrupt == 1) { //If FIRQ is running postpone the IRQ
    return;
  }

  if (!CC_I) {
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
    }

    MemWrite8(B_REG, --S_REG);
    MemWrite8(A_REG, --S_REG);
    MemWrite8(getcc(), --S_REG);

    PC_REG = MemRead16(VIRQ);
    CC_I = 1;
  }

  hd63096IntState->PendingInterrupts &= 254;
}

void HD6309_cpu_nmi(void)
{
  HD6309State* hd63096State = GetHD6309State();
  HD6309IntState* hd63096IntState = GetHD6309IntState();

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
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(getcc(), --S_REG);

  CC_I = 1;
  CC_F = 1;
  PC_REG = MemRead16(VNMI);

  hd63096IntState->PendingInterrupts &= 251;
}
