#include "library/throttlestate.h"

#include "PurgeAuxBuffer.h"
#include "GetFreeBlockCount.h"
#include "AudioAccessors.h"

#include "library/defines.h"

void FrameWait(void)
{
  ThrottleState* throttleState = GetThrottleState();

  QueryPerformanceCounter(&(throttleState->CurrentTime));

  while ((throttleState->TargetTime.QuadPart - throttleState->CurrentTime.QuadPart) > (throttleState->OneMs.QuadPart * 2))	//If we have more that 2Ms till the end of the frame
  {
    Sleep(1);	//Give about 1Ms back to the system
    QueryPerformanceCounter(&(throttleState->CurrentTime));	//And check again
  }

  if (GetSoundStatus())	//Lean on the sound card a bit for timing
  {
    PurgeAuxBuffer();

    if (throttleState->FrameSkip == 1)
    {
      if (GetFreeBlockCount() > AUDIOBUFFERS / 2) {	//Dont let the buffer get lest that half full
        return;
      }

      while (GetFreeBlockCount() < 1);	// Dont let it fill up either
    }
  }

  while (throttleState->CurrentTime.QuadPart < throttleState->TargetTime.QuadPart) {	//Poll Untill frame end.
    QueryPerformanceCounter(&(throttleState->CurrentTime));
  }
}
