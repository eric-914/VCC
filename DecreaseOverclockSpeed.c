#include <ShlObj.h>

#include "configstate.h"
#include "resource.h"

#include "library/systemstate.h"

/**
 * Decrease the overclock speed, as seen after a POKE 65497,0.
 *
 * Setting this value to 0 will make the emulator pause.  Hence the minimum of 2.
 */
void DecreaseOverclockSpeed(SystemState* systemState)
{
  ConfigState* configState = GetConfigState();

  if (configState->TempConfig.CPUMultiplyer == 2)
  {
    return;
  }

  configState->TempConfig.CPUMultiplyer = (unsigned char)(configState->TempConfig.CPUMultiplyer - 1);

  // Send updates to the dialog if it's open.
  if (systemState->ConfigDialog != NULL)
  {
    HWND hDlg = configState->hWndConfig[1];

    SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, configState->TempConfig.CPUMultiplyer);

    sprintf(configState->OutBuffer, "%2.3f Mhz", (float)(configState->TempConfig.CPUMultiplyer) * 0.894);

    SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));
  }

  configState->CurrentConfig = configState->TempConfig;

  systemState->ResetPending = 4;
}
