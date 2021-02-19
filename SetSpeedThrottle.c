#include "library/vccstate.h"

unsigned char SetSpeedThrottle(unsigned char throttle)
{
  VccState* vccState = GetVccState();

  if (throttle != QUERY) {
    vccState->Throttle = throttle;
  }

  return(vccState->Throttle);
}
