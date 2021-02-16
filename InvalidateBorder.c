#include "library/graphicsstate.h"

void InvalidateBorder(void)
{
  GetGraphicsState()->BorderChange = 5;
}
