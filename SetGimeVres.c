#include "library/graphicsstate.h"

#include "SetupDisplay.h"

void SetGimeVres(unsigned char vres)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC3Vres != vres)
  {
    gs->CC3Vres = vres;
    SetupDisplay();
    gs->BorderChange = 3;
  }
}
