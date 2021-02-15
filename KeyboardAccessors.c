#include "keyboardstate.h"

unsigned char GimeGetKeyboardInterruptState()
{
  return GetKeyBoardState()->KeyboardInterruptEnabled;
}

void GimeSetKeyboardInterruptState(unsigned char state)
{
  GetKeyBoardState()->KeyboardInterruptEnabled = !!state;
}

bool GetPaste() {
  return GetKeyBoardState()->Pasting;
}

void SetPaste(bool flag) {
  GetKeyBoardState()->Pasting = flag;
}
