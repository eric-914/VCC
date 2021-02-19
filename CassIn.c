#include "cocostate.h"
#include "SetCassetteSample.h"
#include "GetDACSample.h"

extern "C" {
  __declspec(dllexport) void __cdecl CassIn(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->AudioBuffer[coco->AudioIndex] = GetDACSample(); 

    SetCassetteSample(coco->CassBuffer[coco->AudioIndex++]);
  }
}
