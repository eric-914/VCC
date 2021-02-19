#include <string>

#include "library/clipboardstate.h"

string GetClipboardText()
{
  ClipboardState* clipboardState = GetClipboardState();

  if (!OpenClipboard(nullptr)) {
    MessageBox(0, "Unable to open clipboard.", "Clipboard", 0);

    return("");
  }

  HANDLE hClip = GetClipboardData(CF_TEXT);

  if (hClip == nullptr) {
    CloseClipboard();

    MessageBox(0, "No text found in clipboard.", "Clipboard", 0);

    return("");
  }

  char* tmp = static_cast<char*>(GlobalLock(hClip));

  if (tmp == nullptr) {
    CloseClipboard();

    MessageBox(0, "NULL Pointer", "Clipboard", 0);

    return("");
  }

  string out(tmp);

  GlobalUnlock(hClip);
  CloseClipboard();

  return out;
}
