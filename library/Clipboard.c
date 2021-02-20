#include "Clipboard.h"

ClipboardState* InitializeInstance(ClipboardState*);

static ClipboardState* instance = InitializeInstance(new ClipboardState());

extern "C" {
  __declspec(dllexport) ClipboardState* __cdecl GetClipboardState() {
    return instance;
  }
}

ClipboardState* InitializeInstance(ClipboardState* p) {
  p->PasteWithNew = false;

  return p;
}

extern "C" {
  __declspec(dllexport) bool __cdecl ClipboardEmpty() {
    return instance->ClipboardText.empty();
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetCurrentKeyMap() {
    return instance->CurrentKeyMap;
  }
}

extern "C" {
  __declspec(dllexport) char __cdecl PeekClipboard() {
    return instance->ClipboardText[0]; // get the next key in the string
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl PopClipboard() {
    //TODO: Will get some <string> assertion failures if these 3 lines are combined into 1.
    size_t length = instance->ClipboardText.length() - 1;
    string remaining = instance->ClipboardText.substr(1, length);

    instance->ClipboardText = remaining; //move to next key in string
  }
}

extern "C" {
  __declspec(dllexport) bool __cdecl SetClipboard(string sendout) {
    const char* clipout = sendout.c_str();
    const size_t len = strlen(clipout) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);

    if (hMem == 0) {
      MessageBox(0, "Failed to access clipboard.", "Clipboard", 0);
      return false;
    }

    memcpy(GlobalLock(hMem), clipout, len);

    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();

    return TRUE;
  }
}