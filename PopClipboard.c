#include "library/clipboardstate.h"

void PopClipboard() {
  ClipboardState* clipboardState = GetClipboardState();

  clipboardState->ClipboardText = clipboardState->ClipboardText.substr(1, clipboardState->ClipboardText.length() - 1); //move to next key in string
}
