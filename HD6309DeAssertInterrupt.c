#include "library/hd6309state.h"
#include "library/hd6309intstate.h"

void HD6309DeAssertInterrupt(unsigned char interrupt) // 4 nmi 2 firq 1 irq
{
  HD6309State* hd63096State = GetHD6309State();
  HD6309IntState* hd6309IntState = GetHD6309IntState();

  hd6309IntState->PendingInterrupts &= ~(1 << (interrupt - 1));
  hd63096State->InInterrupt = 0;
}
