#include "library/vccstate.h"

unsigned char SetFrameSkip(unsigned char skip)
{
  VccState* vccState = GetVccState();

  if (skip != QUERY) {
    vccState->EmuState.FrameSkip = skip;
  }

  return(vccState->EmuState.FrameSkip);
}
