#include "mc6821state.h"

unsigned char VDG_Mode(void)
{
  MC6821State* mc6821State = GetMC6821State();

  return((mc6821State->regb[2] & 248) >> 3);
}
