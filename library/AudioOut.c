#include "cocostate.h"

#include "GetDACSample.h"

extern "C" {
  __declspec(dllexport) void __cdecl AudioOut(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->AudioBuffer[coco->AudioIndex++] = GetDACSample();
  }
}
