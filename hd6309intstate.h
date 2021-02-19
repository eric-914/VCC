#pragma once

typedef struct {
  unsigned char IRQWaiter;
  unsigned char PendingInterrupts;
  unsigned char InsCycles[2][25];
  unsigned char* NatEmuCycles[24];
} HD6309IntState;

extern "C" __declspec(dllexport) HD6309IntState * __cdecl GetHD6309IntState();
