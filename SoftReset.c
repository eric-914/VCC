#include "vccstate.h"

#include "PakInterfaceAccessors.h"
#include "CopyRom.h"
#include "MmuReset.h"
#include "GimeReset.h"
#include "PiaReset.h"
#include "MC6883Reset.h"

#include "library/cpudef.h"

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
