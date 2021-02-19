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

#define D_REG	(mc6809State->d.Reg)
#define PC_REG (mc6809State->pc.Reg)
#define X_REG	(mc6809State->x.Reg)
#define Y_REG	(mc6809State->y.Reg)
#define U_REG	(mc6809State->u.Reg)
#define S_REG	(mc6809State->s.Reg)
#define A_REG	(mc6809State->d.B.msb)
#define B_REG	(mc6809State->d.B.lsb)

#define DP_REG (mc6809State->dp.Reg)
#define DPA (mc6809State->dp.B.msb)
#define PC_H (mc6809State->pc.B.msb)
#define PC_L (mc6809State->pc.B.lsb)
#define U_H (mc6809State->u.B.msb)
#define U_L (mc6809State->u.B.lsb)
#define S_H (mc6809State->s.B.msb)
#define S_L (mc6809State->s.B.lsb)
#define X_H (mc6809State->x.B.msb)
#define X_L (mc6809State->x.B.lsb)
#define Y_H (mc6809State->y.B.msb)
#define Y_L (mc6809State->y.B.lsb)

#define CC_E mc6809State->cc[E]
#define CC_F mc6809State->cc[F]
#define CC_H mc6809State->cc[H]
#define CC_I mc6809State->cc[I]
#define CC_N mc6809State->cc[N]
#define CC_Z mc6809State->cc[Z]
#define CC_V mc6809State->cc[V]
#define CC_C mc6809State->cc[C]

#define PUR(_I) (*(mc6809State->ureg8[_I]))
#define PXF(_I) (*(mc6809State->xfreg16[_I]))
