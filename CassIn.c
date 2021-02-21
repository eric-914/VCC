#include "library/CoCo.h"
#include "library/MC6821.h"

extern "C" {
  __declspec(dllexport) void __cdecl CassIn(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->AudioBuffer[coco->AudioIndex] = GetDACSample(); 

    SetCassetteSample(coco->CassBuffer[coco->AudioIndex++]);
  }
}
