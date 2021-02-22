#include "library/HD6309.h"

#include "library/HD6309Macros.h"

static HD6309State* hd63096State = GetHD6309State();

static unsigned char* NatEmuCycles[] =
{
  &(hd63096State->NatEmuCycles65),
  &(hd63096State->NatEmuCycles64),
  &(hd63096State->NatEmuCycles32),
  &(hd63096State->NatEmuCycles21),
  &(hd63096State->NatEmuCycles54),
  &(hd63096State->NatEmuCycles97),
  &(hd63096State->NatEmuCycles85),
  &(hd63096State->NatEmuCycles51),
  &(hd63096State->NatEmuCycles31),
  &(hd63096State->NatEmuCycles1110),
  &(hd63096State->NatEmuCycles76),
  &(hd63096State->NatEmuCycles75),
  &(hd63096State->NatEmuCycles43),
  &(hd63096State->NatEmuCycles87),
  &(hd63096State->NatEmuCycles86),
  &(hd63096State->NatEmuCycles98),
  &(hd63096State->NatEmuCycles2726),
  &(hd63096State->NatEmuCycles3635),
  &(hd63096State->NatEmuCycles3029),
  &(hd63096State->NatEmuCycles2827),
  &(hd63096State->NatEmuCycles3726),
  &(hd63096State->NatEmuCycles3130),
  &(hd63096State->NatEmuCycles42),
  &(hd63096State->NatEmuCycles53)
};

void setmd(unsigned char binmd)
{
  MD_NATIVE6309 = !!(binmd & (1 << NATIVE6309));
  MD_FIRQMODE = !!(binmd & (1 << FIRQMODE));
  MD_UNDEFINED2 = !!(binmd & (1 << MD_UNDEF2));
  MD_UNDEFINED3 = !!(binmd & (1 << MD_UNDEF3));
  MD_UNDEFINED4 = !!(binmd & (1 << MD_UNDEF4));
  MD_UNDEFINED5 = !!(binmd & (1 << MD_UNDEF5));
  MD_ILLEGAL = !!(binmd & (1 << ILLEGAL));
  MD_ZERODIV = !!(binmd & (1 << ZERODIV));

  for (short i = 0; i < 24; i++)
  {
    *NatEmuCycles[i] = hd63096State->InsCycles[MD_NATIVE6309][i];
  }
}

unsigned char getmd(void)
{
  unsigned char binmd = 0;

#define TSM(_MD, _F) if (_MD) { binmd |= (1 << _F); } //--Can't use the same macro name

  TSM(MD_NATIVE6309, NATIVE6309);
  TSM(MD_FIRQMODE, FIRQMODE);
  TSM(MD_UNDEFINED2, MD_UNDEF2);
  TSM(MD_UNDEFINED3, MD_UNDEF3);
  TSM(MD_UNDEFINED4, MD_UNDEF4);
  TSM(MD_UNDEFINED5, MD_UNDEF5);
  TSM(MD_ILLEGAL, ILLEGAL);
  TSM(MD_ZERODIV, ZERODIV);

  return(binmd);
}
