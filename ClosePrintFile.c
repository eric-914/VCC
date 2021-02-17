#include "mc6821state.h"

void ClosePrintFile(void)
{
  MC6821State* mc6821State = GetMC6821State();

  CloseHandle(mc6821State->hPrintFile);

  mc6821State->hPrintFile = INVALID_HANDLE_VALUE;

  FreeConsole();

  mc6821State->hOut = NULL;
}
