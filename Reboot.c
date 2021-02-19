#include "library/vccstate.h"

void Reboot(void)
{
  GetVccState()->EmuState.ResetPending = 2;
}
