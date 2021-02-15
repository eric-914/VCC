/*****************************************************************************/
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
/*****************************************************************************/
/*
  Keyboard handling / translation - system -> emulator

  TODO: move joystick code out of here
  TODO: any key(s) that results in a multi-key (eg. SHIFT/ALT/CTRL)
          combination on the CoCo side is only sending one interrupt
      signal for both keys.  This seems to work in virtually all cases,
      but is it an accurate emulation?
*/
/*****************************************************************************/

#include "library/keyboarddef.h"

// this must be before defines.h as it contains Windows types and not Windows.h include
#include <windows.h>
#include <dinput.h>
#include <assert.h>

#include "keyboardstate.h"
#include "keyboard.h"
#include "tcc1014registers.h" //GimeAssertKeyboardInterrupt();
#include "mc6821.h" //GetMuxState() //DACState()

#include "library/keyboardlayout.h"
#include "library/joystickinput.h"
#include "library/joystickstate.h"
#include "library/xDebug.h"


char SetMouseStatus(char, unsigned char);

unsigned char GimeGetKeyboardInterruptState()
{
  KeyboardState* keyboardState = GetKeyBoardState();

  return keyboardState->KeyboardInterruptEnabled;
}

void GimeSetKeyboardInterruptState(unsigned char state)
{
  KeyboardState* keyboardState = GetKeyBoardState();

  keyboardState->KeyboardInterruptEnabled = !!state;
}

