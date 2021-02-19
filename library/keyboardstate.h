#pragma once

#include "keyboarddef.h"

typedef struct {
  /** track all keyboard scan codes state (up/down) */
  int ScanTable[256];

  /** run-time 'rollover' table to pass to the MC6821 when a key is pressed */
  unsigned char RolloverTable[8];	// CoCo 'keys' for emulator

  /** run-time key translation table - convert key up/down messages to 'rollover' codes */
  keytranslationentry_t KeyTransTable[KBTABLE_ENTRY_COUNT];	// run-time keyboard layout table (key(s) to keys(s) translation)

  unsigned char KeyboardInterruptEnabled;
  bool Pasting;  //Are the keyboard functions in the middle of a paste operation?
} KeyboardState;

//--Spelled funny because there's a GetKeyboardState() in User32.dll
extern "C" __declspec(dllexport) KeyboardState * __cdecl GetKeyBoardState();