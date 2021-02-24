#include <windows.h>

#include "../resources/resource.h"

#include "VCC.h"
#include "PAKInterface.h"
#include "Clipboard.h"
#include "Graphics.h"

#include "MessageHandlers.h"

extern "C" {
  __declspec(dllexport) void __cdecl ProcessCommandMessage(HWND hWnd, WPARAM wParam) {
    int wmId, wmEvent;

    VccState* vccState = GetVccState();

    // Force all keys up to prevent key repeats
    SendSavedKeyEvents();

    wmId = LOWORD(wParam);
    wmEvent = HIWORD(wParam);

    // Parse the menu selections:
    // Added for Dynamic menu system
    if ((wmId >= ID_SDYNAMENU) && (wmId <= ID_EDYNAMENU))
    {
      DynamicMenuActivated(&(vccState->SystemState), wmId - ID_SDYNAMENU);	//Calls to the loaded DLL so it can do the right thing
      return;
    }

    switch (wmId)
    {
    case IDM_HELP_ABOUT:
      HelpAbout(hWnd);
      break;

    case ID_CONFIGURE_OPTIONS:
      ShowConfiguration();
      break;

    case IDOK:
      SendMessage(hWnd, WM_CLOSE, 0, 0);

      break;

    case ID_FILE_EXIT:
      EmuExit();
      break;

    case ID_FILE_RESET:
      EmuReset(2);

      break;

    case ID_FILE_RUN:
      EmuRun();
      break;

    case ID_FILE_RESET_SFT:
      EmuReset(1);
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
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ProcessSysCommandMessage(HWND hWnd, WPARAM wParam) {
    //-------------------------------------------------------------
    // Control ATL key menu access.
    // Here left ALT is hardcoded off and right ALT on
    // TODO: Add config check boxes to control them
    //-------------------------------------------------------------
    if ((wParam != SC_KEYMENU) || (!(GetKeyState(VK_LMENU) & 0x8000))) {
      ProcessCommandMessage(hWnd, wParam);
    }
  }
}
