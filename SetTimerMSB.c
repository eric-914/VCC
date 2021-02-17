#include "registersstate.h"

#include "SetInterruptTimer.h"

void SetTimerMSB(unsigned char data) //94
{
  unsigned short temp;

  RegistersState* registersState = GetRegistersState();

  temp = ((registersState->GimeRegisters[0x94] << 8) + registersState->GimeRegisters[0x95]) & 4095;

  SetInterruptTimer(temp);
}
