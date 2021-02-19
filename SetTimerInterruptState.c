#include "library/cocostate.h"

extern "C" {
  __declspec(dllexport) void __cdecl SetTimerInterruptState(unsigned char state)
  {
    GetCoCoState()->TimerInterruptEnabled = state;
  }
}
