#include "library/mc6821state.h"

unsigned char GetMuxState(void)
{
  MC6821State* mc6821State = GetMC6821State();

  return (((mc6821State->rega[1] & 8) >> 3) + ((mc6821State->rega[3] & 8) >> 2));
}
