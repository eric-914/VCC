#pragma once

typedef struct {
  unsigned char* Rom;

  unsigned char EnhancedFIRQFlag;
  unsigned char EnhancedIRQFlag;
  unsigned char VDG_Mode;
  unsigned char Dis_Offset;
  unsigned char MPU_Rate;
  unsigned char LastIrq;
  unsigned char LastFirq;

  unsigned short VerticalOffsetRegister;

  int InterruptTimer;

  unsigned char GimeRegisters[256];
} RegistersState;

extern "C" __declspec(dllexport) RegistersState * __cdecl GetRegistersState();
