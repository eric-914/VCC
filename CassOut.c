#include "cocostate.h"

extern unsigned char GetCasSample(void); /* mc6821.c */

extern "C" {
  __declspec(dllexport) void __cdecl CassOut(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->CassBuffer[coco->AudioIndex++] = GetCasSample();
  }
}
