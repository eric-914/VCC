#include "clipboardstate.h"

ClipboardState* InitializeInstance(ClipboardState* coco);

static ClipboardState* instance = InitializeInstance(new ClipboardState());

extern "C" {
  __declspec(dllexport) ClipboardState* __cdecl GetClipboardState() {
    return instance;
  }
}

ClipboardState* InitializeInstance(ClipboardState* c) {
  c->PasteWithNew = false;

  return c;
}
