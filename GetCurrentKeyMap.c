#include "library/clipboardstate.h"

int GetCurrentKeyMap() {
  return GetClipboardState()->CurrentKeyMap;
}
