#include "cocostate.h"

#include "library/graphicsstate.h"

void SetLinesperScreen(unsigned char lines)
{
  CoCoState* coco = GetCoCoState();
  GraphicsState* graphicsState = GetGraphicsState();

  lines = (lines & 3);

  coco->LinesperScreen = graphicsState->Lpf[lines];
  coco->TopBorder = graphicsState->VcenterTable[lines];
  coco->BottomBorder = 243 - (coco->TopBorder + coco->LinesperScreen); //4 lines of top border are unrendered 244-4=240 rendered scanlines
}
