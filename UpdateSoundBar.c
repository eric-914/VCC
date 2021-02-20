#include <ShlObj.h>

#include "configstate.h"
#include "resources/resource.h"

void UpdateSoundBar(unsigned short left, unsigned short right)
{
  ConfigState* configState = GetConfigState();

  if (configState->hDlgBar == NULL) {
    return;
  }

  SendDlgItemMessage(configState->hDlgBar, IDC_PROGRESSLEFT, PBM_SETPOS, left >> 8, 0);
  SendDlgItemMessage(configState->hDlgBar, IDC_PROGRESSRIGHT, PBM_SETPOS, right >> 8, 0);
}
