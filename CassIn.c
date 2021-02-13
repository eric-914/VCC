#include "cocostate.h"

extern unsigned int GetDACSample(void); /* mc6821.c */
extern void SetCassetteSample(unsigned char); /* mc6821.c */

extern "C" {
  __declspec(dllexport) void __cdecl CassIn(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->AudioBuffer[coco->AudioIndex] = GetDACSample(); 

    SetCassetteSample(coco->CassBuffer[coco->AudioIndex++]);
  }
}
