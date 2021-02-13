#include "cocostate.h"

#include "SetMasterTickCounter.h"

extern "C" {
  __declspec(dllexport) void __cdecl SetTimerClockRate(unsigned char clockRate)
  {
    //1= 279.265nS (1/ColorBurst)
    //0= 63.695uS  (1/60*262)  1 scanline time

    GetCoCoState()->TimerClockRate = !!clockRate;

    SetMasterTickCounter();
  }
}
