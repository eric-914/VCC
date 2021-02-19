#include "library/throttlestate.h"
#include "library/defines.h"

void CalibrateThrottle(void)
{
  ThrottleState* throttleState = GetThrottleState();

  timeBeginPeriod(1);	//Needed to get max resolution from the timer normally its 10Ms
  QueryPerformanceFrequency(&(throttleState->MasterClock));

  throttleState->OneFrame.QuadPart = throttleState->MasterClock.QuadPart / (TARGETFRAMERATE);
  throttleState->OneMs.QuadPart = throttleState->MasterClock.QuadPart / 1000;
  throttleState->fMasterClock = (float)(throttleState->MasterClock.QuadPart);
}
