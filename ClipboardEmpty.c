#include "clipboardstate.h"

bool ClipboardEmpty() {
  return GetClipboardState()->ClipboardText.empty();
}
