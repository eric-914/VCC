#include "vccstate.h"

#include "vccKeyboardHandleKey.h"

// Send key up events to keyboard handler for saved keys
void SendSavedKeyEvents() {
  VccState* vccState = GetVccState();

  if (vccState->SC_save1) {
    vccKeyboardHandleKey(vccState->KB_save1, vccState->SC_save1, kEventKeyUp);
  }

  if (vccState->SC_save2) {
    vccKeyboardHandleKey(vccState->KB_save2, vccState->SC_save2, kEventKeyUp);
  }

  vccState->SC_save1 = 0;
  vccState->SC_save2 = 0;
}
