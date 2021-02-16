#include "vccstate.h"

unsigned char SetRamSize(unsigned char size)
{
  VccState* vccState = GetVccState();

  if (size != QUERY) {
    vccState->EmuState.RamSize = size;
  }

  return(vccState->EmuState.RamSize);
}
