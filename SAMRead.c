#include "library/Registers.h"

unsigned char SAMRead(unsigned char port) //SAM don't talk much :)
{
  RegistersState* registersState = GetRegistersState();

  if ((port >= 0xF0) && (port <= 0xFF)) { //IRQ vectors from rom
    return(registersState->Rom[0x3F00 + port]);
  }

  return(0);
}
