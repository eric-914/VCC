#include "cocostate.h"

#include "library/defines.h"

extern "C" {
  __declspec(dllexport) void __cdecl SetMasterTickCounter(void)
  {
    double Rate[2] = { PICOSECOND / (TARGETFRAMERATE * LINESPERFIELD), PICOSECOND / COLORBURST };

    CoCoState* coco = GetCoCoState();

    if (coco->UnxlatedTickCounter == 0) {
      coco->MasterTickCounter = 0;
    }
    else {
      coco->MasterTickCounter = (coco->UnxlatedTickCounter + 2) * Rate[coco->TimerClockRate];
    }

    if (coco->MasterTickCounter != coco->OldMaster)
    {
      coco->OldMaster = coco->MasterTickCounter;
      coco->PicosToInterrupt = coco->MasterTickCounter;
    }

    coco->IntEnable = coco->MasterTickCounter == 0 ? 0 : 1;
  }
}
