#include "di.version.h"
#include <dinput.h>
#include <assert.h>

#include "Keyboard.h"
#include "keyboardlayout.h"
#include "Joystick.h"
#include "MC6821.h"

#include "TC1014Registers.h"
#include "xDebug.h"

KeyboardState* InitializeInstance(KeyboardState*);

static KeyboardState* instance = InitializeInstance(new KeyboardState());

//--Spelled funny because there's a GetKeyboardState() in User32.dll
extern "C" {
  __declspec(dllexport) KeyboardState* __cdecl GetKeyBoardState() {
    return instance;
  }
}

KeyboardState* InitializeInstance(KeyboardState* p) {
  p->KeyboardInterruptEnabled = 0;
  p->Pasting = false;

  return p;
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl GimeGetKeyboardInterruptState()
  {
    return instance->KeyboardInterruptEnabled;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GimeSetKeyboardInterruptState(unsigned char state)
  {
    instance->KeyboardInterruptEnabled = !!state;
  }
}

extern "C" {
  __declspec(dllexport) bool __cdecl GetPaste() {
    return instance->Pasting;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetPaste(bool flag) {
    instance->Pasting = flag;
  }
}

/**
  Key translation table compare function for sorting (with qsort)
*/
extern "C" {
  __declspec(dllexport) int __cdecl KeyTransCompare(const void* e1, const void* e2)
  {
    keytranslationentry_t* entry1 = (keytranslationentry_t*)e1;
    keytranslationentry_t* entry2 = (keytranslationentry_t*)e2;
    int result = 0;

    // empty listing push to end
    if (entry1->ScanCode1 == 0 && entry1->ScanCode2 == 0 && entry2->ScanCode1 != 0)
    {
      return 1;
    }
    else {
      if (entry2->ScanCode1 == 0 && entry2->ScanCode2 == 0 && entry1->ScanCode1 != 0)
      {
        return -1;
      }
      else {
        // push shift/alt/control by themselves to the end
        if (entry1->ScanCode2 == 0 && (entry1->ScanCode1 == DIK_LSHIFT || entry1->ScanCode1 == DIK_LMENU || entry1->ScanCode1 == DIK_LCONTROL))
        {
          result = 1;
        }
        else {
          // push shift/alt/control by themselves to the end
          if (entry2->ScanCode2 == 0 && (entry2->ScanCode1 == DIK_LSHIFT || entry2->ScanCode1 == DIK_LMENU || entry2->ScanCode1 == DIK_LCONTROL))
          {
            result = -1;
          }
          else {
            // move double key combos in front of single ones
            if (entry1->ScanCode2 == 0 && entry2->ScanCode2 != 0)
            {
              result = 1;
            }
            else {
              // move double key combos in front of single ones
              if (entry2->ScanCode2 == 0 && entry1->ScanCode2 != 0)
              {
                result = -1;
              }
              else
              {
                result = entry1->ScanCode1 - entry2->ScanCode1;

                if (result == 0)
                {
                  result = entry1->Row1 - entry2->Row1;
                }

                if (result == 0)
                {
                  result = entry1->Col1 - entry2->Col1;
                }

                if (result == 0)
                {
                  result = entry1->Row2 - entry2->Row2;
                }

                if (result == 0)
                {
                  result = entry1->Col2 - entry2->Col2;
                }
              }
            }
          }
        }
      }
    }

    return result;
  }
}

/*
  Rebuilds the run-time keyboard translation lookup table based on the
  current keyboard layout.

  The entries are sorted.  Any SHIFT + [char] entries need to be placed first
*/
extern "C" {
  __declspec(dllexport) void __cdecl vccKeyboardBuildRuntimeTable(keyboardlayout_e keyBoardLayout)
  {
    int index1 = 0;
    int index2 = 0;
    keytranslationentry_t* keyLayoutTable = NULL;
    keytranslationentry_t	keyTransEntry;

    assert(keyBoardLayout >= 0 && keyBoardLayout < kKBLayoutCount);

    switch (keyBoardLayout)
    {
    case kKBLayoutCoCo:
      keyLayoutTable = GetKeyTranslationsCoCo();
      break;

    case kKBLayoutNatural:
      keyLayoutTable = GetKeyTranslationsNatural();
      break;

    case kKBLayoutCompact:
      keyLayoutTable = GetKeyTranslationsCompact();
      break;

    case kKBLayoutCustom:
      keyLayoutTable = GetKeyTranslationsCustom();
      break;

    default:
      assert(0 && "unknown keyboard layout");
      break;
    }

    //XTRACE("Building run-time key table for layout # : %d - %s\n", keyBoardLayout, k_keyboardLayoutNames[keyBoardLayout]);

    // copy the selected keyboard layout to the run-time table
    memset(instance->KeyTransTable, 0, sizeof(instance->KeyTransTable));
    index2 = 0;

    for (index1 = 0; ; index1++)
    {
      memcpy(&keyTransEntry, &keyLayoutTable[index1], sizeof(keytranslationentry_t));

      //
      // Change entries to what the code expects
      //
      // Make sure ScanCode1 is never 0
      // If the key combo uses SHIFT, put it in ScanCode1
      // Completely clear unused entries (ScanCode1+2 are 0)
      //

      //
      // swaps ScanCode1 with ScanCode2 if ScanCode2 == DIK_LSHIFT
      //
      if (keyTransEntry.ScanCode2 == DIK_LSHIFT)
      {
        keyTransEntry.ScanCode2 = keyTransEntry.ScanCode1;
        keyTransEntry.ScanCode1 = DIK_LSHIFT;
      }

      //
      // swaps ScanCode1 with ScanCode2 if ScanCode1 is zero
      //
      if ((keyTransEntry.ScanCode1 == 0) && (keyTransEntry.ScanCode2 != 0))
      {
        keyTransEntry.ScanCode1 = keyTransEntry.ScanCode2;
        keyTransEntry.ScanCode2 = 0;
      }

      // check for terminating entry
      if (keyTransEntry.ScanCode1 == 0 && keyTransEntry.ScanCode2 == 0)
      {
        break;
      }

      memcpy(&(instance->KeyTransTable[index2++]), &keyTransEntry, sizeof(keytranslationentry_t));

      assert(index2 <= KBTABLE_ENTRY_COUNT && "keyboard layout table is longer than we can handle");
    }

    //
    // Sort the key translation table
    //
    // Since the table is searched from beginning to end each
    // time a key is pressed, we want them to be in the correct 
    // order.
    //
    qsort(instance->KeyTransTable, KBTABLE_ENTRY_COUNT, sizeof(keytranslationentry_t), KeyTransCompare);

#ifdef _DEBUG
    //
    // Debug dump the table
    //
    for (index1 = 0; index1 < KBTABLE_ENTRY_COUNT; index1++)
    {
      // check for null entry
      if (instance->KeyTransTable[index1].ScanCode1 == 0 && instance->KeyTransTable[index1].ScanCode2 == 0)
      {
        // done
        break;
      }

      //XTRACE("Key: %3d - 0x%02X (%3d) 0x%02X (%3d) - %2d %2d  %2d %2d\n",
      //  Index1,
      //  KeyTransTable[Index1].ScanCode1,
      //  KeyTransTable[Index1].ScanCode1,
      //  KeyTransTable[Index1].ScanCode2,
      //  KeyTransTable[Index1].ScanCode2,
      //  KeyTransTable[Index1].Row1,
      //  KeyTransTable[Index1].Col1,
      //  KeyTransTable[Index1].Row2,
      //  KeyTransTable[Index1].Col2
      //);
    }
#endif
  }
}

/*
  Get CoCo 'scan' code

  Only called from MC6821.c to read the keyboard/joystick state

  should be a push instead of a pull?
*/
extern "C" {
  __declspec(dllexport) unsigned char __cdecl vccKeyboardGetScan(unsigned char column)
  {
    unsigned char temp;
    unsigned char mask = 1;
    unsigned char ret_val = 0;

    JoystickState* joystickState = GetJoystickState();

    temp = ~column; //Get column

    for (unsigned char x = 0; x < 8; x++)
    {
      if ((temp & mask)) // Found an active column scan
      {
        ret_val |= instance->RolloverTable[x];
      }

      mask = (mask << 1);
    }

    ret_val = 127 - ret_val;

    //Collect CA2 and CB2 from the PIA (1of4 Multiplexer)
    joystickState->StickValue = get_pot_value(MC6821_GetMuxState());

    if (joystickState->StickValue != 0)		//OS9 joyin routine needs this (koronis rift works now)
    {
      if (joystickState->StickValue >= MC6821_DACState())		// Set bit of stick >= DAC output $FF20 Bits 7-2
      {
        ret_val |= 0x80;
      }
    }

    if (joystickState->LeftButton1Status == 1)
    {
      //Left Joystick Button 1 Down?
      ret_val = ret_val & 0xFD;
    }

    if (joystickState->RightButton1Status == 1)
    {
      //Right Joystick Button 1 Down?
      ret_val = ret_val & 0xFE;
    }

    if (joystickState->LeftButton2Status == 1)
    {
      //Left Joystick Button 2 Down?
      ret_val = ret_val & 0xF7;
    }

    if (joystickState->RightButton2Status == 1)
    {
      //Right Joystick Button 2 Down?
      ret_val = ret_val & 0xFB;
    }

#if 0 // no noticible change when this is disabled
    // TODO: move to MC6821/GIME
    {
      /** another keyboard IRQ flag - this should really be in the GIME code*/
      static unsigned char IrqFlag = 0;
      if ((ret_val & 0x7F) != 0x7F)
      {
        if ((IrqFlag == 0) & GimeGetKeyboardInterruptState())
        {
          GimeAssertKeyboardInterrupt();
          IrqFlag = 1;
        }
      }
      else
      {
        IrqFlag = 0;
      }
    }
#endif

    return (ret_val);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl vccKeyboardUpdateRolloverTable()
  {
    unsigned char	lockOut = 0;

    // clear the rollover table
    for (int index = 0; index < 8; index++)
    {
      instance->RolloverTable[index] = 0;
    }

    // set rollover table based on ScanTable key status
    for (int index = 0; index < KBTABLE_ENTRY_COUNT; index++)
    {
      // stop at last entry
      if ((instance->KeyTransTable[index].ScanCode1 == 0) && (instance->KeyTransTable[index].ScanCode2 == 0))
      {
        break;
      }

      if (lockOut != instance->KeyTransTable[index].ScanCode1)
      {
        // Single input key 
        if ((instance->KeyTransTable[index].ScanCode1 != 0) && (instance->KeyTransTable[index].ScanCode2 == 0))
        {
          // check if key pressed
          if (instance->ScanTable[instance->KeyTransTable[index].ScanCode1] == KEY_DOWN)
          {
            int col = instance->KeyTransTable[index].Col1;

            assert(col >= 0 && col < 8);

            instance->RolloverTable[col] |= instance->KeyTransTable[index].Row1;

            col = instance->KeyTransTable[index].Col2;

            assert(col >= 0 && col < 8);

            instance->RolloverTable[col] |= instance->KeyTransTable[index].Row2;
          }
        }

        // Double Input Key
        if ((instance->KeyTransTable[index].ScanCode1 != 0) && (instance->KeyTransTable[index].ScanCode2 != 0))
        {
          // check if both keys pressed
          if ((instance->ScanTable[instance->KeyTransTable[index].ScanCode1] == KEY_DOWN) && (instance->ScanTable[instance->KeyTransTable[index].ScanCode2] == KEY_DOWN))
          {
            int col = instance->KeyTransTable[index].Col1;

            assert(col >= 0 && col < 8);

            instance->RolloverTable[col] |= instance->KeyTransTable[index].Row1;

            col = instance->KeyTransTable[index].Col2;

            assert(col >= 0 && col < 8);

            instance->RolloverTable[col] |= instance->KeyTransTable[index].Row2;

            // always SHIFT
            lockOut = instance->KeyTransTable[index].ScanCode1;

            break;
          }
        }
      }
    }
  }
}

/*
  Dispatch keyboard event to the emulator.

  Called from system. eg. WndProc : WM_KEYDOWN/WM_SYSKEYDOWN/WM_SYSKEYUP/WM_KEYUP

  @param key Windows virtual key code (VK_XXXX - not used)
  @param ScanCode keyboard scan code (DIK_XXXX - DirectInput)
  @param Status Key status - kEventKeyDown/kEventKeyUp
*/
extern "C" {
  __declspec(dllexport) void __cdecl vccKeyboardHandleKey(unsigned char key, unsigned char scanCode, keyevent_e keyState)
  {
    XTRACE("Key  : %c (%3d / 0x%02X)  Scan : %d / 0x%02X\n", key == 0 ? '0' : key, key == 0 ? '0' : key, key == 0 ? '0' : key, scanCode, scanCode);

    JoystickState* joystickState = GetJoystickState();

    //If requested, abort pasting operation.
    if (scanCode == 0x01 || scanCode == 0x43 || scanCode == 0x3F) {
      instance->Pasting = false;

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
      instance->ScanTable[scanCode] = KEY_DOWN;

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
      instance->ScanTable[scanCode] = KEY_UP;

      // TODO: verify this is accurate emulation
      // Clean out rollover table on shift release
      if (scanCode == DIK_LSHIFT)
      {
        for (int Index = 0; Index < KBTABLE_ENTRY_COUNT; Index++)
        {
          instance->ScanTable[Index] = KEY_UP;
        }
      }

      vccKeyboardUpdateRolloverTable();

      break;
    }
  }
}
