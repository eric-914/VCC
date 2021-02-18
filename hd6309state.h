#pragma once

typedef union
{
  unsigned short Reg;
  struct
  {
    unsigned char lsb, msb;
  } B;
} CpuRegister;

typedef union
{
  unsigned int Reg;
  struct
  {
    unsigned short msw, lsw;
  } Word;
  struct
  {
    unsigned char mswlsb, mswmsb, lswlsb, lswmsb;	//Might be backwards
  } Byte;
} WideRegister;

typedef struct {
} HD6309State;

extern "C" __declspec(dllexport) HD6309State* __cdecl GetHD6309State();
