#include "library/throttlestate.h"

void EndRender(unsigned char skip)
{
  ThrottleState* throttleState = GetThrottleState();

  throttleState->FrameSkip = skip;
  throttleState->TargetTime.QuadPart = (throttleState->StartTime.QuadPart + (throttleState->OneFrame.QuadPart * throttleState->FrameSkip));
}
