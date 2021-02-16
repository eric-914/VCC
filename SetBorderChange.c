#include "library/graphicsstate.h"

void SetBorderChange(unsigned char data)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->BorderChange > 0) {
    gs->BorderChange--;
  }
}
