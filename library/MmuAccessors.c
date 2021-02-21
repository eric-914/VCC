#include <windows.h>

#include "MMU.h"

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuRegister(unsigned char Register, unsigned char data)
  {
    unsigned char BankRegister, Task;

    MmuState* mmuState = GetMmuState();

    BankRegister = Register & 7;
    Task = !!(Register & 8);

    mmuState->MmuRegisters[Task][BankRegister] = mmuState->MmuPrefix | (data & mmuState->RamMask[mmuState->CurrentRamConfig]); //gime.c returns what was written so I can get away with this
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuTask(unsigned char task)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->MmuTask = task;
    mmuState->MmuState = (!mmuState->MmuEnabled) << 1 | mmuState->MmuTask;
  }
}

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

extern "C" {
  __declspec(dllexport) void __cdecl SetVectors(unsigned char data)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->RamVectors = !!data; //Bit 3 of $FF90 MC3
  }
}