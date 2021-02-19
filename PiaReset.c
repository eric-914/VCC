#include <stdint.h>

#include "library/mc6821state.h"

void PiaReset()
{
  MC6821State* mc6821State = GetMC6821State();

  // Clear the PIA registers
  for (uint8_t index = 0; index < 4; index++)
  {
    mc6821State->rega[index] = 0;
    mc6821State->regb[index] = 0;
    mc6821State->rega_dd[index] = 0;
    mc6821State->regb_dd[index] = 0;
  }
}
