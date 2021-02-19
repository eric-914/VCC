#include "library/vccstate.h"

#include "PakInterfaceAccessors.h"
#include "SetClockSpeed.h"
#include "MmuInit.h"

#include "HD6309Init.h"
#include "HD6309Exec.h"
#include "HD6309Reset.h"
#include "HD6309AssertInterrupt.h"
#include "HD6309DeAssertInterrupt.h"
#include "HD6309ForcePC.h"

#include "MC6809Init.h"
#include "MC6809Exec.h"
#include "MC6809Reset.h"
#include "MC6809AssertInterrupt.h"
#include "MC6809DeAssertInterrupt.h"
#include "MC6809ForcePC.h"

#include "MC6883Reset.h"

#include "PiaReset.h"
#include "GimeReset.h"
#include "UpdateBusPointer.h"

#include "library/cpudef.h"

void HardReset(SystemState* const systemState)
{
  VccState* vccState = GetVccState();

  systemState->RamBuffer = MmuInit(systemState->RamSize);	//Allocate RAM/ROM & copy ROM Images from source
  systemState->WRamBuffer = (unsigned short*)systemState->RamBuffer;

  if (systemState->RamBuffer == NULL)
  {
    MessageBox(NULL, "Can't allocate enough RAM, Out of memory", "Error", 0);

    exit(0);
  }

  CPU* cpu = GetCPU();

  if (systemState->CpuType == 1)
  {
    cpu->CPUInit = HD6309Init;
    cpu->CPUExec = HD6309Exec;
    cpu->CPUReset = HD6309Reset;
    cpu->CPUAssertInterrupt = HD6309AssertInterrupt;
    cpu->CPUDeAssertInterrupt = HD6309DeAssertInterrupt;
    cpu->CPUForcePC = HD6309ForcePC;
  }
  else
  {
    cpu->CPUInit = MC6809Init;
    cpu->CPUExec = MC6809Exec;
    cpu->CPUReset = MC6809Reset;
    cpu->CPUAssertInterrupt = MC6809AssertInterrupt;
    cpu->CPUDeAssertInterrupt = MC6809DeAssertInterrupt;
    cpu->CPUForcePC = MC6809ForcePC;
  }

  PiaReset();
  MC6883Reset();	//Captures interal rom pointer for CPU Interrupt Vectors

  cpu->CPUInit();
  cpu->CPUReset();		// Zero all CPU Registers and sets the PC to VRESET

  GimeReset();
  UpdateBusPointer();

  vccState->EmuState.TurboSpeedFlag = 1;

  ResetBus();
  SetClockSpeed(1);
}
