#pragma once

#define D_REG	(hd63096State->q.Word.lsw)
#define W_REG	(hd63096State->q.Word.msw)
#define PC_REG	(hd63096State->pc.Reg)
#define X_REG	(hd63096State->x.Reg)
#define Y_REG	(hd63096State->y.Reg)
#define U_REG	(hd63096State->u.Reg)
#define S_REG	(hd63096State->s.Reg)
#define A_REG	(hd63096State->q.Byte.lswmsb)
#define B_REG	(hd63096State->q.Byte.lswlsb)
#define E_REG	(hd63096State->q.Byte.mswmsb)
#define F_REG	(hd63096State->q.Byte.mswlsb)	
#define Q_REG	(hd63096State->q.Reg)
#define V_REG	(hd63096State->v.Reg)
#define O_REG	(hd63096State->z.Reg)

#define DPADDRESS(r) (hd63096State->dp.Reg | MemRead8(r))

#define DP_REG (hd63096State->dp.Reg)
#define DPA (hd63096State->dp.B.msb)
#define PC_H (hd63096State->pc.B.msb)
#define PC_L (hd63096State->pc.B.lsb)
#define U_H (hd63096State->u.B.msb)
#define U_L (hd63096State->u.B.lsb)
#define S_H (hd63096State->s.B.msb)
#define S_L (hd63096State->s.B.lsb)
#define X_H (hd63096State->x.B.msb)
#define X_L (hd63096State->x.B.lsb)
#define Y_H (hd63096State->y.B.msb)
#define Y_L (hd63096State->y.B.lsb)
#define Z_H (hd63096State->z.B.msb)
#define Z_L (hd63096State->z.B.lsb)

#define CC_E hd63096State->cc[E]
#define CC_F hd63096State->cc[F]
#define CC_H hd63096State->cc[H]
#define CC_I hd63096State->cc[I]
#define CC_N hd63096State->cc[N]
#define CC_Z hd63096State->cc[Z]
#define CC_V hd63096State->cc[V]
#define CC_C hd63096State->cc[C]

#define PUR(_I) (*(hd63096State->ureg8[_I]))
#define PXF(_I) (*(hd63096State->xfreg16[_I]))

#define MD_NATIVE6309	hd63096State->md[NATIVE6309]
#define MD_FIRQMODE	  hd63096State->md[FIRQMODE]
#define MD_UNDEFINED2 hd63096State->md[MD_UNDEF2]
#define MD_UNDEFINED3 hd63096State->md[MD_UNDEF3]
#define MD_UNDEFINED4 hd63096State->md[MD_UNDEF4]
#define MD_UNDEFINED5 hd63096State->md[MD_UNDEF5]
#define MD_ILLEGAL		hd63096State->md[ILLEGAL]
#define MD_ZERODIV		hd63096State->md[ZERODIV]
