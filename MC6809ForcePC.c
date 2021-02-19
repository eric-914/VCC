#include "library/mc6809state.h"

void MC6809ForcePC(unsigned short newPC)
{
  MC6809State* mc6809State = GetMC6809State();

  PC_REG = newPC;

  mc6809State->PendingInterrupts = 0;
  mc6809State->SyncWaiting = 0;
}
