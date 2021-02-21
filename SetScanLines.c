#include "library/VCC.h"

#include "library/DirectDrawAccessors.h"

#include "library/systemstate.h"
#include "library/graphicsstate.h"

unsigned char SetScanLines(SystemState* systemState, unsigned char lines)
{
  if (lines != QUERY)
  {
    systemState->ScanLines = lines;

    Cls(0, systemState);

    GetGraphicsState()->BorderChange = 3;
  }

  return(0);
}
