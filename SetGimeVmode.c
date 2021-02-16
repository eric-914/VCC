#include "library/graphicsstate.h"

#include "SetupDisplay.h"

void SetGimeVmode(unsigned char vmode)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC3Vmode != vmode)
  {
    gs->CC3Vmode = vmode;
    SetupDisplay();
    gs->BorderChange = 3;
  }
}
