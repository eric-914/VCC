/*
Copyright 2015 by Joseph Forgione
This file is part of VCC (Virtual Color Computer).

    VCC (Virtual Color Computer) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VCC (Virtual Color Computer) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VCC (Virtual Color Computer).  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <ShlObj.h>

#include "resource.h"
#include "mc6821.h"
#include "audio.h"
#include "DirectDrawInterface.h"
#include "keyboard.h"
#include "configstate.h"

#include "GetTapeCounter.h"
#include "SetTapeCounter.h"
#include "SetTapeMode.h"
#include "LoadTape.h"
#include "GetTapeName.h"
#include "GetKeyName.h"
#include "TranslateScan2Disp.h"
#include "TranslateDisp2Scan.h"
#include "RefreshJoystickStatus.h"
#include "SelectFile.h"

#include "library/cassettedef.h"
#include "library/joystickinput.h"
#include "library/joystickstate.h"

LRESULT CALLBACK CpuConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETRANGE, TRUE, MAKELONG(2, configState->CurrentConfig.MaxOverclock));	//Maximum overclock

    sprintf(configState->OutBuffer, "%2.3f Mhz", (float)(configState->TempConfig.CPUMultiplyer) * .894);

    SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));
    SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, configState->TempConfig.CPUMultiplyer);

    for (unsigned char temp = 0; temp <= 3; temp++) {
      SendDlgItemMessage(hDlg, configState->Ramchoice[temp], BM_SETCHECK, (temp == configState->TempConfig.RamSize), 0);
    }

    for (unsigned char temp = 0; temp <= 1; temp++) {
      SendDlgItemMessage(hDlg, configState->Cpuchoice[temp], BM_SETCHECK, (temp == configState->TempConfig.CpuType), 0);
    }

    SendDlgItemMessage(hDlg, IDC_CPUICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->CpuIcons[configState->TempConfig.CpuType]));

    break;

  case WM_HSCROLL:
    configState->TempConfig.CPUMultiplyer = (unsigned char)SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_GETPOS, (WPARAM)0, (WPARAM)0);

    sprintf(configState->OutBuffer, "%2.3f Mhz", (float)(configState->TempConfig.CPUMultiplyer) * .894);

    SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));

    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_128K:
    case IDC_512K:
    case IDC_2M:
    case IDC_8M:
      for (unsigned char temp = 0; temp <= 3; temp++) {
        if (LOWORD(wParam) == configState->Ramchoice[temp])
        {
          for (unsigned char temp2 = 0; temp2 <= 3; temp2++) {
            SendDlgItemMessage(hDlg, configState->Ramchoice[temp2], BM_SETCHECK, 0, 0);
          }

          SendDlgItemMessage(hDlg, configState->Ramchoice[temp], BM_SETCHECK, 1, 0);

          configState->TempConfig.RamSize = temp;
        }
      }

      break;

    case IDC_6809:
    case IDC_6309:
      for (unsigned char temp = 0; temp <= 1; temp++) {
        if (LOWORD(wParam) == configState->Cpuchoice[temp])
        {
          for (unsigned char temp2 = 0; temp2 <= 1; temp2++) {
            SendDlgItemMessage(hDlg, configState->Cpuchoice[temp2], BM_SETCHECK, 0, 0);
          }

          SendDlgItemMessage(hDlg, configState->Cpuchoice[temp], BM_SETCHECK, 1, 0);

          configState->TempConfig.CpuType = temp;

          SendDlgItemMessage(hDlg, IDC_CPUICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->CpuIcons[configState->TempConfig.CpuType]));
        }
      }

      break;
    }

    break;
  }

  return(0);
}

LRESULT CALLBACK MiscConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg, IDC_AUTOSTART, BM_SETCHECK, configState->TempConfig.AutoStart, 0);
    SendDlgItemMessage(hDlg, IDC_AUTOCART, BM_SETCHECK, configState->TempConfig.CartAutoStart, 0);

    break;

  case WM_COMMAND:
    configState->TempConfig.AutoStart = (unsigned char)SendDlgItemMessage(hDlg, IDC_AUTOSTART, BM_GETCHECK, 0, 0);
    configState->TempConfig.CartAutoStart = (unsigned char)SendDlgItemMessage(hDlg, IDC_AUTOCART, BM_GETCHECK, 0, 0);

    break;
  }

  return(0);
}

LRESULT CALLBACK TapeConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  configState->CounterText.cbSize = sizeof(CHARFORMAT);
  configState->CounterText.dwMask = CFM_BOLD | CFM_COLOR;
  configState->CounterText.dwEffects = CFE_BOLD;
  configState->CounterText.crTextColor = RGB(255, 255, 255);

  configState->ModeText.cbSize = sizeof(CHARFORMAT);
  configState->ModeText.dwMask = CFM_BOLD | CFM_COLOR;
  configState->ModeText.dwEffects = CFE_BOLD;
  configState->ModeText.crTextColor = RGB(255, 0, 0);

  switch (message)
  {
  case WM_INITDIALOG:
    configState->TapeCounter = GetTapeCounter();

    sprintf(configState->OutBuffer, "%i", configState->TapeCounter);

    SendDlgItemMessage(hDlg, IDC_TCOUNT, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));
    SendDlgItemMessage(hDlg, IDC_MODE, WM_SETTEXT, strlen(configState->Tmodes[configState->Tmode]), (LPARAM)(LPCSTR)(configState->Tmodes[configState->Tmode]));

    GetTapeName(configState->TapeFileName);

    SendDlgItemMessage(hDlg, IDC_TAPEFILE, WM_SETTEXT, strlen(configState->TapeFileName), (LPARAM)(LPCSTR)(configState->TapeFileName));
    SendDlgItemMessage(hDlg, IDC_TCOUNT, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
    SendDlgItemMessage(hDlg, IDC_TCOUNT, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) & (configState->CounterText));
    SendDlgItemMessage(hDlg, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
    SendDlgItemMessage(hDlg, IDC_MODE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) & (configState->CounterText));

    configState->hDlgTape = hDlg;

    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_PLAY:
      configState->Tmode = PLAY;

      SetTapeMode(configState->Tmode);

      break;

    case IDC_REC:
      configState->Tmode = REC;

      SetTapeMode(configState->Tmode);

      break;

    case IDC_STOP:
      configState->Tmode = STOP;

      SetTapeMode(configState->Tmode);

      break;

    case IDC_EJECT:
      configState->Tmode = EJECT;

      SetTapeMode(configState->Tmode);

      break;

    case IDC_RESET:
      configState->TapeCounter = 0;

      SetTapeCounter(configState->TapeCounter);

      break;

    case IDC_TBROWSE:
      LoadTape();

      configState->TapeCounter = 0;

      SetTapeCounter(configState->TapeCounter);

      break;
    }

    break;
  }

  return(0);
}

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

LRESULT CALLBACK DisplayConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static bool isRGB;

  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:

    SendDlgItemMessage(hDlg, IDC_FRAMESKIP, TBM_SETRANGE, TRUE, MAKELONG(1, 6));
    SendDlgItemMessage(hDlg, IDC_SCANLINES, BM_SETCHECK, configState->TempConfig.ScanLines, 0);
    SendDlgItemMessage(hDlg, IDC_THROTTLE, BM_SETCHECK, configState->TempConfig.SpeedThrottle, 0);
    SendDlgItemMessage(hDlg, IDC_FRAMESKIP, TBM_SETPOS, TRUE, configState->TempConfig.FrameSkip);
    SendDlgItemMessage(hDlg, IDC_RESIZE, BM_SETCHECK, configState->TempConfig.Resize, 0);
    SendDlgItemMessage(hDlg, IDC_ASPECT, BM_SETCHECK, configState->TempConfig.Aspect, 0);
    SendDlgItemMessage(hDlg, IDC_REMEMBER_SIZE, BM_SETCHECK, configState->TempConfig.RememberSize, 0);

    sprintf(configState->OutBuffer, "%i", configState->TempConfig.FrameSkip);

    SendDlgItemMessage(hDlg, IDC_FRAMEDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));

    for (unsigned char temp = 0; temp <= 1; temp++) {
      SendDlgItemMessage(hDlg, configState->Monchoice[temp], BM_SETCHECK, (temp == configState->TempConfig.MonitorType), 0);
    }

    if (configState->TempConfig.MonitorType == 1) { //If RGB monitor is chosen, gray out palette choice
      isRGB = TRUE;

      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETDONTCLICK, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETDONTCLICK, 1, 0);
    }

    SendDlgItemMessage(hDlg, IDC_MONTYPE, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->MonIcons[configState->TempConfig.MonitorType]));

    for (unsigned char temp = 0; temp <= 1; temp++) {
      SendDlgItemMessage(hDlg, configState->PaletteChoice[temp], BM_SETCHECK, (temp == configState->TempConfig.PaletteType), 0);
    }

    break;

  case WM_HSCROLL:
    configState->TempConfig.FrameSkip = (unsigned char)SendDlgItemMessage(hDlg, IDC_FRAMESKIP, TBM_GETPOS, (WPARAM)0, (WPARAM)0);

    sprintf(configState->OutBuffer, "%i", configState->TempConfig.FrameSkip);

    SendDlgItemMessage(hDlg, IDC_FRAMEDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));

    break;

  case WM_COMMAND:
    configState->TempConfig.Resize = 1;
    configState->TempConfig.Aspect = (unsigned char)SendDlgItemMessage(hDlg, IDC_ASPECT, BM_GETCHECK, 0, 0);
    configState->TempConfig.ScanLines = (unsigned char)SendDlgItemMessage(hDlg, IDC_SCANLINES, BM_GETCHECK, 0, 0);
    configState->TempConfig.SpeedThrottle = (unsigned char)SendDlgItemMessage(hDlg, IDC_THROTTLE, BM_GETCHECK, 0, 0);
    configState->TempConfig.RememberSize = (unsigned char)SendDlgItemMessage(hDlg, IDC_REMEMBER_SIZE, BM_GETCHECK, 0, 0);

    //POINT p = { 640,480 };
    switch (LOWORD(wParam))
    {
    case IDC_REMEMBER_SIZE:
      configState->TempConfig.Resize = 1;

      SendDlgItemMessage(hDlg, IDC_RESIZE, BM_GETCHECK, 1, 0);

      break;

    case IDC_COMPOSITE:
      isRGB = FALSE;
      for (unsigned char temp = 0; temp <= 1; temp++) { //This finds the current Monitor choice, then sets both buttons in the nested loop.
        if (LOWORD(wParam) == configState->Monchoice[temp])
        {
          for (unsigned char temp2 = 0; temp2 <= 1; temp2++) {
            SendDlgItemMessage(hDlg, configState->Monchoice[temp2], BM_SETCHECK, 0, 0);
          }

          SendDlgItemMessage(hDlg, configState->Monchoice[temp], BM_SETCHECK, 1, 0);

          configState->TempConfig.MonitorType = temp;
        }
      }

      SendDlgItemMessage(hDlg, IDC_MONTYPE, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->MonIcons[configState->TempConfig.MonitorType]));
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 0, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 0, 0);
      break;

    case IDC_RGB:
      isRGB = TRUE;

      for (unsigned char temp = 0; temp <= 1; temp++) { //This finds the current Monitor choice, then sets both buttons in the nested loop.
        if (LOWORD(wParam) == configState->Monchoice[temp])
        {
          for (unsigned char temp2 = 0; temp2 <= 1; temp2++) {
            SendDlgItemMessage(hDlg, configState->Monchoice[temp2], BM_SETCHECK, 0, 0);
          }

          SendDlgItemMessage(hDlg, configState->Monchoice[temp], BM_SETCHECK, 1, 0);

          configState->TempConfig.MonitorType = temp;
        }
      }

      SendDlgItemMessage(hDlg, IDC_MONTYPE, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->MonIcons[configState->TempConfig.MonitorType]));
      //If RGB is chosen, disable palette buttons.
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETDONTCLICK, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETDONTCLICK, 1, 0);
      break;

    case IDC_ORG_PALETTE:
      if (!isRGB) {
        //Original Composite palette
        SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETCHECK, 0, 0);
        configState->TempConfig.PaletteType = 0;
      }
      else {
        SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 1, 0);
      }
      break;

    case IDC_UPD_PALETTE:
      if (!isRGB) {
        //New Composite palette
        SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETCHECK, 0, 0);
        configState->TempConfig.PaletteType = 1;
      }
      else {
        SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 1, 0);
      }
      break;
    }
    break;
  }

  return(0);
}

LRESULT CALLBACK InputConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:
    // copy keyboard layout names to the pull-down menu
    for (int x = 0; x < kKBLayoutCount; x++)
    {
      SendDlgItemMessage(hDlg, IDC_KBCONFIG, CB_ADDSTRING, (WPARAM)0, (LPARAM)k_keyboardLayoutNames[x]);
    }

    // select the current layout
    SendDlgItemMessage(hDlg, IDC_KBCONFIG, CB_SETCURSEL, (WPARAM)(configState->CurrentConfig.KeyMap), (LPARAM)0);
    break;

  case WM_COMMAND:
    configState->TempConfig.KeyMap = (unsigned char)SendDlgItemMessage(hDlg, IDC_KBCONFIG, CB_GETCURSEL, 0, 0);
    break;
  }

  return(0);
}

LRESULT CALLBACK JoyStickConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static int LeftJoyStick[6] = { IDC_LEFT_LEFT,IDC_LEFT_RIGHT,IDC_LEFT_UP,IDC_LEFT_DOWN,IDC_LEFT_FIRE1,IDC_LEFT_FIRE2 };
  static int RightJoyStick[6] = { IDC_RIGHT_LEFT,IDC_RIGHT_RIGHT,IDC_RIGHT_UP,IDC_RIGHT_DOWN,IDC_RIGHT_FIRE1,IDC_RIGHT_FIRE2 };
  static int LeftRadios[4] = { IDC_LEFT_KEYBOARD,IDC_LEFT_USEMOUSE,IDC_LEFTAUDIO,IDC_LEFTJOYSTICK };
  static int RightRadios[4] = { IDC_RIGHT_KEYBOARD,IDC_RIGHT_USEMOUSE,IDC_RIGHTAUDIO,IDC_RIGHTJOYSTICK };

  ConfigState* configState = GetConfigState();
  JoystickState* joystickState = GetJoystickState();

  extern SystemState EmuState;

  switch (message)
  {
  case WM_INITDIALOG:
    configState->JoystickIcons[0] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_KEYBOARD);
    configState->JoystickIcons[1] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_MOUSE);
    configState->JoystickIcons[2] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_AUDIO);
    configState->JoystickIcons[3] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_JOYSTICK);

    for (unsigned char temp = 0; temp < 68; temp++)
    {
      for (unsigned char temp2 = 0; temp2 < 6; temp2++)
      {
        SendDlgItemMessage(hDlg, LeftJoyStick[temp2], CB_ADDSTRING, (WPARAM)0, (LPARAM)GetKeyName(temp));
        SendDlgItemMessage(hDlg, RightJoyStick[temp2], CB_ADDSTRING, (WPARAM)0, (LPARAM)GetKeyName(temp));
      }
    }

    for (unsigned char temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, LeftJoyStick[temp]), (joystickState->Left.UseMouse == 0));
    }

    for (unsigned char temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, RightJoyStick[temp]), (joystickState->Right.UseMouse == 0));
    }

    for (unsigned char temp = 0; temp <= 2; temp++)
    {
      SendDlgItemMessage(hDlg, configState->LeftJoystickEmulation[temp], BM_SETCHECK, (temp == joystickState->Left.HiRes), 0);
      SendDlgItemMessage(hDlg, configState->RightJoystickEmulation[temp], BM_SETCHECK, (temp == joystickState->Right.HiRes), 0);
    }

    EnableWindow(GetDlgItem(hDlg, IDC_LEFTAUDIODEVICE), (joystickState->Left.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTAUDIODEVICE), (joystickState->Right.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_LEFTJOYSTICKDEVICE), (joystickState->Left.UseMouse == 3));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTJOYSTICKDEVICE), (joystickState->Right.UseMouse == 3));

    EnableWindow(GetDlgItem(hDlg, IDC_LEFTJOYSTICK), (configState->NumberofJoysticks > 0));		//Grey the Joystick Radios if
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTJOYSTICK), (configState->NumberofJoysticks > 0));	  //No Joysticks are present

    //populate joystick combo boxs
    for (unsigned char index = 0; index < configState->NumberofJoysticks; index++)
    {
      SendDlgItemMessage(hDlg, IDC_RIGHTJOYSTICKDEVICE, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetStickName(index));
      SendDlgItemMessage(hDlg, IDC_LEFTJOYSTICKDEVICE, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetStickName(index));
    }

    SendDlgItemMessage(hDlg, IDC_RIGHTJOYSTICKDEVICE, CB_SETCURSEL, (WPARAM)joystickState->Right.DiDevice, (LPARAM)0);
    SendDlgItemMessage(hDlg, IDC_LEFTJOYSTICKDEVICE, CB_SETCURSEL, (WPARAM)joystickState->Left.DiDevice, (LPARAM)0);

    SendDlgItemMessage(hDlg, LeftJoyStick[0], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Left.Left), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[1], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Left.Right), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[2], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Left.Up), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[3], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Left.Down), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[4], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Left.Fire1), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[5], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Left.Fire2), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[0], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Right.Left), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[1], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Right.Right), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[2], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Right.Up), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[3], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Right.Down), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[4], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Right.Fire1), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[5], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(joystickState->Right.Fire2), (LPARAM)0);

    for (unsigned char temp = 0; temp <= 3; temp++)
    {
      SendDlgItemMessage(hDlg, LeftRadios[temp], BM_SETCHECK, temp == joystickState->Left.UseMouse, 0);
    }

    for (unsigned char temp = 0; temp <= 3; temp++)
    {
      SendDlgItemMessage(hDlg, RightRadios[temp], BM_SETCHECK, temp == joystickState->Right.UseMouse, 0);
    }

    configState->Left = joystickState->Left;
    configState->Right = joystickState->Right;

    SendDlgItemMessage(hDlg, IDC_LEFTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->JoystickIcons[joystickState->Left.UseMouse]));
    SendDlgItemMessage(hDlg, IDC_RIGHTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->JoystickIcons[joystickState->Right.UseMouse]));
    break;

  case WM_COMMAND:
    for (unsigned char temp = 0; temp <= 3; temp++)
    {
      if (LOWORD(wParam) == LeftRadios[temp])
      {
        for (unsigned char temp2 = 0; temp2 <= 3; temp2++) {
          SendDlgItemMessage(hDlg, LeftRadios[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, LeftRadios[temp], BM_SETCHECK, 1, 0);

        configState->Left.UseMouse = temp;
      }
    }

    for (unsigned char temp = 0; temp <= 3; temp++) {
      if (LOWORD(wParam) == RightRadios[temp])
      {
        for (unsigned char temp2 = 0; temp2 <= 3; temp2++) {
          SendDlgItemMessage(hDlg, RightRadios[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, RightRadios[temp], BM_SETCHECK, 1, 0);

        configState->Right.UseMouse = temp;
      }
    }

    for (unsigned char temp = 0; temp <= 2; temp++) {
      if (LOWORD(wParam) == configState->LeftJoystickEmulation[temp])
      {
        for (unsigned char temp2 = 0; temp2 <= 2; temp2++) {
          SendDlgItemMessage(hDlg, configState->LeftJoystickEmulation[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, configState->LeftJoystickEmulation[temp], BM_SETCHECK, 1, 0);

        configState->Left.HiRes = temp;
      }
    }

    for (unsigned char temp = 0; temp <= 2; temp++)
    {
      if (LOWORD(wParam) == configState->RightJoystickEmulation[temp])
      {
        for (unsigned char temp2 = 0; temp2 <= 2; temp2++) {
          SendDlgItemMessage(hDlg, configState->RightJoystickEmulation[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, configState->RightJoystickEmulation[temp], BM_SETCHECK, 1, 0);

        configState->Right.HiRes = temp;
      }
    }

    for (unsigned char temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, LeftJoyStick[temp]), (configState->Left.UseMouse == 0));
    }

    for (unsigned char temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, RightJoyStick[temp]), (configState->Right.UseMouse == 0));
    }

    EnableWindow(GetDlgItem(hDlg, IDC_LEFTAUDIODEVICE), (configState->Left.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTAUDIODEVICE), (configState->Right.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_LEFTJOYSTICKDEVICE), (configState->Left.UseMouse == 3));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTJOYSTICKDEVICE), (configState->Right.UseMouse == 3));

    configState->Left.Left = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[0], CB_GETCURSEL, 0, 0));
    configState->Left.Right = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[1], CB_GETCURSEL, 0, 0));
    configState->Left.Up = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[2], CB_GETCURSEL, 0, 0));
    configState->Left.Down = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[3], CB_GETCURSEL, 0, 0));
    configState->Left.Fire1 = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[4], CB_GETCURSEL, 0, 0));
    configState->Left.Fire2 = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[5], CB_GETCURSEL, 0, 0));

    configState->Right.Left = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[0], CB_GETCURSEL, 0, 0));
    configState->Right.Right = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[1], CB_GETCURSEL, 0, 0));
    configState->Right.Up = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[2], CB_GETCURSEL, 0, 0));
    configState->Right.Down = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[3], CB_GETCURSEL, 0, 0));
    configState->Right.Fire1 = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[4], CB_GETCURSEL, 0, 0));
    configState->Right.Fire2 = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[5], CB_GETCURSEL, 0, 0));

    configState->Right.DiDevice = (unsigned char)SendDlgItemMessage(hDlg, IDC_RIGHTJOYSTICKDEVICE, CB_GETCURSEL, 0, 0);
    configState->Left.DiDevice = (unsigned char)SendDlgItemMessage(hDlg, IDC_LEFTJOYSTICKDEVICE, CB_GETCURSEL, 0, 0);	//Fix Me;

    SendDlgItemMessage(hDlg, IDC_LEFTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->JoystickIcons[configState->Left.UseMouse]));
    SendDlgItemMessage(hDlg, IDC_RIGHTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->JoystickIcons[configState->Right.UseMouse]));

    break;
  }

  return(0);
}

LRESULT CALLBACK BitBanger(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  extern SystemState EmuState;

  switch (message)
  {
  case WM_INITDIALOG:
    if (!strlen(configState->SerialCaptureFile)) {
      strcpy(configState->SerialCaptureFile, "No Capture File");
    }

    SendDlgItemMessage(hDlg, IDC_SERIALFILE, WM_SETTEXT, strlen(configState->SerialCaptureFile), (LPARAM)(LPCSTR)(configState->SerialCaptureFile));
    SendDlgItemMessage(hDlg, IDC_LF, BM_SETCHECK, configState->TextMode, 0);
    SendDlgItemMessage(hDlg, IDC_PRINTMON, BM_SETCHECK, configState->PrtMon, 0);
    break;

  case WM_COMMAND:

    switch (LOWORD(wParam))
    {
    case IDC_OPEN:
      SelectFile(&EmuState, configState->SerialCaptureFile);
      SendDlgItemMessage(hDlg, IDC_SERIALFILE, WM_SETTEXT, strlen(configState->SerialCaptureFile), (LPARAM)(LPCSTR)(configState->SerialCaptureFile));
      break;

    case IDC_CLOSE:
      ClosePrintFile();
      strcpy(configState->SerialCaptureFile, "No Capture File");

      SendDlgItemMessage(hDlg, IDC_SERIALFILE, WM_SETTEXT, strlen(configState->SerialCaptureFile), (LPARAM)(LPCSTR)(configState->SerialCaptureFile));

      configState->PrtMon = FALSE;
      SetMonState(configState->PrtMon);

      SendDlgItemMessage(hDlg, IDC_PRINTMON, BM_SETCHECK, configState->PrtMon, 0);

      break;

    case IDC_LF:
      configState->TextMode = (char)SendDlgItemMessage(hDlg, IDC_LF, BM_GETCHECK, 0, 0);

      SetSerialParams(configState->TextMode);

      break;

    case IDC_PRINTMON:
      configState->PrtMon = (char)SendDlgItemMessage(hDlg, IDC_PRINTMON, BM_GETCHECK, 0, 0);

      SetMonState(configState->PrtMon);
    }

    break;
  }

  return(0);
}


LRESULT CALLBACK Config(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static char tabTitles[TABS][10] = { "Audio","CPU","Display","Keyboard","Joysticks","Misc","Tape","BitBanger" };
  static unsigned char tabCount = 0, selectedTab = 0;
  static HWND hWndTabDialog;
  TCITEM tabs = TCITEM();

  ConfigState* configState = GetConfigState();
  JoystickState* joystickState = GetJoystickState();

  extern SystemState EmuState;

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
    configState->hWndConfig[7] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_BITBANGER), hWndTabDialog, (DLGPROC)BitBanger);

    //Set the title text for all tabs
    for (tabCount = 0;tabCount < TABS;tabCount++)
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
