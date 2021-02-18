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
  CpuRegister pc, x, y, u, s, dp, d;

  unsigned char ccbits;
  unsigned int cc[8];

  unsigned char* ureg8[8];
  unsigned short* xfreg16[8];

  char InInterrupt;
  unsigned char IRQWaiter;
  unsigned char PendingInterrupts;
  unsigned int SyncWaiting;
  int CycleCounter;
} MC6809State;

extern "C" __declspec(dllexport) MC6809State * __cdecl GetMC6809State();
