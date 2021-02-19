#include <windows.h>
#include <ddraw.h>

#include "library/directdrawstate.h"
#include "library/systemstate.h"

POINT GetCurrentWindowSize() {
  DirectDrawState* ddState = GetDirectDrawState();

  return (ddState->RememberWinSize);
}

// Checks if the memory associated with surfaces is lost and restores if necessary.
void CheckSurfaces()
{
  DirectDrawState* ddState = GetDirectDrawState();

  if (ddState->DDSurface) {	// Check the primary surface
    if (ddState->DDSurface->IsLost() == DDERR_SURFACELOST) {
      ddState->DDSurface->Restore();
    }
  }

  if (ddState->DDBackSurface) {	// Check the back buffer
    if (ddState->DDBackSurface->IsLost() == DDERR_SURFACELOST) {
      ddState->DDBackSurface->Restore();
    }
  }
}

void SetStatusBarText(char* textBuffer, SystemState* systemState)
{
  DirectDrawState* ddState = GetDirectDrawState();

  if (!systemState->FullScreen)
  {
    SendMessage(ddState->hWndStatusBar, WM_SETTEXT, strlen(textBuffer), (LPARAM)(LPCSTR)textBuffer);
    SendMessage(ddState->hWndStatusBar, WM_SIZE, 0, 0);
  }
  else {
    strcpy(ddState->StatusText, textBuffer);
  }
}

void Cls(unsigned int ClsColor, SystemState* systemState)
{
  DirectDrawState* ddState = GetDirectDrawState();

  systemState->ResetPending = 3; //Tell Main loop to hold Emu
  ddState->Color = ClsColor;
}

unsigned char SetInfoBand(unsigned char infoBand)
{
  DirectDrawState* ddState = GetDirectDrawState();

  if (infoBand != QUERY) {
    ddState->InfoBand = infoBand;
  }

  return(ddState->InfoBand);
}

unsigned char SetResize(unsigned char resizeable)
{
  DirectDrawState* ddState = GetDirectDrawState();

  if (resizeable != QUERY) {
    ddState->Resizeable = resizeable;
  }

  return(ddState->Resizeable);
}

unsigned char SetAspect(unsigned char forceAspect)
{
  DirectDrawState* ddState = GetDirectDrawState();

  if (forceAspect != QUERY) {
    ddState->ForceAspect = forceAspect;
  }

  return(ddState->ForceAspect);
}
