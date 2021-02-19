#include "library/mc6821state.h"

int OpenPrintFile(char* filename)
{
  MC6821State* mc6821State = GetMC6821State();

  mc6821State->hPrintFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

  if (mc6821State->hPrintFile == INVALID_HANDLE_VALUE) {
    return(0);
  }

  return(1);
}
