#include "vccstate.h"

#include "PakInterfaceAccessors.h"
#include "CopyRom.h"
#include "MmuReset.h"
#include "GimeReset.h"

#include "library/cpudef.h"

extern void PiaReset();
extern void MC6883Reset();

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
