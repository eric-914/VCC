#include <string>

#include "library/clipboardstate.h"
#include "ConfigAccessors.h"
#include "GetClipboardText.h"
#include "vccKeyboardBuildRuntimeTable.h"
#include "library/KeyboardAccessors.h"

#include "library/keyboarddef.h"
#include "library/graphicsstate.h"

void PasteText() {
  string cliptxt, clipparse, lines, out, debugout, tmp;
  unsigned char sc;
  char letter;
  bool CSHIFT;
  bool LCNTRL;

  ClipboardState* clipboardState = GetClipboardState();

  int GraphicsMode = GetGraphicsState()->GraphicsMode;

  if (GraphicsMode != 0) {
    int tmp = MessageBox(0, "Warning: You are not in text mode. Continue Pasting?", "Clipboard", MB_YESNO);

    if (tmp != 6) {
      return;
    }
  }

  SetPaste(true);

  //This sets the keyboard to Natural,
  //but we need to read it first so we can set it back
  clipboardState->CurrentKeyMap = GetKeyboardLayout();

  vccKeyboardBuildRuntimeTable((keyboardlayout_e)1);

  cliptxt = GetClipboardText().c_str();

  if (clipboardState->PasteWithNew) {
    cliptxt = "NEW\n" + cliptxt;
  }

  for (int t = 0; t < (int)cliptxt.length(); t++) {
    char tmp = cliptxt[t];

    if (tmp != (char)'\n') {
      lines += tmp;
    }
    else { //...the character is a <CR>
      if (lines.length() > 249 && lines.length() < 257 && clipboardState->CodePaste == true) {
        size_t b = lines.find(" ");
        string main = lines.substr(0, 249);
        string extra = lines.substr(249, lines.length() - 249);
        string spaces;

        for (int p = 1; p < 249; p++) {
          spaces.append(" ");
        }

        string linestr = lines.substr(0, b);

        lines = main + "\n\nEDIT " + linestr + "\n" + spaces + "I" + extra + "\n";
        clipparse += lines;
        lines.clear();
      }

      if (lines.length() >= 257 && clipboardState->CodePaste == true) {
        // Line is too long to handle. Truncate.
        size_t b = lines.find(" ");
        string linestr = "Warning! Line " + lines.substr(0, b) + " is too long for BASIC and will be truncated.";

        MessageBox(0, linestr.c_str(), "Clipboard", 0);

        lines = (lines.substr(0, 249));
      }

      if (lines.length() <= 249 || clipboardState->CodePaste == false) {
        // Just a regular line.
        clipparse += lines + "\n";
        lines.clear();
      }
    }

    if (t == cliptxt.length() - 1) {
      clipparse += lines;
    }
  }

  cliptxt = clipparse;

  for (int pp = 0; pp <= (int)cliptxt.size(); pp++) {
    sc = 0;
    CSHIFT = FALSE;
    LCNTRL = FALSE;
    letter = cliptxt[pp];

    switch (letter)
    {
    case '@': sc = 0x03; CSHIFT = TRUE; break;
    case 'A': sc = 0x1E; CSHIFT = TRUE; break;
    case 'B': sc = 0x30; CSHIFT = TRUE; break;
    case 'C': sc = 0x2E; CSHIFT = TRUE; break;
    case 'D': sc = 0x20; CSHIFT = TRUE; break;
    case 'E': sc = 0x12; CSHIFT = TRUE; break;
    case 'F': sc = 0x21; CSHIFT = TRUE; break;
    case 'G': sc = 0x22; CSHIFT = TRUE; break;
    case 'H': sc = 0x23; CSHIFT = TRUE; break;
    case 'I': sc = 0x17; CSHIFT = TRUE; break;
    case 'J': sc = 0x24; CSHIFT = TRUE; break;
    case 'K': sc = 0x25; CSHIFT = TRUE; break;
    case 'L': sc = 0x26; CSHIFT = TRUE; break;
    case 'M': sc = 0x32; CSHIFT = TRUE; break;
    case 'N': sc = 0x31; CSHIFT = TRUE; break;
    case 'O': sc = 0x18; CSHIFT = TRUE; break;
    case 'P': sc = 0x19; CSHIFT = TRUE; break;
    case 'Q': sc = 0x10; CSHIFT = TRUE; break;
    case 'R': sc = 0x13; CSHIFT = TRUE; break;
    case 'S': sc = 0x1F; CSHIFT = TRUE; break;
    case 'T': sc = 0x14; CSHIFT = TRUE; break;
    case 'U': sc = 0x16; CSHIFT = TRUE; break;
    case 'V': sc = 0x2F; CSHIFT = TRUE; break;
    case 'W': sc = 0x11; CSHIFT = TRUE; break;
    case 'X': sc = 0x2D; CSHIFT = TRUE; break;
    case 'Y': sc = 0x15; CSHIFT = TRUE; break;
    case 'Z': sc = 0x2C; CSHIFT = TRUE; break;
    case ' ': sc = 0x39; break;
    case 'a': sc = 0x1E; break;
    case 'b': sc = 0x30; break;
    case 'c': sc = 0x2E; break;
    case 'd': sc = 0x20; break;
    case 'e': sc = 0x12; break;
    case 'f': sc = 0x21; break;
    case 'g': sc = 0x22; break;
    case 'h': sc = 0x23; break;
    case 'i': sc = 0x17; break;
    case 'j': sc = 0x24; break;
    case 'k': sc = 0x25; break;
    case 'l': sc = 0x26; break;
    case 'm': sc = 0x32; break;
    case 'n': sc = 0x31; break;
    case 'o': sc = 0x18; break;
    case 'p': sc = 0x19; break;
    case 'q': sc = 0x10; break;
    case 'r': sc = 0x13; break;
    case 's': sc = 0x1F; break;
    case 't': sc = 0x14; break;
    case 'u': sc = 0x16; break;
    case 'v': sc = 0x2F; break;
    case 'w': sc = 0x11; break;
    case 'x': sc = 0x2D; break;
    case 'y': sc = 0x15; break;
    case 'z': sc = 0x2C; break;
    case '0': sc = 0x0B; break;
    case '1': sc = 0x02; break;
    case '2': sc = 0x03; break;
    case '3': sc = 0x04; break;
    case '4': sc = 0x05; break;
    case '5': sc = 0x06; break;
    case '6': sc = 0x07; break;
    case '7': sc = 0x08; break;
    case '8': sc = 0x09; break;
    case '9': sc = 0x0A; break;
    case '!': sc = 0x02; CSHIFT = TRUE; break;
    case '#': sc = 0x04; CSHIFT = TRUE;	break;
    case '$': sc = 0x05; CSHIFT = TRUE;	break;
    case '%': sc = 0x06; CSHIFT = TRUE;	break;
    case '^': sc = 0x07; CSHIFT = TRUE;	break;
    case '&': sc = 0x08; CSHIFT = TRUE;	break;
    case '*': sc = 0x09; CSHIFT = TRUE;	break;
    case '(': sc = 0x0A; CSHIFT = TRUE;	break;
    case ')': sc = 0x0B; CSHIFT = TRUE;	break;
    case '-': sc = 0x0C; break;
    case '=': sc = 0x0D; break;
    case ';': sc = 0x27; break;
    case '\'': sc = 0x28; break;
    case '/': sc = 0x35; break;
    case '.': sc = 0x34; break;
    case ',': sc = 0x33; break;
    case '\n': sc = 0x1C; break;
    case '+': sc = 0x0D; CSHIFT = TRUE;	break;
    case ':': sc = 0x27; CSHIFT = TRUE;	break;
    case '\"': sc = 0x28; CSHIFT = TRUE; break;
    case '?': sc = 0x35; CSHIFT = TRUE; break;
    case '<': sc = 0x33; CSHIFT = TRUE; break;
    case '>': sc = 0x34; CSHIFT = TRUE; break;
    case '[': sc = 0x1A; LCNTRL = TRUE; break;
    case ']': sc = 0x1B; LCNTRL = TRUE; break;
    case '{': sc = 0x1A; CSHIFT = TRUE; break;
    case '}': sc = 0x1B; CSHIFT = TRUE; break;
    case '\\"': sc = 0x2B; LCNTRL = TRUE; break;
    case '|': sc = 0x2B; CSHIFT = TRUE; break;
    case '`': sc = 0x29; break;
    case '~': sc = 0x29; CSHIFT = TRUE; break;
    case '_': sc = 0x0C; CSHIFT = TRUE; break;
    case 0x09: sc = 0x39; break; // TAB
    default: sc = 0xFF;	break;
    }

    if (CSHIFT) { out += 0x36; CSHIFT = FALSE; }
    if (LCNTRL) { out += 0x1D; LCNTRL = FALSE; }

    out += sc;
  }

  clipboardState->ClipboardText = out;
}
