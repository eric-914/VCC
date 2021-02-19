#include <string>

#include "library/clipboardstate.h"

bool SetClipboard(string sendout) {
  const char* clipout = sendout.c_str();
  const size_t len = strlen(clipout) + 1;
  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);

  ClipboardState* clipboardState = GetClipboardState();

  if (hMem == 0) {
    throw;
  }

  memcpy(GlobalLock(hMem), clipout, len);

  GlobalUnlock(hMem);
  OpenClipboard(0);
  EmptyClipboard();
  SetClipboardData(CF_TEXT, hMem);
  CloseClipboard();

  return TRUE;
}
