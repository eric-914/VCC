#include "library/registersstate.h"

#include "library/CoCo.h"

void SetTimerLSB(unsigned char data) //95
{
  unsigned short temp;

  RegistersState* registersState = GetRegistersState();

  temp = ((registersState->GimeRegisters[0x94] << 8) + registersState->GimeRegisters[0x95]) & 4095;

  SetInterruptTimer(temp);
}
