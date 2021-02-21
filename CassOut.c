#include "library/CoCo.h"
#include "library/MC6821.h"

extern "C" {
  __declspec(dllexport) void __cdecl CassOut(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->CassBuffer[coco->AudioIndex++] = GetCasSample();
  }
}
