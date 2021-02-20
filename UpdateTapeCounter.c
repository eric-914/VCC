#include <windows.h>
#include <string>

#include "library/configstate.h"
#include "resources/resource.h"

#include "library/Cassette.h"
#include "library/fileoperations.h"

void UpdateTapeCounter(unsigned int counter, unsigned char tapeMode)
{
  ConfigState* configState = GetConfigState();

  if (configState->hDlgTape == NULL) {
    return;
  }

  configState->TapeCounter = counter;
  configState->Tmode = tapeMode;

  sprintf(configState->OutBuffer, "%i", configState->TapeCounter);

  SendDlgItemMessage(configState->hDlgTape, IDC_TCOUNT, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));
  SendDlgItemMessage(configState->hDlgTape, IDC_MODE, WM_SETTEXT, strlen(configState->Tmodes[configState->Tmode]), (LPARAM)(LPCSTR)(configState->Tmodes[configState->Tmode]));

  GetTapeName(configState->TapeFileName);
  FilePathStripPath(configState->TapeFileName);

  SendDlgItemMessage(configState->hDlgTape, IDC_TAPEFILE, WM_SETTEXT, strlen(configState->TapeFileName), (LPARAM)(LPCSTR)(configState->TapeFileName));

  switch (configState->Tmode)
  {
  case REC:
    SendDlgItemMessage(configState->hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0xAF, 0, 0));
    break;

  case PLAY:
    SendDlgItemMessage(configState->hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0xAF, 0));
    break;

  default:
    SendDlgItemMessage(configState->hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
    break;
  }
}
