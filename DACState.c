#include "library/mc6821state.h"

unsigned char DACState(void)
{
  MC6821State* mc6821State = GetMC6821State();

  return (mc6821State->regb[0] >> 2);
}
