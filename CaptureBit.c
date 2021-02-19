#include "mc6821state.h"
#include "WritePrintMon.h"

void CaptureBit(unsigned char sample)
{
  unsigned long bytesMoved = 0;
  static unsigned char bitMask = 1, startWait = 1;
  static char Byte = 0;

  MC6821State* mc6821State = GetMC6821State();

  if (mc6821State->hPrintFile == INVALID_HANDLE_VALUE) {
    return;
  }

  if (startWait & sample) { //Waiting for start bit
    return;
  }

  if (startWait)
  {
    startWait = 0;

    return;
  }

  if (sample) {
    Byte |= bitMask;
  }

  bitMask = bitMask << 1;

  if (!bitMask)
  {
    bitMask = 1;
    startWait = 1;

    WriteFile(mc6821State->hPrintFile, &Byte, 1, &bytesMoved, NULL);

    if (mc6821State->MonState) {
      WritePrintMon(&Byte);
    }

    if ((Byte == 0x0D) & mc6821State->AddLF)
    {
      Byte = 0x0A;

      WriteFile(mc6821State->hPrintFile, &Byte, 1, &bytesMoved, NULL);
    }

    Byte = 0;
  }
}
