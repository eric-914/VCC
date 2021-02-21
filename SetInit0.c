#include "library/registersstate.h"

#include "library/Graphics.h"
#include "library/MMU.h"

void SetInit0(unsigned char data)
{
  RegistersState* registersState = GetRegistersState();

  SetCompatMode(!!(data & 128));
  SetMmuEnabled(!!(data & 64)); //MMUEN
  SetRomMap(data & 3);			//MC0-MC1
  SetVectors(data & 8);			//MC3

  registersState->EnhancedFIRQFlag = (data & 16) >> 4;
  registersState->EnhancedIRQFlag = (data & 32) >> 5;
}
