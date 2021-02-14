#include "clipboardstate.h"

char PeekClipboard() {
  return GetClipboardState()->ClipboardText[0]; // get the next key in the string
}
