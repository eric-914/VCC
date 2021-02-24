#pragma once

#define D_REG	  (instance->q.Word.lsw)
#define W_REG	  (instance->q.Word.msw)
#define PC_REG	(instance->pc.Reg)
#define X_REG	  (instance->x.Reg)
#define Y_REG	  (instance->y.Reg)
#define U_REG	  (instance->u.Reg)
#define S_REG	  (instance->s.Reg)
#define A_REG	  (instance->q.Byte.lswmsb)
#define B_REG	  (instance->q.Byte.lswlsb)
#define E_REG	  (instance->q.Byte.mswmsb)
#define F_REG	  (instance->q.Byte.mswlsb)	
#define Q_REG	  (instance->q.Reg)
#define V_REG	  (instance->v.Reg)
#define O_REG	  (instance->z.Reg)

#define DPADDRESS(r) (instance->dp.Reg | MemRead8(r))

#define DP_REG (instance->dp.Reg)

#define DPA   (instance->dp.B.msb)
#define PC_H  (instance->pc.B.msb)
#define PC_L  (instance->pc.B.lsb)
#define U_H   (instance->u.B.msb)
#define U_L   (instance->u.B.lsb)
#define S_H   (instance->s.B.msb)
#define S_L   (instance->s.B.lsb)
#define X_H   (instance->x.B.msb)
#define X_L   (instance->x.B.lsb)
#define Y_H   (instance->y.B.msb)
#define Y_L   (instance->y.B.lsb)
#define Z_H   (instance->z.B.msb)
#define Z_L   (instance->z.B.lsb)

#define CC_E instance->cc[E]
#define CC_F instance->cc[F]
#define CC_H instance->cc[H]
#define CC_I instance->cc[I]
#define CC_N instance->cc[N]
#define CC_Z instance->cc[Z]
#define CC_V instance->cc[V]
#define CC_C instance->cc[C]

#define PUR(_I) (*(instance->ureg8[_I]))
#define PXF(_I) (*(instance->xfreg16[_I]))

#define MD_NATIVE6309	instance->md[NATIVE6309]
#define MD_FIRQMODE	  instance->md[FIRQMODE]
#define MD_UNDEFINED2 instance->md[MD_UNDEF2]
#define MD_UNDEFINED3 instance->md[MD_UNDEF3]
#define MD_UNDEFINED4 instance->md[MD_UNDEF4]
#define MD_UNDEFINED5 instance->md[MD_UNDEF5]
#define MD_ILLEGAL		instance->md[ILLEGAL]
#define MD_ZERODIV		instance->md[ZERODIV]
