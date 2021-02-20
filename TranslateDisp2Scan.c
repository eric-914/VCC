#include <assert.h>

#include "library/configstate.h"

unsigned char TranslateDisp2Scan(LRESULT x)
{
  ConfigState* configState = GetConfigState();

  assert(x >= 0 && x < SCAN_TRANS_COUNT);

  return configState->TranslateDisp2Scan[x];
}
