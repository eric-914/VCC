#include <windows.h>
#include <ShlObj.h>

#include "library/Config.h"
#include "resources/resource.h"

#include "library/Audio.h"

LRESULT CALLBACK AudioConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:
    configState->hDlgBar = hDlg;	//Save the handle to update sound bars

    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETPOS, 0, 0);
    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETRANGE32, 0, 0x7F);
    SendDlgItemMessage(hDlg, IDC_PROGRESSRIGHT, PBM_SETPOS, 0, 0);
    SendDlgItemMessage(hDlg, IDC_PROGRESSRIGHT, PBM_SETRANGE32, 0, 0x7F);
    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETPOS, 0, 0);
    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETBARCOLOR, 0, 0xFFFF);
    SendDlgItemMessage(hDlg, IDC_PROGRESSRIGHT, PBM_SETBARCOLOR, 0, 0xFFFF);
    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETBKCOLOR, 0, 0);
    SendDlgItemMessage(hDlg, IDC_PROGRESSRIGHT, PBM_SETBKCOLOR, 0, 0);

    for (unsigned char index = 0; index < configState->NumberOfSoundCards; index++) {
      SendDlgItemMessage(hDlg, IDC_SOUNDCARD, CB_ADDSTRING, (WPARAM)0, (LPARAM)(configState->SoundCards[index].CardName));
    }

    for (unsigned char index = 0; index < 4; index++) {
      SendDlgItemMessage(hDlg, IDC_RATE, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetRateList(index));
    }

    SendDlgItemMessage(hDlg, IDC_RATE, CB_SETCURSEL, (WPARAM)(configState->TempConfig.AudioRate), (LPARAM)0);

    configState->TempConfig.SndOutDev = 0;

    for (unsigned char index = 0; index < configState->NumberOfSoundCards; index++) {
      if (!strcmp(configState->SoundCards[index].CardName, configState->TempConfig.SoundCardName)) {
        configState->TempConfig.SndOutDev = index;
      }
    }

    SendDlgItemMessage(hDlg, IDC_SOUNDCARD, CB_SETCURSEL, (WPARAM)(configState->TempConfig.SndOutDev), (LPARAM)0);

    break;

  case WM_COMMAND:
    configState->TempConfig.SndOutDev = (unsigned char)SendDlgItemMessage(hDlg, IDC_SOUNDCARD, CB_GETCURSEL, 0, 0);
    configState->TempConfig.AudioRate = (unsigned char)SendDlgItemMessage(hDlg, IDC_RATE, CB_GETCURSEL, 0, 0);

    strcpy(configState->TempConfig.SoundCardName, configState->SoundCards[configState->TempConfig.SndOutDev].CardName);

    break;
  }

  return(0);
}
