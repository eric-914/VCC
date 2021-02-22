#include "library/VCC.h"
#include "library/MC6821.h"
#include "library/cpudef.h"
#include "library/PAKInterface.h"
#include "library/MMU.h"
#include "library/Registers.h"

#include "CopyRom.h"
#include "GimeReset.h"

void SoftReset(void)
{
  VccState* vccState = GetVccState();

  MC6883Reset();
  PiaReset();

  GetCPU()->CPUReset();

  GimeReset();
  MmuReset();
  CopyRom();
  ResetBus();

  vccState->EmuState.TurboSpeedFlag = 1;
}
