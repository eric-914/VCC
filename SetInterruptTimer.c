#include "library/cocostate.h"

#include "SetMasterTickCounter.h"

extern "C" {
  __declspec(dllexport) void __cdecl SetInterruptTimer(unsigned short timer)
  {
    GetCoCoState()->UnxlatedTickCounter = (timer & 0xFFF);

    SetMasterTickCounter();
  }
}
