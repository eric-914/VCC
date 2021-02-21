#include <windows.h>

#include "MMU.h"

extern "C" {
  __declspec(dllexport) void __cdecl SetMapType(unsigned char type)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->MapType = type;

    UpdateMmuArray();
  }
}

