#include "vccstate.h"

#include "PakInterfaceAccessors.h"

#include "library/cpudef.h"

extern void PiaReset();
extern void mc6883_reset();
extern void GimeReset(void);
extern void MmuReset(void);
extern void CopyRom(void);

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
