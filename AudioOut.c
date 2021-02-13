#include "cocostate.h"

extern unsigned int GetDACSample(void); /* mc6821.c */

extern "C" {
  __declspec(dllexport) void __cdecl AudioOut(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->AudioBuffer[coco->AudioIndex++] = GetDACSample();
  }
}
