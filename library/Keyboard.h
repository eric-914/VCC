#pragma once

#define KBTABLE_ENTRY_COUNT 100	///< key translation table maximum size, (arbitrary) most of the layouts are < 80 entries
#define KEY_DOWN	1
#define KEY_UP		0

typedef enum keyevent_e
{
  kEventKeyUp = 0,
  kEventKeyDown = 1
} keyevent_e;

/**
  Keyboard layouts
*/
typedef enum keyboardlayout_e
{
  kKBLayoutCoCo = 0,
  kKBLayoutNatural,
  kKBLayoutCompact,
  kKBLayoutCustom,

  kKBLayoutCount
} keyboardlayout_e;

/**
  Keyboard layout names used to populate the
  layout selection pull-down in the config dialog

  This of course must match keyboardlayout_e above
*/
const char* const k_keyboardLayoutNames[] =
{
  "CoCo (DECB)",
  "Natural (OS-9)",
  "Compact (OS-9)",
  "Custom"
};

typedef struct keytranslationentry_t
{
  unsigned char ScanCode1;
  unsigned char ScanCode2;
  unsigned char Row1;
  unsigned char Col1;
  unsigned char Row2;
  unsigned char Col2;
} keytranslationentry_t;

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

extern "C" __declspec(dllexport) bool __cdecl GetPaste();

extern "C" __declspec(dllexport) int __cdecl KeyTransCompare(const void*, const void*);

extern "C" __declspec(dllexport) unsigned char __cdecl GimeGetKeyboardInterruptState();
extern "C" __declspec(dllexport) unsigned char __cdecl vccKeyboardGetScan(unsigned char);

extern "C" __declspec(dllexport) void __cdecl GimeSetKeyboardInterruptState(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetPaste(bool);
extern "C" __declspec(dllexport) void __cdecl vccKeyboardBuildRuntimeTable(keyboardlayout_e);
extern "C" __declspec(dllexport) void __cdecl vccKeyboardHandleKey(unsigned char, unsigned char, keyevent_e);
extern "C" __declspec(dllexport) void __cdecl vccKeyboardUpdateRolloverTable();
