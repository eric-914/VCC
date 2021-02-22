#include "library/MMU.h"
#include "library/HD6309.h"

#include "hd6309opcodes.h"

int HD6309Exec(int cycleFor)
{
  HD6309State* hd63096State = GetHD6309State();

  hd63096State->CycleCounter = 0;
  hd63096State->gCycleFor = cycleFor;

  while (hd63096State->CycleCounter < cycleFor) {

    if (hd63096State->PendingInterrupts)
    {
      if (hd63096State->PendingInterrupts & 4) {
        HD6309_cpu_nmi();
      }

      if (hd63096State->PendingInterrupts & 2) {
        HD6309_cpu_firq();
      }

      if (hd63096State->PendingInterrupts & 1)
      {
        if (hd63096State->IRQWaiter == 0) { // This is needed to fix a subtle timming problem
          HD6309_cpu_irq();		// It allows the CPU to see $FF03 bit 7 high before
        }
        else {				// The IRQ is asserted.
          hd63096State->IRQWaiter -= 1;
        }
      }
    }

    if (hd63096State->SyncWaiting == 1) { //Abort the run nothing happens asyncronously from the CPU
      return(0); // WDZ - Experimental SyncWaiting should still return used cycles (and not zero) by breaking from loop
    }

    Page_1();
  }

  return(cycleFor - hd63096State->CycleCounter);
}
