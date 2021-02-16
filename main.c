#include "library/di.version.h"

#include <windows.h>
#include <process.h>
#include <dinput.h>

#include "vccdef.h"
#include "vccstate.h"
#include "resource.h"
#include "pakinterfacedef.h"

#include "About.h"
#include "AudioAccessors.h"
#include "ConfigAccessors.h"
#include "CopyText.h"
#include "CreateDDWindow.h"
#include "DecreaseOverclockSpeed.h"
#include "DirectDrawAccessors.h"
#include "DynamicMenuActivated.h"
#include "DynamicMenuCallback.h"
#include "EmuLoop.h"
#include "FullScreenToggle.h"
#include "IncreaseOverclockSpeed.h"
#include "InitInstance.h"
#include "InvalidateBorder.h "
#include "JoystickAccessors.h"
#include "LoadConfig.h"
#include "LoadIniFile.h"
#include "MainConfig.h"
#include "PakInterfaceAccessors.h"
#include "PasteBASIC.h"
#include "PasteBASICWithNew.h"
#include "PasteText.h"
#include "SaveConfig.h"
#include "SaveLastTwoKeyDownEvents.h"
#include "SendSavedKeyEvents.h"
#include "SetClockSpeed.h"
#include "SetMonitorType.h"
#include "SetSpeedThrottle.h"
#include "UnloadDll.h"
#include "WriteIniFile.h"
#include "vccKeyboardHandleKey.h"

#include "library\fileoperations.h"
#include "library\graphicsstate.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ INT nCmdShow) {
  MSG  msg;
  HANDLE hEvent;
  HANDLE OleInitialize(NULL); //Work around fixs app crashing in "Open file" system dialogs (related to Adobe acrobat 7+
  char temp1[MAX_PATH] = "";
  char temp2[MAX_PATH] = " Running on ";
  unsigned threadID;

  VccState* vccState = GetVccState();

  vccState->EmuState.WindowInstance = hInstance;

  LoadString(hInstance, IDS_APP_TITLE, vccState->AppName, MAX_LOADSTRING);

  GetCmdLineArgs(lpCmdLine, &(vccState->CmdArg)); //Parse command line

  if (strlen(vccState->CmdArg.QLoadFile) != 0)
  {
    strcpy(vccState->QuickLoadFile, vccState->CmdArg.QLoadFile);
    strcpy(temp1, vccState->CmdArg.QLoadFile);

    FilePathStripPath(temp1);

    _strlwr(temp1);

    temp1[0] = toupper(temp1[0]);

    strcat(temp1, temp2);
    strcat(temp1, vccState->AppName);
    strcpy(vccState->AppName, temp1);
  }

  vccState->EmuState.WindowSize.x = 640;
  vccState->EmuState.WindowSize.y = 480;

  LoadConfig(&(vccState->EmuState), vccState->CmdArg);
  InitInstance(hInstance, nCmdShow);

  if (!CreateDDWindow(&(vccState->EmuState)))
  {
    MessageBox(0, "Can't create primary Window", "Error", 0);

    exit(0);
  }

  Cls(0, &(vccState->EmuState));
  DynamicMenuCallback(&(vccState->EmuState), "", 0, 0);
  DynamicMenuCallback(&(vccState->EmuState), "", 1, 0);
  LoadConfig(&(vccState->EmuState), vccState->CmdArg);			//Loads the default config file Vcc.ini from the exec directory

  vccState->EmuState.ResetPending = 2;

  SetClockSpeed(1);	//Default clock speed .89 MHZ	

  vccState->BinaryRunning = true;
  vccState->EmuState.EmulationRunning = vccState->AutoStart;

  if (strlen(vccState->CmdArg.QLoadFile) != 0)
  {
    vccState->Qflag = 255;
    vccState->EmuState.EmulationRunning = 1;
  }

  hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  if (hEvent == NULL)
  {
    MessageBox(0, "Can't create Thread!!", "Error", 0);

    return(0);
  }

  vccState->hEMUThread = (HANDLE)_beginthreadex(NULL, 0, &EmuLoop, hEvent, 0, &threadID);

  if (vccState->hEMUThread == NULL)
  {
    MessageBox(0, "Can't Start main Emulation Thread!", "Ok", 0);

    return(0);
  }

  WaitForSingleObject(hEvent, INFINITE);
  SetThreadPriority(vccState->hEMUThread, THREAD_PRIORITY_NORMAL);

  while (vccState->BinaryRunning)
  {
    if (vccState->FlagEmuStop == TH_WAITING)		//Need to stop the EMU thread for screen mode change
    {								                  //As it holds the Secondary screen buffer open while running
      FullScreenToggle();

      vccState->FlagEmuStop = TH_RUNNING;
    }

    GetMessage(&msg, NULL, 0, 0);		//Seems if the main loop stops polling for Messages the child threads stall

    TranslateMessage(&msg);

    DispatchMessage(&msg);
  }

  CloseHandle(hEvent);
  CloseHandle(vccState->hEMUThread);
  timeEndPeriod(1);
  UnloadDll(&(vccState->EmuState));
  SoundDeInit();
  WriteIniFile(); //Save Any changes to ini File

  return (INT)msg.wParam;
}

