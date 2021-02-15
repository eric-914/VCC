#include "cocostate.h"

#include "FlushCassetteBuffer.h"
#include "LoadCassetteBuffer.h"
#include "FlushAudioBuffer.h"

#include "library/graphicsstate.h"

extern /* _inline */ int CPUCycle(void);

extern void irq_fs(int);
extern unsigned char LockScreen(SystemState* systemState);
extern void UnlockScreen(SystemState* USState);
extern void GimeAssertVertInterrupt(void);
extern void SetBoarderChange(unsigned char);
extern float CalculateFPS(void);

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

  for (systemState->LineCounter = 0; systemState->LineCounter < 4; systemState->LineCounter++) {		//4 non-Rendered top Boarder lines
    CPUCycle();
  }

  if (!(FrameCounter % systemState->FrameSkip)) {
    if (LockScreen(systemState)) {
      return(0);
    }
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < (coco->TopBoarder - 4); systemState->LineCounter++)
  {
    if (!(FrameCounter % systemState->FrameSkip)) {
      coco->DrawTopBoarder[systemState->BitDepth](systemState);
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

  for (systemState->LineCounter = 0; systemState->LineCounter < (coco->BottomBoarder); systemState->LineCounter++)	// Bottom boarder
  {
    CPUCycle();

    if (!(FrameCounter % systemState->FrameSkip)) {
      coco->DrawBottomBoarder[systemState->BitDepth](systemState);
    }
  }

  if (!(FrameCounter % systemState->FrameSkip))
  {
    UnlockScreen(systemState);
    SetBoarderChange(0);
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
