#include <assert.h>

#include "library/configstate.h"

unsigned char TranslateScan2Disp(int x)
{
  ConfigState* configState = GetConfigState();

  assert(x >= 0 && x < SCAN_TRANS_COUNT);

  return configState->TranslateScan2Disp[x];
}
