#include "hd6309state.h"
#include "hd6309intstate.h"

void HD6309ForcePC(unsigned short address)
{
  HD6309State* hd63096State = GetHD6309State();
  HD6309IntState* hd6309IntState = GetHD6309IntState();

  PC_REG = address;

  hd6309IntState->PendingInterrupts = 0;
  hd63096State->SyncWaiting = 0;
}
