#include "vccstate.h"

#include "PakInterfaceAccessors.h"
#include "CopyRom.h"
#include "MmuReset.h"
#include "GimeReset.h"

#include "library/cpudef.h"

extern void PiaReset();
extern void mc6883_reset();

void SoftReset(void)
{
  VccState* vccState = GetVccState();

  mc6883_reset();
  PiaReset();

  GetCPU()->CPUReset();

  GimeReset();
  MmuReset();
  CopyRom();
  ResetBus();

  vccState->EmuState.TurboSpeedFlag = 1;
}
