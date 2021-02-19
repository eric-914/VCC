#include "cocostate.h"
#include "GetCasSample.h"

extern "C" {
  __declspec(dllexport) void __cdecl CassOut(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->CassBuffer[coco->AudioIndex++] = GetCasSample();
  }
}
