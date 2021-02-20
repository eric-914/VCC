#include "library/Clipboard.h"

#include "PasteText.h"

void PasteBASICWithNew() {
  ClipboardState* clipboardState = GetClipboardState();

  int tmp = MessageBox(0, "Warning: This operation will erase the Coco's BASIC memory\nbefore pasting. Continue?", "Clipboard", MB_YESNO);

  if (tmp != 6) {
    return;
  }

  clipboardState->CodePaste = true;
  clipboardState->PasteWithNew = true;

  PasteText();

  clipboardState->CodePaste = false;
  clipboardState->PasteWithNew = false;
}
