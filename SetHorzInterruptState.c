#include "library/cocostate.h"

extern "C" {
  __declspec(dllexport) void __cdecl SetHorzInterruptState(unsigned char state)
  {
    GetCoCoState()->HorzInterruptEnabled = !!state;
  }
}
