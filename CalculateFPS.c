#include "library/throttlestate.h"

#include "library/defines.h"

float CalculateFPS(void) //Done at end of render;
{
  static unsigned short frameCount = 0;
  static float fps = 0, fNow = 0, fLast = 0;

  ThrottleState* throttleState = GetThrottleState();

  if (++frameCount != FRAMEINTERVAL) {
    return(fps);
  }

  QueryPerformanceCounter(&(throttleState->Now));

  fNow = (float)(throttleState->Now.QuadPart);
  fps = (fNow - fLast) / throttleState->fMasterClock;
  fLast = fNow;
  frameCount = 0;
  fps = FRAMEINTERVAL / fps;

  return(fps);
}
