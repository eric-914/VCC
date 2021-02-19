#include "library/cocostate.h"

#include "FlushCassetteBuffer.h"
#include "LoadCassetteBuffer.h"
#include "FlushAudioBuffer.h"
#include "CalculateFPS.h"
#include "LockScreen.h"
#include "UnlockScreen.h"
#include "CPUCycle.h"
#include "SetBorderChange.h"
#include "RegistersAccessors.h"
#include "irq_fs.h"

#include "library/graphicsstate.h"

float RenderFrame(SystemState* systemState)
{
  static unsigned short FrameCounter = 0;

  CoCoState* coco = GetCoCoState();

  //********************************Start of frame Render*****************************************************
  GetGraphicsState()->BlinkState = coco->BlinkPhase;

  irq_fs(0);				//FS low to High transition start of display Blink needs this

  for (systemState->LineCounter = 0; systemState->LineCounter < 13; systemState->LineCounter++) {		//Vertical Blanking 13 H lines
    CPUCycle();
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < 4; systemState->LineCounter++) {		//4 non-Rendered top Border lines
    CPUCycle();
  }

  if (!(FrameCounter % systemState->FrameSkip)) {
    if (LockScreen(systemState)) {
      return(0);
    }
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < (coco->TopBorder - 4); systemState->LineCounter++)
  {
    if (!(FrameCounter % systemState->FrameSkip)) {
      coco->DrawTopBorder[systemState->BitDepth](systemState);
    }

    CPUCycle();
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < coco->LinesperScreen; systemState->LineCounter++)		//Active Display area		
  {
    CPUCycle();

    if (!(FrameCounter % systemState->FrameSkip)) {
      coco->UpdateScreen[systemState->BitDepth](systemState);
    }
  }

  irq_fs(1);  //End of active display FS goes High to Low

  if (coco->VertInterruptEnabled) {
    GimeAssertVertInterrupt();
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < (coco->BottomBorder); systemState->LineCounter++)	// Bottom border
  {
    CPUCycle();

    if (!(FrameCounter % systemState->FrameSkip)) {
      coco->DrawBottomBorder[systemState->BitDepth](systemState);
    }
  }

  if (!(FrameCounter % systemState->FrameSkip))
  {
    UnlockScreen(systemState);
    SetBorderChange(0);
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < 6; systemState->LineCounter++) {		//Vertical Retrace 6 H lines
    CPUCycle();
  }

  switch (coco->SoundOutputMode)
  {
  case 0:
    FlushAudioBuffer(coco->AudioBuffer, coco->AudioIndex << 2);
    break;

  case 1:
    FlushCassetteBuffer(coco->CassBuffer, coco->AudioIndex);
    break;

  case 2:
    LoadCassetteBuffer(coco->CassBuffer);
    break;
  }

  coco->AudioIndex = 0;

  return(CalculateFPS());
}
