#include "library/mc6821state.h"

void SetSerialParams(unsigned char textMode)
{
  MC6821State* mc6821State = GetMC6821State();

  mc6821State->AddLF = textMode;
}
