#include "library/cocostate.h"

extern "C" {
  __declspec(dllexport) void __cdecl CocoReset(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->HorzInterruptEnabled = 0;
    coco->VertInterruptEnabled = 0;
    coco->TimerInterruptEnabled = 0;
    coco->MasterTimer = 0;
    coco->TimerClockRate = 0;
    coco->MasterTickCounter = 0;
    coco->UnxlatedTickCounter = 0;
    coco->OldMaster = 0;

    coco->SoundInterrupt = 0;
    coco->PicosToSoundSample = 0;
    coco->CycleDrift = 0;
    coco->CyclesThisLine = 0;
    coco->PicosThisLine = 0;
    coco->IntEnable = 0;
    coco->AudioIndex = 0;
  }
}

