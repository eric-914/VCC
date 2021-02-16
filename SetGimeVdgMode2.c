#include "library/graphicsstate.h"

#include "SetupDisplay.h"

void SetGimeVdgMode2(unsigned char vdgmode2) //5 bits from PIA Register
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC2VDGPiaMode != vdgmode2)
  {
    gs->CC2VDGPiaMode = vdgmode2;
    SetupDisplay();
    gs->BorderChange = 3;
  }
}
