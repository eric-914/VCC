#include "library/di.version.h"
#include <dinput.h>

#include "library/configstate.h"

#include "library/configdef.h"

void BuildTransDisp2ScanTable()
{
  ConfigState* configState = GetConfigState();

  for (int i = 0; i < SCAN_TRANS_COUNT; i++) {
    for (int j = SCAN_TRANS_COUNT - 1; j >= 0; j--) {
      if (j == configState->TranslateScan2Disp[i]) {
        configState->TranslateDisp2Scan[j] = (unsigned char)i;
      }
    }
  }

  configState->TranslateDisp2Scan[0] = 0;

  // Left and Right Shift
  configState->TranslateDisp2Scan[51] = DIK_LSHIFT;
}
