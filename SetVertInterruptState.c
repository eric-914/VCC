#include "library/cocostate.h"

extern "C" {
  __declspec(dllexport) void __cdecl SetVertInterruptState(unsigned char state)
  {
    GetCoCoState()->VertInterruptEnabled = !!state;
  }
}
