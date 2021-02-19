#include "library/mc6821state.h"

void WritePrintMon(char* data)
{
  unsigned long dummy;

  MC6821State* mc6821State = GetMC6821State();

  if (mc6821State->hOut == NULL)
  {
    AllocConsole();

    mc6821State->hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTitle("Printer Monitor");
  }

  WriteConsole(mc6821State->hOut, data, 1, &dummy, 0);

  if (data[0] == 0x0D)
  {
    data[0] = 0x0A;

    WriteConsole(mc6821State->hOut, data, 1, &dummy, 0);
  }
}
