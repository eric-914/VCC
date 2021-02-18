#include "mc6809defs.h"
#include "mc6809state.h"

void mc6809_setcc(unsigned char bincc)
{
  MC6809State* mc6809State = GetMC6809State();

  CC_E = !!(bincc & (1 << E));
  CC_F = !!(bincc & (1 << F));
  CC_H = !!(bincc & (1 << H));
  CC_I = !!(bincc & (1 << I));
  CC_N = !!(bincc & (1 << N));
  CC_Z = !!(bincc & (1 << Z));
  CC_V = !!(bincc & (1 << V));
  CC_C = !!(bincc & (1 << C));
}

unsigned char mc6809_getcc(void)
{
  unsigned char bincc = 0;

  MC6809State* mc6809State = GetMC6809State();

#define TST(_CC, _F) if (_CC) { bincc |= (1 << _F); }

  TST(CC_E, E);
  TST(CC_F, F);
  TST(CC_H, H);
  TST(CC_I, I);
  TST(CC_N, N);
  TST(CC_Z, Z);
  TST(CC_V, V);
  TST(CC_C, C);

  return(bincc);
}
