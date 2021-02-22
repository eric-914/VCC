#include "library/VCC.h"
#include "library/DirectDraw.h"
#include "library/Graphics.h"
#include "library/systemstate.h"

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
