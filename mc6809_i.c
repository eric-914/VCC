#include "library/mc6809defs.h"
#include "library/mc6809state.h"
#include "mc6809_cc.h"

#include "MemWrite8.h"
#include "MemRead16.h"

void MC609_cpu_firq(void)
{
  MC6809State* mc6809State = GetMC6809State();

  if (!CC_F)
  {
    mc6809State->InInterrupt = 1; //Flag to indicate FIRQ has been asserted
    CC_E = 0; // Turn E flag off

    MemWrite8(PC_L, --S_REG);
    MemWrite8(PC_H, --S_REG);
    MemWrite8(mc6809_getcc(), --S_REG);

    CC_I = 1;
    CC_F = 1;
    PC_REG = MemRead16(VFIRQ);
  }

  mc6809State->PendingInterrupts &= 253;
}

void MC609_cpu_irq(void)
{
  MC6809State* mc6809State = GetMC6809State();

  if (mc6809State->InInterrupt == 1) { //If FIRQ is running postpone the IRQ
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
    MemWrite8(B_REG, --S_REG);
    MemWrite8(A_REG, --S_REG);
    MemWrite8(mc6809_getcc(), --S_REG);
    PC_REG = MemRead16(VIRQ);
    CC_I = 1;
  }

  mc6809State->PendingInterrupts &= 254;
}

void MC609_cpu_nmi(void)
{
  MC6809State* mc6809State = GetMC6809State();

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
  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(mc6809_getcc(), --S_REG);

  CC_I = 1;
  CC_F = 1;
  PC_REG = MemRead16(VNMI);

  mc6809State->PendingInterrupts &= 251;
}
