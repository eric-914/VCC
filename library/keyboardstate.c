#include "keyboardstate.h"

const unsigned char KeyboardInterruptEnabled = 0;
const bool Pasting = false;

KeyboardState* InitializeInstance(KeyboardState*);

static KeyboardState* instance = InitializeInstance(new KeyboardState());

//--Spelled funny because there's a GetKeyboardState() in User32.dll
extern "C" {
  __declspec(dllexport) KeyboardState* __cdecl GetKeyBoardState() {
    return instance;
  }
}

KeyboardState* InitializeInstance(KeyboardState* k) {
  k->KeyboardInterruptEnabled = KeyboardInterruptEnabled;
  k->Pasting = Pasting;

  return k;
}