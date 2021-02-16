#include "library/graphicsstate.h"

#include "SetupDisplay.h"

void SetGimeHorzOffset(unsigned char data)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->HorzOffsetReg != data)
  {
    gs->Hoffset = (data << 1);
    gs->HorzOffsetReg = data;
    SetupDisplay();
  }
}
