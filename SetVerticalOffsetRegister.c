#include "library/graphicsstate.h"

#include "SetupDisplay.h"

//These grab the Video info for all COCO 3 modes
void SetVerticalOffsetRegister(unsigned short voRegister)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->VerticalOffsetRegister != voRegister)
  {
    gs->VerticalOffsetRegister = voRegister;

    SetupDisplay();
  }
}
