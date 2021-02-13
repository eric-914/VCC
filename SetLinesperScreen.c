#include "cocostate.h"

#include "library/graphicsstate.h"

void SetLinesperScreen(unsigned char lines)
{
  CoCoState* coco = GetCoCoState();
  GraphicsState* graphicsState = GetGraphicsState();

  lines = (lines & 3);

  coco->LinesperScreen = graphicsState->Lpf[lines];
  coco->TopBoarder = graphicsState->VcenterTable[lines];
  coco->BottomBoarder = 243 - (coco->TopBoarder + coco->LinesperScreen); //4 lines of top boarder are unrendered 244-4=240 rendered scanlines
}
