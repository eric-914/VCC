#include "library/Mmu.h"
#include "library/CoCo.h"

void SetInit1(unsigned char data)
{
  SetMmuTask(data & 1);			//TR
  SetTimerClockRate(data & 32);	//TINS
}