/*--------------------------------------------------------------------------*/
// The Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId, wmEvent;
  unsigned int x, y;
  unsigned char kb_char = (unsigned char)wParam;
  static unsigned char OEMscan = 0;
  static char ascii = 0;
  static RECT clientSize;
  static unsigned long width = 0, height = 0;

  VccState* vccState = GetVccState();

  switch (message)
  {

  case WM_SYSCOMMAND:
    //-------------------------------------------------------------
    // Control ATL key menu access.
    // Here left ALT is hardcoded off and right ALT on
    // TODO: Add config check boxes to control them
    //-------------------------------------------------------------
    if (wParam == SC_KEYMENU) {
      if (GetKeyState(VK_LMENU) & 0x8000) return 0; // Left off
    }
    // falls through to WM_COMMAND

  case WM_COMMAND:
    // Force all keys up to prevent key repeats
    SendSavedKeyEvents();

    wmId = LOWORD(wParam);
    wmEvent = HIWORD(wParam);

    // Parse the menu selections:
    // Added for Dynamic menu system
    if ((wmId >= ID_SDYNAMENU) && (wmId <= ID_EDYNAMENU))
    {
      DynamicMenuActivated(&(vccState->EmuState), wmId - ID_SDYNAMENU);	//Calls to the loaded DLL so it can do the right thing
      break;
    }

    switch (wmId)
    {
    case IDM_HELP_ABOUT:
      DialogBox(vccState->EmuState.WindowInstance, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
      break;

    case ID_CONFIGURE_OPTIONS:
#ifdef CONFIG_DIALOG_MODAL
      // open config dialog modally
      DialogBox(EmuState.WindowInstance, (LPCTSTR)IDD_TCONFIG, hWnd, (DLGPROC)Config);
#else

      // open config dialog if not already open
      // opens modeless so you can control the cassette
      // while emulator is still running (assumed)
      if (vccState->EmuState.ConfigDialog == NULL)
      {
        vccState->EmuState.ConfigDialog = CreateDialog(vccState->EmuState.WindowInstance, (LPCTSTR)IDD_TCONFIG, vccState->EmuState.WindowHandle, (DLGPROC)MainConfig);

        // open modeless
        ShowWindow(vccState->EmuState.ConfigDialog, SW_SHOWNORMAL);
      }
#endif
      break;

    case IDOK:
      SendMessage(hWnd, WM_CLOSE, 0, 0);

      break;

    case ID_FILE_EXIT:
      vccState->BinaryRunning = 0;

      break;

    case ID_FILE_RESET:
      if (vccState->EmuState.EmulationRunning) {
        vccState->EmuState.ResetPending = 2;
      }

      break;

    case ID_FILE_RUN:
      vccState->EmuState.EmulationRunning = TRUE;

      InvalidateBorder();

      break;

    case ID_FILE_RESET_SFT:
      if (vccState->EmuState.EmulationRunning) {
        vccState->EmuState.ResetPending = 1;
      }

      break;

    case ID_FILE_LOAD:
      LoadIniFile();

      break;

    case ID_SAVE_CONFIG:
      SaveConfig();

      break;

    case ID_COPY_TEXT:
      CopyText();

      break;

    case ID_PASTE_TEXT:
      PasteText();

      break;

    case ID_PASTE_BASIC:
      PasteBASIC();

      break;

    case ID_PASTE_BASIC_NEW:
      PasteBASICWithNew();

      break;

    case ID_FLIP_ARTIFACTS:
      FlipArtifacts();

      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }

    break;

  case WM_KILLFOCUS:
    // Force keys up if main widow keyboard focus is lost.  Otherwise
    // down keys will cause issues with OS-9 on return
    SendSavedKeyEvents();

    break;

  case WM_CLOSE:
    vccState->BinaryRunning = 0;

    break;

  case WM_CHAR:
    return 0;

  case WM_SYSCHAR:
    DefWindowProc(hWnd, message, wParam, lParam);

    return 0;

  case WM_KEYUP:
  case WM_SYSKEYUP:
    // send emulator key up event to the emulator
    // TODO: Key up checks whether the emulation is running, this does not

    OEMscan = (unsigned char)((lParam & 0x00FF0000) >> 16);

    vccKeyboardHandleKey(kb_char, OEMscan, kEventKeyUp);

    return 0;

    //----------------------------------------------------------------------------------------
    //	lParam bits
    //	  0-15	The repeat count for the current message. The value is the number of times
    //			the keystroke is autorepeated as a result of the user holding down the key.
    //			If the keystroke is held long enough, multiple messages are sent. However,
    //			the repeat count is not cumulative.
    //	 16-23	The scan code. The value depends on the OEM.
    //	    24	Indicates whether the key is an extended key, such as the right-hand ALT and
    //			CTRL keys that appear on an enhanced 101- or 102-key keyboard. The value is
    //			one if it is an extended key; otherwise, it is zero.
    //	 25-28	Reserved; do not use.
    //	    29	The context code. The value is always zero for a WM_KEYDOWN message.
    //	    30	The previous key state. The value is one if the key is down before the
    //	   		message is sent, or it is zero if the key is up.
    //	    31	The transition state. The value is always zero for a WM_KEYDOWN message.
    //----------------------------------------------------------------------------------------

  case WM_SYSKEYDOWN:
    // Ignore repeated system keys
    if (lParam >> 30) {
      return 0;
    }

  case WM_KEYDOWN:
    // get key scan code for emulator control keys
    OEMscan = (unsigned char)((lParam & 0x00FF0000) >> 16); // just get the scan code

    switch (OEMscan)
    {
    case DIK_F3:
      DecreaseOverclockSpeed(&(vccState->EmuState));
      break;

    case DIK_F4:
      IncreaseOverclockSpeed(&(vccState->EmuState));
      break;

    case DIK_F5:
      if (vccState->EmuState.EmulationRunning) {
        vccState->EmuState.ResetPending = 1;
      }
      break;

    case DIK_F6:
      SetMonitorType(!SetMonitorType(QUERY));
      break;

    case DIK_F8:
      SetSpeedThrottle(!SetSpeedThrottle(QUERY));
      break;

    case DIK_F9:
      vccState->EmuState.EmulationRunning = !vccState->EmuState.EmulationRunning;

      if (vccState->EmuState.EmulationRunning) {
        vccState->EmuState.ResetPending = 2;
      }
      else {
        SetStatusBarText("", &(vccState->EmuState));
      }

      break;

    case DIK_F10:
      SetInfoBand(!SetInfoBand(QUERY));
      InvalidateBorder();

      break;

    case DIK_F11:
      if (vccState->FlagEmuStop == TH_RUNNING)
      {
        vccState->FlagEmuStop = TH_REQWAIT;
        vccState->EmuState.FullScreen = !vccState->EmuState.FullScreen;
      }

      break;

    default:
      // send other keystrokes to the emulator if it is active
      if (vccState->EmuState.EmulationRunning)
      {
        vccKeyboardHandleKey(kb_char, OEMscan, kEventKeyDown);

        // Save key down in case focus is lost
        SaveLastTwoKeyDownEvents(kb_char, OEMscan);
      }
      break;
    }

    return 0;

  case WM_LBUTTONDOWN:  //0 = Left 1=right
    SetButtonStatus(0, 1);
    break;

  case WM_LBUTTONUP:
    SetButtonStatus(0, 0);
    break;

  case WM_RBUTTONDOWN:
    SetButtonStatus(1, 1);
    break;

  case WM_RBUTTONUP:
    SetButtonStatus(1, 0);
    break;

  case WM_MOUSEMOVE:
    if (vccState->EmuState.EmulationRunning)
    {
      x = LOWORD(lParam);
      y = HIWORD(lParam);

      GetClientRect(vccState->EmuState.WindowHandle, &clientSize);

      x /= ((clientSize.right - clientSize.left) >> 6);
      y /= (((clientSize.bottom - clientSize.top) - 20) >> 6);

      SetJoystick(x, y);
    }

    return(0);
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}
