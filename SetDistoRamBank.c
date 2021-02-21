#include "library/mmustate.h"
#include "library/MmuAccessors.h"
#include "library/Graphics.h"

void SetDistoRamBank(unsigned char data)
{
  MmuState* mmuState = GetMmuState();

  switch (mmuState->CurrentRamConfig)
  {
  case 0:	// 128K
    return;
    break;

  case 1:	//512K
    return;
    break;

  case 2:	//2048K
    SetVideoBank(data & 3);
    SetMmuPrefix(0);

    return;

  case 3:	//8192K	//No Can 3 
    SetVideoBank(data & 0x0F);
    SetMmuPrefix((data & 0x30) >> 4);

    return;
  }
}
