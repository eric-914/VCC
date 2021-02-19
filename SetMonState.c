#include "library/mc6821state.h"

void SetMonState(BOOL state)
{
  MC6821State* mc6821State = GetMC6821State();

  if (mc6821State->MonState & !state)
  {
    FreeConsole();

    mc6821State->hOut = NULL;
  }

  mc6821State->MonState = state;
}
