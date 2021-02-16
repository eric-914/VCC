#include "library/graphicsstate.h"

#include "SetupDisplay.h"

void SetVideoBank(unsigned char data)
{
  GetGraphicsState()->DistoOffset = data * (512 * 1024);

  SetupDisplay();
}
