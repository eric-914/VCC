#include "library/mc6809state.h"

void MC6809AssertInterrupt(unsigned char interrupt, unsigned char waiter) // 4 nmi 2 firq 1 irq
{
  MC6809State* mc6809State = GetMC6809State();

  mc6809State->SyncWaiting = 0;
  mc6809State->PendingInterrupts |= (1 << (interrupt - 1));
  mc6809State->IRQWaiter = waiter;
}
