#include "library/graphicsstate.h"

#include "SetupDisplay.h"

void SetGimeVdgMode(unsigned char vdgMode) //3 bits from SAM Registers
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC2VDGMode != vdgMode)
  {
    gs->CC2VDGMode = vdgMode;
    SetupDisplay();
    gs->BorderChange = 3;
  }
}
