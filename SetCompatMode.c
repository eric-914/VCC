#include "library/graphicsstate.h"

#include "SetupDisplay.h"

void SetCompatMode(unsigned char mode)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CompatMode != mode)
  {
    gs->CompatMode = mode;
    SetupDisplay();
    gs->BorderChange = 3;
  }
}
