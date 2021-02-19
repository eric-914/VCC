#include "library/mc6821state.h"

void SetCassetteSample(unsigned char sample)
{
  MC6821State* mc6821State = GetMC6821State();

  mc6821State->regb[0] = mc6821State->regb[0] & 0xFE;

  if (sample > 0x7F) {
    mc6821State->regb[0] = mc6821State->regb[0] | 1;
  }
}
