#include "library/mc6821state.h"

#include "AssertCart.h"

#include "library/cpudef.h"
#include "library/defines.h"

void irq_fs(int phase)	//60HZ Vertical sync pulse 16.667 mS
{
  MC6821State* mc6821State = GetMC6821State();

  if ((mc6821State->CartInserted == 1) && (mc6821State->CartAutoStart == 1)) {
    AssertCart();
  }

  CPU* cpu = GetCPU();

  switch (phase)
  {
  case 0:	//FS went High to low
    if ((mc6821State->rega[3] & 2) == 0) { //IRQ on High to low transition
      mc6821State->rega[3] = (mc6821State->rega[3] | 128);
    }

    if (mc6821State->rega[3] & 1) {
      cpu->CPUAssertInterrupt(IRQ, 1);
    }

    return;

  case 1:	//FS went Low to High

    if ((mc6821State->rega[3] & 2)) //IRQ  Low to High transition
    {
      mc6821State->rega[3] = (mc6821State->rega[3] | 128);

      if (mc6821State->rega[3] & 1) {
        cpu->CPUAssertInterrupt(IRQ, 1);
      }
    }

    return;
  }
}
