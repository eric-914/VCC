#include "library/vccstate.h"

// Save last two key down events
void SaveLastTwoKeyDownEvents(unsigned char kb_char, unsigned char oemScan) {
  VccState* vccState = GetVccState();

  // Ignore zero scan code
  if (oemScan == 0) {
    return;
  }

  // Remember it
  vccState->KeySaveToggle = !vccState->KeySaveToggle;

  if (vccState->KeySaveToggle) {
    vccState->KB_save1 = kb_char;
    vccState->SC_save1 = oemScan;
  }
  else {
    vccState->KB_save2 = kb_char;
    vccState->SC_save2 = oemScan;
  }
}
