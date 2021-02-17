#include "registersstate.h"

#include "KeyboardAccessors.h"
#include "SetVertInterruptState.h"
#include "SetHorzInterruptState.h"
#include "SetTimerInterruptState.h"

void SetGimeFIRQSteering(unsigned char data) //93
{
  RegistersState* registersState = GetRegistersState();

  if ((registersState->GimeRegisters[0x92] & 2) | (registersState->GimeRegisters[0x93] & 2)) {
    GimeSetKeyboardInterruptState(1);
  }
  else {
    GimeSetKeyboardInterruptState(0);
  }

  if ((registersState->GimeRegisters[0x92] & 8) | (registersState->GimeRegisters[0x93] & 8)) {
    SetVertInterruptState(1);
  }
  else {
    SetVertInterruptState(0);
  }

  if ((registersState->GimeRegisters[0x92] & 16) | (registersState->GimeRegisters[0x93] & 16)) {
    SetHorzInterruptState(1);
  }
  else {
    SetHorzInterruptState(0);
  }

  // Moon Patrol Demo Using Timer for FIRQ Side Scroll 
  if ((registersState->GimeRegisters[0x92] & 32) | (registersState->GimeRegisters[0x93] & 32)) {
    SetTimerInterruptState(1);
  }
  else {
    SetTimerInterruptState(0);
  }
}
