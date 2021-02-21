#include "library/VCC.h"

#include "library/Graphics.h"

unsigned char SetMonitorType(unsigned char type)
{
  GraphicsState* gs = GetGraphicsState();

  int borderColor = gs->CC3BorderColor;
  SetGimeBorderColor(0);

  if (type != QUERY)
  {
    gs->MonType = type & 1;

    for (unsigned char palIndex = 0; palIndex < 16; palIndex++)
    {
      gs->Pallete16Bit[palIndex] = gs->PalleteLookup16[gs->MonType][gs->Pallete[palIndex]];
      gs->Pallete32Bit[palIndex] = gs->PalleteLookup32[gs->MonType][gs->Pallete[palIndex]];
      gs->Pallete8Bit[palIndex] = gs->PalleteLookup8[gs->MonType][gs->Pallete[palIndex]];
    }
  }

  SetGimeBorderColor(borderColor);

  return(gs->MonType);
}
