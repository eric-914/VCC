#include "throttlestate.h"

const unsigned char FrameSkip = 0;
const float fMasterClock = 0;

ThrottleState* InitializeInstance(ThrottleState* coco);

static ThrottleState* instance = InitializeInstance(new ThrottleState());

extern "C" {
  __declspec(dllexport) ThrottleState* __cdecl GetThrottleState() {
    return instance;
  }
}

ThrottleState* InitializeInstance(ThrottleState* t) {
  t->FrameSkip = FrameSkip;
  t->fMasterClock = fMasterClock;

  return t;
}
