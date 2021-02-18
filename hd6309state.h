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

extern "C" __declspec(dllexport) HD6309State * __cdecl GetHD6309State();

#define D_REG	(q.Word.lsw)
#define W_REG	(q.Word.msw)
#define PC_REG	(pc.Reg)
#define X_REG	(x.Reg)
#define Y_REG	(y.Reg)
#define U_REG	(u.Reg)
#define S_REG	(s.Reg)
#define A_REG	(q.Byte.lswmsb)
#define B_REG	(q.Byte.lswlsb)
#define E_REG	(q.Byte.mswmsb)
#define F_REG	(q.Byte.mswlsb)	
#define Q_REG	(q.Reg)
#define V_REG	(v.Reg)
#define O_REG	(z.Reg)

#define DPADDRESS(r) (dp.Reg | MemRead8(r))

#define DP_REG (dp.Reg)
#define DPA (dp.B.msb)
#define PC_H (pc.B.msb)
#define PC_L (pc.B.lsb)
#define U_H (u.B.msb)
#define U_L (u.B.lsb)
#define S_H (s.B.msb)
#define S_L (s.B.lsb)
#define X_H (x.B.msb)
#define X_L (x.B.lsb)
#define Y_H (y.B.msb)
#define Y_L (y.B.lsb)
#define Z_H (z.B.msb)
#define Z_L (z.B.lsb)

#define CC_E cc[E]
#define CC_F cc[F]
#define CC_H cc[H]
#define CC_I cc[I]
#define CC_N cc[N]
#define CC_Z cc[Z]
#define CC_V cc[V]
#define CC_C cc[C]

#define PUR(_I) (*(ureg8[_I]))
#define PXF(_I) (*(xfreg16[_I]))

#define MD_NATIVE6309	md[NATIVE6309]
#define MD_FIRQMODE	  md[FIRQMODE]
#define MD_UNDEFINED2 md[MD_UNDEF2]
#define MD_UNDEFINED3 md[MD_UNDEF3]
#define MD_UNDEFINED4 md[MD_UNDEF4]
#define MD_UNDEFINED5 md[MD_UNDEF5]
#define MD_ILLEGAL		md[ILLEGAL]
#define MD_ZERODIV		md[ZERODIV]
