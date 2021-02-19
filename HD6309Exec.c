#include "hd6309state.h"
#include "hd6309intstate.h"
#include "hd6309_i.h"
#include "hd6309vector.h"

#include "MemRead8.h"

int HD6309Exec(int cycleFor)
{
  HD6309State* hd63096State = GetHD6309State();
  HD6309IntState* hd63096IntState = GetHD6309IntState();

  hd63096State->CycleCounter = 0;
  hd63096State->gCycleFor = cycleFor;

  while (hd63096State->CycleCounter < cycleFor) {

    if (hd63096IntState->PendingInterrupts)
    {
      if (hd63096IntState->PendingInterrupts & 4) {
        HD6309_cpu_nmi();
      }

      if (hd63096IntState->PendingInterrupts & 2) {
        HD6309_cpu_firq();
      }

      if (hd63096IntState->PendingInterrupts & 1)
      {
        if (hd63096IntState->IRQWaiter == 0) { // This is needed to fix a subtle timming problem
          HD6309_cpu_irq();		// It allows the CPU to see $FF03 bit 7 high before
        }
        else {				// The IRQ is asserted.
          hd63096IntState->IRQWaiter -= 1;
        }
      }
    }

    if (hd63096State->SyncWaiting == 1) { //Abort the run nothing happens asyncronously from the CPU
      return(0); // WDZ - Experimental SyncWaiting should still return used cycles (and not zero) by breaking from loop
    }

    JmpVec1[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
  }

  return(cycleFor - hd63096State->CycleCounter);
}

void Page_2(void) //10
{
  HD6309State* hd63096State = GetHD6309State();

  JmpVec2[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
}

void Page_3(void) //11
{
  HD6309State* hd63096State = GetHD6309State();

  JmpVec3[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
}
