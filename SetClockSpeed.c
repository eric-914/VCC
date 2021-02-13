#include "cocostate.h"

extern "C" {
  __declspec(dllexport) void __cdecl SetClockSpeed(unsigned short cycles)
  {
    GetCoCoState()->OverClock = cycles;
  }
}
