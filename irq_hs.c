#include "mc6821state.h"
#include "mc6821def.h"

#include "library/cpudef.h"
#include "library/defines.h"

void irq_hs(int phase)	//63.5 uS
{
  MC6821State* mc6821State = GetMC6821State();
  CPU* cpu = GetCPU();

  switch (phase)
  {
  case FALLING:	//HS went High to low
    if ((mc6821State->rega[1] & 2)) { //IRQ on low to High transition
      return;
    }

    mc6821State->rega[1] = (mc6821State->rega[1] | 128);

    if (mc6821State->rega[1] & 1) {
      cpu->CPUAssertInterrupt(IRQ, 1);
    }

    break;

  case RISING:	//HS went Low to High
    if (!(mc6821State->rega[1] & 2)) { //IRQ  High to low transition
      return;
    }

    mc6821State->rega[1] = (mc6821State->rega[1] | 128);

    if (mc6821State->rega[1] & 1) {
      cpu->CPUAssertInterrupt(IRQ, 1);
    }

    break;

  case ANY:
    mc6821State->rega[1] = (mc6821State->rega[1] | 128);

    if (mc6821State->rega[1] & 1) {
      cpu->CPUAssertInterrupt(IRQ, 1);
    }

    break;
  }
}
