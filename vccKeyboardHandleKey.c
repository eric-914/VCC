#include "library/di.version.h"

#include <windows.h>
#include <dinput.h>

#include "keyboardstate.h"
#include "KeyboardAccessors.h"
#include "JoystickAccessors.h"
#include "vccKeyboardUpdateRolloverTable.h"

#include "library/joystickstate.h"
#include "library/keyboarddef.h"
#include "library/xDebug.h"

extern void GimeAssertKeyboardInterrupt(void);

/*
  Dispatch keyboard event to the emulator.

  Called from system. eg. WndProc : WM_KEYDOWN/WM_SYSKEYDOWN/WM_SYSKEYUP/WM_KEYUP

  @param key Windows virtual key code (VK_XXXX - not used)
  @param ScanCode keyboard scan code (DIK_XXXX - DirectInput)
  @param Status Key status - kEventKeyDown/kEventKeyUp
*/
extern "C" {
  void vccKeyboardHandleKey(unsigned char key, unsigned char scanCode, keyevent_e keyState)
  {
    XTRACE("Key  : %c (%3d / 0x%02X)  Scan : %d / 0x%02X\n", key == 0 ? '0' : key, key == 0 ? '0' : key, key == 0 ? '0' : key, scanCode, scanCode);

    KeyboardState* keyboardState = GetKeyBoardState();
    JoystickState* joystickState = GetJoystickState();

    //If requested, abort pasting operation.
    if (scanCode == 0x01 || scanCode == 0x43 || scanCode == 0x3F) {
      keyboardState->Pasting = false;

      OutputDebugString("ABORT PASTING!!!\n");
    }

    // check for shift key
    // Left and right shift generate different scan codes
    if (scanCode == DIK_RSHIFT)
    {
      scanCode = DIK_LSHIFT;
    }

#if 0 // TODO: CTRL and/or ALT?
    // CTRL key - right -> left
    if (ScanCode == DIK_RCONTROL)
    {
      ScanCode = DIK_LCONTROL;
    }
    // ALT key - right -> left
    if (ScanCode == DIK_RMENU)
    {
      ScanCode = DIK_LMENU;
    }
#endif

    switch (keyState)
    {
    default:
      // internal error
      break;

      // Key Down
    case kEventKeyDown:
      if ((joystickState->Left.UseMouse == 0) || (joystickState->Right.UseMouse == 0))
      {
        scanCode = SetMouseStatus(scanCode, 1);
      }

      // track key is down
      keyboardState->ScanTable[scanCode] = KEY_DOWN;

      vccKeyboardUpdateRolloverTable();

      if (GimeGetKeyboardInterruptState())
      {
        GimeAssertKeyboardInterrupt();
      }

      break;

      // Key Up
    case kEventKeyUp:
      if ((joystickState->Left.UseMouse == 0) || (joystickState->Right.UseMouse == 0))
      {
        scanCode = SetMouseStatus(scanCode, 0);
      }

      // reset key (released)
      keyboardState->ScanTable[scanCode] = KEY_UP;

      // TODO: verify this is accurate emulation
      // Clean out rollover table on shift release
      if (scanCode == DIK_LSHIFT)
      {
        for (int Index = 0; Index < KBTABLE_ENTRY_COUNT; Index++)
        {
          keyboardState->ScanTable[Index] = KEY_UP;
        }
      }

      vccKeyboardUpdateRolloverTable();

      break;
    }
  }
}
