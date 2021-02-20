#include <windows.h>
#include <ddraw.h>

#include "directdrawstate.h"
#include "systemstate.h"

extern "C" {
  __declspec(dllexport) POINT __cdecl GetCurrentWindowSize() {
    DirectDrawState* ddState = GetDirectDrawState();

    return (ddState->RememberWinSize);
  }
}

// Checks if the memory associated with surfaces is lost and restores if necessary.
extern "C" {
  __declspec(dllexport) void __cdecl CheckSurfaces()
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
}

extern "C" {
  __declspec(dllexport) void __cdecl SetStatusBarText(char* textBuffer, SystemState* systemState)
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
}

extern "C" {
  __declspec(dllexport) void __cdecl Cls(unsigned int ClsColor, SystemState* systemState)
  {
    DirectDrawState* ddState = GetDirectDrawState();

    systemState->ResetPending = 3; //Tell Main loop to hold Emu
    ddState->Color = ClsColor;
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetInfoBand(unsigned char infoBand)
  {
    DirectDrawState* ddState = GetDirectDrawState();

    if (infoBand != QUERY) {
      ddState->InfoBand = infoBand;
    }

    return(ddState->InfoBand);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetResize(unsigned char resizeable)
  {
    DirectDrawState* ddState = GetDirectDrawState();

    if (resizeable != QUERY) {
      ddState->Resizeable = resizeable;
    }

    return(ddState->Resizeable);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetAspect(unsigned char forceAspect)
  {
    DirectDrawState* ddState = GetDirectDrawState();

    if (forceAspect != QUERY) {
      ddState->ForceAspect = forceAspect;
    }

    return(ddState->ForceAspect);
  }
}