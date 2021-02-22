#include "library/HD6309.h"

void HD6309DeAssertInterrupt(unsigned char interrupt) // 4 nmi 2 firq 1 irq
{
  HD6309State* hd63096State = GetHD6309State();

  hd63096State->PendingInterrupts &= ~(1 << (interrupt - 1));
  hd63096State->InInterrupt = 0;
}