/*
  Get CoCo 'scan' code

  Only called from MC6821.c to read the keyboard/joystick state

  should be a push instead of a pull?
*/
unsigned char vccKeyboardGetScan(unsigned char column)
{
  unsigned char temp;
  unsigned char mask = 1;
  unsigned char ret_val = 0;

  KeyboardState* keyboardState = GetKeyBoardState();

  temp = ~column; //Get column

  for (unsigned char x = 0; x < 8; x++)
  {
    if ((temp & mask)) // Found an active column scan
    {
      ret_val |= keyboardState->RolloverTable[x];
    }

    mask = (mask << 1);
  }

  ret_val = 127 - ret_val;

  JoystickState* joystickState = GetJoystickState();

  //Collect CA2 and CB2 from the PIA (1of4 Multiplexer)
  joystickState->StickValue = get_pot_value(GetMuxState());

  if (joystickState->StickValue != 0)		//OS9 joyin routine needs this (koronis rift works now)
  {
    if (joystickState->StickValue >= DACState())		// Set bit of stick >= DAC output $FF20 Bits 7-2
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

void _vccKeyboardUpdateRolloverTable()
{
  unsigned char	lockOut = 0;

  KeyboardState* keyboardState = GetKeyBoardState();

  // clear the rollover table
  for (int index = 0; index < 8; index++)
  {
    keyboardState->RolloverTable[index] = 0;
  }

  // set rollover table based on ScanTable key status
  for (int index = 0; index < KBTABLE_ENTRY_COUNT; index++)
  {
    // stop at last entry
    if ((keyboardState->KeyTransTable[index].ScanCode1 == 0) && (keyboardState->KeyTransTable[index].ScanCode2 == 0))
    {
      break;
    }

    if (lockOut != keyboardState->KeyTransTable[index].ScanCode1)
    {
      // Single input key 
      if ((keyboardState->KeyTransTable[index].ScanCode1 != 0) && (keyboardState->KeyTransTable[index].ScanCode2 == 0))
      {
        // check if key pressed
        if (keyboardState->ScanTable[keyboardState->KeyTransTable[index].ScanCode1] == KEY_DOWN)
        {
          int col = keyboardState->KeyTransTable[index].Col1;

          assert(col >= 0 && col < 8);

          keyboardState->RolloverTable[col] |= keyboardState->KeyTransTable[index].Row1;

          col = keyboardState->KeyTransTable[index].Col2;

          assert(col >= 0 && col < 8);

          keyboardState->RolloverTable[col] |= keyboardState->KeyTransTable[index].Row2;
        }
      }

      // Double Input Key
      if ((keyboardState->KeyTransTable[index].ScanCode1 != 0) && (keyboardState->KeyTransTable[index].ScanCode2 != 0))
      {
        // check if both keys pressed
        if ((keyboardState->ScanTable[keyboardState->KeyTransTable[index].ScanCode1] == KEY_DOWN) && (keyboardState->ScanTable[keyboardState->KeyTransTable[index].ScanCode2] == KEY_DOWN))
        {
          int col = keyboardState->KeyTransTable[index].Col1;

          assert(col >= 0 && col < 8);

          keyboardState->RolloverTable[col] |= keyboardState->KeyTransTable[index].Row1;

          col = keyboardState->KeyTransTable[index].Col2;

          assert(col >= 0 && col < 8);

          keyboardState->RolloverTable[col] |= keyboardState->KeyTransTable[index].Row2;

          // always SHIFT
          lockOut = keyboardState->KeyTransTable[index].ScanCode1;

          break;
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
void vccKeyboardHandleKey(unsigned char key, unsigned char scanCode, keyevent_e keyState)
{
  XTRACE("Key  : %c (%3d / 0x%02X)  Scan : %d / 0x%02X\n", key == 0 ? '0' : key, key == 0 ? '0' : key, key == 0 ? '0' : key, scanCode, scanCode);

  KeyboardState* keyboardState = GetKeyBoardState();

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

  JoystickState* joystickState = GetJoystickState();

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

    _vccKeyboardUpdateRolloverTable();

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

    _vccKeyboardUpdateRolloverTable();

    break;
  }
}

/**
  Key translation table compare function for sorting (with qsort)
*/
int keyTransCompare(const void* e1, const void* e2)
{
  keytranslationentry_t* entry1 = (keytranslationentry_t*)e1;
  keytranslationentry_t* entry2 = (keytranslationentry_t*)e2;
  int result = 0;

  KeyboardState* keyboardState = GetKeyBoardState();

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

/*
  Rebuilds the run-time keyboard translation lookup table based on the
  current keyboard layout.

  The entries are sorted.  Any SHIFT + [char] entries need to be placed first
*/
void vccKeyboardBuildRuntimeTable(keyboardlayout_e keyBoardLayout)
{
  int index1 = 0;
  int index2 = 0;
  keytranslationentry_t* keyLayoutTable = NULL;
  keytranslationentry_t	keyTransEntry;

  KeyboardState* keyboardState = GetKeyBoardState();

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
  memset(keyboardState->KeyTransTable, 0, sizeof(keyboardState->KeyTransTable));
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

    memcpy(&(keyboardState->KeyTransTable[index2++]), &keyTransEntry, sizeof(keytranslationentry_t));

    assert(index2 <= KBTABLE_ENTRY_COUNT && "keyboard layout table is longer than we can handle");
  }

  //
  // Sort the key translation table
  //
  // Since the table is searched from beginning to end each
  // time a key is pressed, we want them to be in the correct 
  // order.
  //
  qsort(keyboardState->KeyTransTable, KBTABLE_ENTRY_COUNT, sizeof(keytranslationentry_t), keyTransCompare);

#ifdef _DEBUG
  //
  // Debug dump the table
  //
  for (index1 = 0; index1 < KBTABLE_ENTRY_COUNT; index1++)
  {
    // check for null entry
    if (keyboardState->KeyTransTable[index1].ScanCode1 == 0 && keyboardState->KeyTransTable[index1].ScanCode2 == 0)
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

/*
  Joystick related code
*/

void joystick(unsigned short x, unsigned short y)
{
  KeyboardState* keyboardState = GetKeyBoardState();

  if (x > 63) {
    x = 63;
  }

  if (y > 63) {
    y = 63;
  }

  JoystickState* joystickState = GetJoystickState();

  if (joystickState->Left.UseMouse == 1)
  {
    joystickState->LeftStickX = x;
    joystickState->LeftStickY = y;
  }

  if (joystickState->Right.UseMouse == 1)
  {
    joystickState->RightStickX = x;
    joystickState->RightStickY = y;
  }

  return;
}

void SetStickNumbers(unsigned char leftStickNumber, unsigned char rightStickNumber)
{
  JoystickState* joystickState = GetJoystickState();

  joystickState->LeftStickNumber = leftStickNumber;
  joystickState->RightStickNumber = rightStickNumber;
}

unsigned short get_pot_value(unsigned char pot)
{
  DIJOYSTATE2 Stick1;

  KeyboardState* keyboardState = GetKeyBoardState();
  JoystickState* joystickState = GetJoystickState();

  if (joystickState->Left.UseMouse == 3)
  {
    JoyStickPoll(&Stick1, joystickState->LeftStickNumber);
    joystickState->LeftStickX = (unsigned short)Stick1.lX >> 10;
    joystickState->LeftStickY = (unsigned short)Stick1.lY >> 10;
    joystickState->LeftButton1Status = Stick1.rgbButtons[0] >> 7;
    joystickState->LeftButton2Status = Stick1.rgbButtons[1] >> 7;
  }

  if (joystickState->Right.UseMouse == 3)
  {
    JoyStickPoll(&Stick1, joystickState->RightStickNumber);
    joystickState->RightStickX = (unsigned short)Stick1.lX >> 10;
    joystickState->RightStickY = (unsigned short)Stick1.lY >> 10;
    joystickState->RightButton1Status = Stick1.rgbButtons[0] >> 7;
    joystickState->RightButton2Status = Stick1.rgbButtons[1] >> 7;
  }

  switch (pot)
  {
  case 0:
    return(joystickState->RightStickX);
    break;

  case 1:
    return(joystickState->RightStickY);
    break;

  case 2:
    return(joystickState->LeftStickX);
    break;

  case 3:
    return(joystickState->LeftStickY);
    break;
  }

  return (0);
}

char SetMouseStatus(char scanCode, unsigned char phase)
{
  char retValue = scanCode;

  KeyboardState* keyboardState = GetKeyBoardState();
  JoystickState* joystickState = GetJoystickState();

  switch (phase)
  {
  case 0:
    if (joystickState->Left.UseMouse == 0)
    {
      if (scanCode == joystickState->Left.Left)
      {
        joystickState->LeftStickX = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Right)
      {
        joystickState->LeftStickX = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Up)
      {
        joystickState->LeftStickY = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Down)
      {
        joystickState->LeftStickY = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Fire1)
      {
        joystickState->LeftButton1Status = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Fire2)
      {
        joystickState->LeftButton2Status = 0;
        retValue = 0;
      }
    }

    if (joystickState->Right.UseMouse == 0)
    {
      if (scanCode == joystickState->Right.Left)
      {
        joystickState->RightStickX = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Right)
      {
        joystickState->RightStickX = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Up)
      {
        joystickState->RightStickY = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Down)
      {
        joystickState->RightStickY = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Fire1)
      {
        joystickState->RightButton1Status = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Fire2)
      {
        joystickState->RightButton2Status = 0;
        retValue = 0;
      }
    }
    break;

  case 1:
    if (joystickState->Left.UseMouse == 0)
    {
      if (scanCode == joystickState->Left.Left)
      {
        joystickState->LeftStickX = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Right)
      {
        joystickState->LeftStickX = 63;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Up)
      {
        joystickState->LeftStickY = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Down)
      {
        joystickState->LeftStickY = 63;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Fire1)
      {
        joystickState->LeftButton1Status = 1;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Fire2)
      {
        joystickState->LeftButton2Status = 1;
        retValue = 0;
      }
    }

    if (joystickState->Right.UseMouse == 0)
    {
      if (scanCode == joystickState->Right.Left)
      {
        retValue = 0;
        joystickState->RightStickX = 0;
      }

      if (scanCode == joystickState->Right.Right)
      {
        joystickState->RightStickX = 63;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Up)
      {
        joystickState->RightStickY = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Down)
      {
        joystickState->RightStickY = 63;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Fire1)
      {
        joystickState->RightButton1Status = 1;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Fire2)
      {
        joystickState->RightButton2Status = 1;
        retValue = 0;
      }
    }
    break;
  }

  return(retValue);
}

void SetButtonStatus(unsigned char side, unsigned char state) //Side=0 Left Button Side=1 Right Button State 1=Down
{
  unsigned char buttonStatus = (side << 1) | state;

  KeyboardState* keyboardState = GetKeyBoardState();
  JoystickState* joystickState = GetJoystickState();

  if (joystickState->Left.UseMouse == 1)
    switch (buttonStatus)
    {
    case 0:
      joystickState->LeftButton1Status = 0;
      break;

    case 1:
      joystickState->LeftButton1Status = 1;
      break;

    case 2:
      joystickState->LeftButton2Status = 0;
      break;

    case 3:
      joystickState->LeftButton2Status = 1;
      break;
    }

  if (joystickState->Right.UseMouse == 1)
    switch (buttonStatus)
    {
    case 0:
      joystickState->RightButton1Status = 0;
      break;

    case 1:
      joystickState->RightButton1Status = 1;
      break;

    case 2:
      joystickState->RightButton2Status = 0;
      break;

    case 3:
      joystickState->RightButton2Status = 1;
      break;
    }
}

bool GetPaste() {
  KeyboardState* keyboardState = GetKeyBoardState();

  return keyboardState->Pasting;
}

void SetPaste(bool flag) {
  KeyboardState* keyboardState = GetKeyBoardState();

  keyboardState->Pasting = flag;
}
