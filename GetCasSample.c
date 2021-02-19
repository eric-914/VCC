#include "library/mc6821state.h"

unsigned char GetCasSample(void)
{
  MC6821State* mc6821State = GetMC6821State();

  return(mc6821State->Csample);
}
