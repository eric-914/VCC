#include "mc6809state.h"

void MC6809ForcePC(unsigned short NewPC)
{
  MC6809State* mc6809State = GetMC6809State();

  PC_REG = NewPC;

  mc6809State->PendingInterrupts = 0;
  mc6809State->SyncWaiting = 0;
}
