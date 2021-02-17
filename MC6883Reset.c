#include "registersstate.h"

#include "MmuAccessors.h"

void MC6883Reset()
{
  RegistersState* registersState = GetRegistersState();

  registersState->VDG_Mode = 0;
  registersState->Dis_Offset = 0;
  registersState->MPU_Rate = 0;

  registersState->Rom = GetInternalRomPointer();
}
