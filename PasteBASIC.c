#include "library/Clipboard.h"

#include "PasteText.h"

void PasteBASIC() {
  ClipboardState* clipboardState = GetClipboardState();

  clipboardState->CodePaste = true;

  PasteText();

  clipboardState->CodePaste = false;
}
