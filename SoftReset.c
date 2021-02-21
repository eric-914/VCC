#include "library/vccstate.h"
#include "library/MC6821.h"
#include "library/cpudef.h"
#include "library/PakInterfaceAccessors.h"

#include "CopyRom.h"
#include "MmuReset.h"
#include "GimeReset.h"
#include "MC6883Reset.h"

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
