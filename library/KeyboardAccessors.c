#include "keyboardstate.h"

extern "C" {
  __declspec(dllexport) unsigned char __cdecl GimeGetKeyboardInterruptState()
  {
    return GetKeyBoardState()->KeyboardInterruptEnabled;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GimeSetKeyboardInterruptState(unsigned char state)
  {
    GetKeyBoardState()->KeyboardInterruptEnabled = !!state;
  }
}

extern "C" {
  __declspec(dllexport) bool __cdecl GetPaste() {
    return GetKeyBoardState()->Pasting;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetPaste(bool flag) {
    GetKeyBoardState()->Pasting = flag;
  }
}