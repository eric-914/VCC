#include "library/MC6809.h"

void MC6809DeAssertInterrupt(unsigned char interrupt) // 4 nmi 2 firq 1 irq
{
  MC6809State* mc6809State = GetMC6809State();

  mc6809State->PendingInterrupts &= ~(1 << (interrupt - 1));
  mc6809State->InInterrupt = 0;
}
