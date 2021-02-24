#pragma once

#define D_REG	 (instance->d.Reg)
#define PC_REG (instance->pc.Reg)
#define X_REG	 (instance->x.Reg)
#define Y_REG	 (instance->y.Reg)
#define U_REG	 (instance->u.Reg)
#define S_REG	 (instance->s.Reg)
#define A_REG	 (instance->d.B.msb)
#define B_REG	 (instance->d.B.lsb)

#define DP_REG (instance->dp.Reg)

#define DPA  (instance->dp.B.msb)
#define PC_H (instance->pc.B.msb)
#define PC_L (instance->pc.B.lsb)
#define U_H  (instance->u.B.msb)
#define U_L  (instance->u.B.lsb)
#define S_H  (instance->s.B.msb)
#define S_L  (instance->s.B.lsb)
#define X_H  (instance->x.B.msb)
#define X_L  (instance->x.B.lsb)
#define Y_H  (instance->y.B.msb)
#define Y_L  (instance->y.B.lsb)

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
