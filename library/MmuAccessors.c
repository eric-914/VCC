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

extern "C" {
  __declspec(dllexport) void __cdecl SetRomMap(unsigned char data)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->RomMap = (data & 3);

    UpdateMmuArray();
  }
}

