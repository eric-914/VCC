#include "library/graphicsstate.h"

#include "SetupDisplay.h"

// These grab the Video info for all COCO 2 modes
void SetGimeVdgOffset(unsigned char offset)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC2Offset != offset)
  {
    gs->CC2Offset = offset;
    SetupDisplay();
  }
}
