#include "mc6821state.h"

#include "library/cpudef.h"
#include "library/defines.h"

void AssertCart(void)
{
  MC6821State* mc6821State = GetMC6821State();
  CPU* cpu = GetCPU();

  mc6821State->regb[3] = (mc6821State->regb[3] | 128);

  if (mc6821State->regb[3] & 1) {
    cpu->CPUAssertInterrupt(FIRQ, 0);
  }
  else {
    cpu->CPUDeAssertInterrupt(FIRQ); //Kludge but working
  }
}
