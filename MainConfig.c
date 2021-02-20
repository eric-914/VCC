#include <windows.h>
#include <ShlObj.h>

#include "library/Config.h"
#include "resources/resource.h"
#include "library/vccstate.h"

#include "RefreshJoystickStatus.h"

#include "AudioConfig.h"
#include "BitBangerConfig.h"
#include "CpuConfig.h"
#include "DisplayConfig.h"
#include "InputConfig.h"
#include "JoyStickConfig.h"
#include "LoadConfig.h"
#include "MiscConfig.h"
#include "TapeConfig.h"
#include "UpdateConfig.h"
#include "SoundInit.h"
#include "vccKeyboardBuildRuntimeTable.h"
#include "library/Joystick.h"

#include "library/keyboarddef.h"
#include "library/systemstate.h"

LRESULT CALLBACK MainConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static char tabTitles[TABS][10] = { "Audio", "CPU", "Display", "Keyboard", "Joysticks", "Misc", "Tape", "BitBanger" };
  static unsigned char tabCount = 0, selectedTab = 0;
  static HWND hWndTabDialog;
  TCITEM tabs = TCITEM();

  ConfigState* configState = GetConfigState();
  JoystickState* joystickState = GetJoystickState();
  VccState* vccState = GetVccState();

  switch (message)
  {
  case WM_INITDIALOG:
    InitCommonControls();

    configState->TempConfig = configState->CurrentConfig;
    configState->CpuIcons[0] = LoadIcon(vccState->EmuState.Resources, (LPCTSTR)IDI_MOTO);
    configState->CpuIcons[1] = LoadIcon(vccState->EmuState.Resources, (LPCTSTR)IDI_HITACHI2);
    configState->MonIcons[0] = LoadIcon(vccState->EmuState.Resources, (LPCTSTR)IDI_COMPOSITE);
    configState->MonIcons[1] = LoadIcon(vccState->EmuState.Resources, (LPCTSTR)IDI_RGB);

    hWndTabDialog = GetDlgItem(hDlg, IDC_CONFIGTAB); //get handle of Tabbed Dialog

    //get handles to all the sub panels in the control
    configState->hWndConfig[0] = CreateDialog(vccState->EmuState.Resources, MAKEINTRESOURCE(IDD_AUDIO), hWndTabDialog, (DLGPROC)AudioConfig);
    configState->hWndConfig[1] = CreateDialog(vccState->EmuState.Resources, MAKEINTRESOURCE(IDD_CPU), hWndTabDialog, (DLGPROC)CpuConfig);
    configState->hWndConfig[2] = CreateDialog(vccState->EmuState.Resources, MAKEINTRESOURCE(IDD_DISPLAY), hWndTabDialog, (DLGPROC)DisplayConfig);
    configState->hWndConfig[3] = CreateDialog(vccState->EmuState.Resources, MAKEINTRESOURCE(IDD_INPUT), hWndTabDialog, (DLGPROC)InputConfig);
    configState->hWndConfig[4] = CreateDialog(vccState->EmuState.Resources, MAKEINTRESOURCE(IDD_JOYSTICK), hWndTabDialog, (DLGPROC)JoyStickConfig);
    configState->hWndConfig[5] = CreateDialog(vccState->EmuState.Resources, MAKEINTRESOURCE(IDD_MISC), hWndTabDialog, (DLGPROC)MiscConfig);
    configState->hWndConfig[6] = CreateDialog(vccState->EmuState.Resources, MAKEINTRESOURCE(IDD_CASSETTE), hWndTabDialog, (DLGPROC)TapeConfig);
    configState->hWndConfig[7] = CreateDialog(vccState->EmuState.Resources, MAKEINTRESOURCE(IDD_BITBANGER), hWndTabDialog, (DLGPROC)BitBangerConfig);

    //Set the title text for all tabs
    for (tabCount = 0; tabCount < TABS; tabCount++)
    {
      tabs.mask = TCIF_TEXT | TCIF_IMAGE;
      tabs.iImage = -1;
      tabs.pszText = tabTitles[tabCount];

      TabCtrl_InsertItem(hWndTabDialog, tabCount, &tabs);
    }

    TabCtrl_SetCurSel(hWndTabDialog, 0);	//Set Initial Tab to 0

    for (tabCount = 0;tabCount < TABS;tabCount++) {	//Hide All the Sub Panels
      ShowWindow(configState->hWndConfig[tabCount], SW_HIDE);
    }

    SetWindowPos(configState->hWndConfig[0], HWND_TOP, 10, 30, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    RefreshJoystickStatus();

    break;

  case WM_NOTIFY:
    if ((LOWORD(wParam)) == IDC_CONFIGTAB) {
      selectedTab = TabCtrl_GetCurSel(hWndTabDialog);

      for (tabCount = 0;tabCount < TABS;tabCount++) {
        ShowWindow(configState->hWndConfig[tabCount], SW_HIDE);
      }

      SetWindowPos(configState->hWndConfig[selectedTab], HWND_TOP, 10, 30, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    }

    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
      configState->hDlgBar = NULL;
      configState->hDlgTape = NULL;
      vccState->EmuState.ResetPending = 4;

      if ((configState->CurrentConfig.RamSize != configState->TempConfig.RamSize) || (configState->CurrentConfig.CpuType != configState->TempConfig.CpuType)) {
        vccState->EmuState.ResetPending = 2;
      }

      if ((configState->CurrentConfig.SndOutDev != configState->TempConfig.SndOutDev) || (configState->CurrentConfig.AudioRate != configState->TempConfig.AudioRate)) {
        SoundInit(vccState->EmuState.WindowHandle, configState->SoundCards[configState->TempConfig.SndOutDev].Guid, configState->TempConfig.AudioRate);
      }

      configState->CurrentConfig = configState->TempConfig;

      vccKeyboardBuildRuntimeTable((keyboardlayout_e)(configState->CurrentConfig.KeyMap));

      joystickState->Right = configState->Right;
      joystickState->Left = configState->Left;

      SetStickNumbers(joystickState->Left.DiDevice, joystickState->Right.DiDevice);

      for (unsigned char temp = 0; temp < TABS; temp++)
      {
        DestroyWindow(configState->hWndConfig[temp]);
      }

#ifdef CONFIG_DIALOG_MODAL
      EndDialog(hDlg, LOWORD(wParam));
#else
      DestroyWindow(hDlg);
#endif
      vccState->EmuState.ConfigDialog = NULL;
      break;

    case IDAPPLY:
      vccState->EmuState.ResetPending = 4;

      if ((configState->CurrentConfig.RamSize != configState->TempConfig.RamSize) || (configState->CurrentConfig.CpuType != configState->TempConfig.CpuType)) {
        vccState->EmuState.ResetPending = 2;
      }

      if ((configState->CurrentConfig.SndOutDev != configState->TempConfig.SndOutDev) || (configState->CurrentConfig.AudioRate != configState->TempConfig.AudioRate)) {
        SoundInit(vccState->EmuState.WindowHandle, configState->SoundCards[configState->TempConfig.SndOutDev].Guid, configState->TempConfig.AudioRate);
      }

      configState->CurrentConfig = configState->TempConfig;

      vccKeyboardBuildRuntimeTable((keyboardlayout_e)(configState->CurrentConfig.KeyMap));

      joystickState->Right = configState->Right;
      joystickState->Left = configState->Left;

      SetStickNumbers(joystickState->Left.DiDevice, joystickState->Right.DiDevice);

      break;

    case IDCANCEL:
      for (unsigned char temp = 0; temp < TABS; temp++)
      {
        DestroyWindow(configState->hWndConfig[temp]);
      }

#ifdef CONFIG_DIALOG_MODAL
      EndDialog(hDlg, LOWORD(wParam));
#else
      DestroyWindow(hDlg);
#endif

      vccState->EmuState.ConfigDialog = NULL;
      break;
    }

    break;
  }

  return FALSE;
}
