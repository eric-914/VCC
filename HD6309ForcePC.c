#include "library/HD6309.h"

#include "library/HD6309Macros.h"

void HD6309ForcePC(unsigned short address)
{
  HD6309State* hd63096State = GetHD6309State();
  
  PC_REG = address;

  hd63096State->PendingInterrupts = 0;
  hd63096State->SyncWaiting = 0;
}
