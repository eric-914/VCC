#include "throttlestate.h"

void StartRender(void)
{
  ThrottleState* throttleState = GetThrottleState();

  QueryPerformanceCounter(&(throttleState->StartTime));
}
