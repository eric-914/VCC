#include "library/graphicsstate.h"

#include "SetupDisplay.h"

void SetGimeBorderColor(unsigned char data)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC3BorderColor != (data & 63))
  {
    gs->CC3BorderColor = data & 63;
    SetupDisplay();
    gs->BorderChange = 3;
  }
}
