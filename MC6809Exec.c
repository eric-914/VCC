#include "library/MC6809.h"
#include "library/MMU.h"

#include "library/MC6809Macros.h"
#include "mc6809opcodes.h"

int MC6809Exec(int cycleFor)
{
  MC6809State* mc6809State = GetMC6809State();

  mc6809State->CycleCounter = 0;

  while (mc6809State->CycleCounter < cycleFor) {

    if (mc6809State->PendingInterrupts)
    {
      if (mc6809State->PendingInterrupts & 4) {
        MC609_cpu_nmi();
      }

      if (mc6809State->PendingInterrupts & 2) {
        MC609_cpu_firq();
      }

      if (mc6809State->PendingInterrupts & 1)
      {
        if (mc6809State->IRQWaiter == 0) { // This is needed to fix a subtle timming problem
          MC609_cpu_irq();		// It allows the CPU to see $FF03 bit 7 high before
        }
        else {				// The IRQ is asserted.
          mc6809State->IRQWaiter -= 1;
        }
      }
    }

    if (mc6809State->SyncWaiting == 1) {
      return(0);
    }

    unsigned char opCode = MemRead8(PC_REG++);

    MC6809ExecOpCode(cycleFor, opCode);
  }

  return(cycleFor - mc6809State->CycleCounter);
}
