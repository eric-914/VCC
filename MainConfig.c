#include <windows.h>
#include <ShlObj.h>

#include "configstate.h"
#include "resource.h"

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

#include "library/keyboarddef.h"
#include "library/joystickstate.h"
#include "library/systemstate.h"

extern "C" void SetStickNumbers(unsigned char, unsigned char);
extern "C" void vccKeyboardBuildRuntimeTable(keyboardlayout_e keyBoardLayout);

extern SystemState EmuState;

LRESULT CALLBACK MainConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static char tabTitles[TABS][10] = { "Audio", "CPU", "Display", "Keyboard", "Joysticks", "Misc", "Tape", "BitBanger" };
  static unsigned char tabCount = 0, selectedTab = 0;
  static HWND hWndTabDialog;
  TCITEM tabs = TCITEM();

  ConfigState* configState = GetConfigState();
  JoystickState* joystickState = GetJoystickState();

  switch (message)
  {
  case WM_INITDIALOG:
    InitCommonControls();

    configState->TempConfig = configState->CurrentConfig;
    configState->CpuIcons[0] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_MOTO);
    configState->CpuIcons[1] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_HITACHI2);
    configState->MonIcons[0] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_COMPOSITE);
    configState->MonIcons[1] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_RGB);

    hWndTabDialog = GetDlgItem(hDlg, IDC_CONFIGTAB); //get handle of Tabbed Dialog

    //get handles to all the sub panels in the control
    configState->hWndConfig[0] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_AUDIO), hWndTabDialog, (DLGPROC)AudioConfig);
    configState->hWndConfig[1] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_CPU), hWndTabDialog, (DLGPROC)CpuConfig);
    configState->hWndConfig[2] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_DISPLAY), hWndTabDialog, (DLGPROC)DisplayConfig);
    configState->hWndConfig[3] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_INPUT), hWndTabDialog, (DLGPROC)InputConfig);
    configState->hWndConfig[4] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_JOYSTICK), hWndTabDialog, (DLGPROC)JoyStickConfig);
    configState->hWndConfig[5] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_MISC), hWndTabDialog, (DLGPROC)MiscConfig);
    configState->hWndConfig[6] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_CASSETTE), hWndTabDialog, (DLGPROC)TapeConfig);
    configState->hWndConfig[7] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_BITBANGER), hWndTabDialog, (DLGPROC)BitBangerConfig);

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
      EmuState.ResetPending = 4;

      if ((configState->CurrentConfig.RamSize != configState->TempConfig.RamSize) || (configState->CurrentConfig.CpuType != configState->TempConfig.CpuType)) {
        EmuState.ResetPending = 2;
      }

      if ((configState->CurrentConfig.SndOutDev != configState->TempConfig.SndOutDev) || (configState->CurrentConfig.AudioRate != configState->TempConfig.AudioRate)) {
        SoundInit(EmuState.WindowHandle, configState->SoundCards[configState->TempConfig.SndOutDev].Guid, configState->TempConfig.AudioRate);
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
      EmuState.ConfigDialog = NULL;
      break;

    case IDAPPLY:
      EmuState.ResetPending = 4;

      if ((configState->CurrentConfig.RamSize != configState->TempConfig.RamSize) || (configState->CurrentConfig.CpuType != configState->TempConfig.CpuType)) {
        EmuState.ResetPending = 2;
      }

      if ((configState->CurrentConfig.SndOutDev != configState->TempConfig.SndOutDev) || (configState->CurrentConfig.AudioRate != configState->TempConfig.AudioRate)) {
        SoundInit(EmuState.WindowHandle, configState->SoundCards[configState->TempConfig.SndOutDev].Guid, configState->TempConfig.AudioRate);
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

      EmuState.ConfigDialog = NULL;
      break;
    }

    break;
  }

  return FALSE;
}