#include "vccstate.h"

#include "PakInterfaceAccessors.h"
#include "SetClockSpeed.h"
#include "MmuInit.h"

#include "library/cpudef.h"

extern void HD6309Init(void);
extern int  HD6309Exec(int);
extern void HD6309Reset(void);
extern void HD6309AssertInterrupt(unsigned char, unsigned char);
extern void HD6309DeAssertInterrupt(unsigned char);// 4 nmi 2 firq 1 irq
extern void HD6309ForcePC(unsigned short);

extern void MC6809Init(void);
extern int  MC6809Exec(int);
extern void MC6809Reset(void);
extern void MC6809AssertInterrupt(unsigned char, unsigned char);
extern void MC6809DeAssertInterrupt(unsigned char);// 4 nmi 2 firq 1 irq
extern void MC6809ForcePC(unsigned short);

extern void PiaReset();
extern void MC6883Reset();
extern void GimeReset(void);
extern void UpdateBusPointer(void);

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
