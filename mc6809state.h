#pragma once

typedef union
{
  unsigned short Reg;
  struct
  {
    unsigned char lsb, msb;
  } B;
} CpuRegister;

typedef struct {
} MC6809State;

extern "C" __declspec(dllexport) MC6809State* __cdecl GetMC6809State();
