#include "DisplayFlip.h"

#include "library/directdrawstate.h"
#include "library/systemstate.h"

void UnlockScreen(SystemState* systemState)
{
  static HRESULT hr;
  static size_t index = 0;
  static HDC hdc;

  DirectDrawState* ddState = GetDirectDrawState();

  if (systemState->FullScreen & ddState->InfoBand) //Put StatusText for full screen here
  {
    ddState->DDBackSurface->GetDC(&hdc);
    SetBkColor(hdc, RGB(0, 0, 0));
    SetTextColor(hdc, RGB(255, 255, 255));

    for (index = strlen(ddState->StatusText); index < 132; index++) {
      ddState->StatusText[index] = 32;
    }

    ddState->StatusText[index] = 0;

    TextOut(hdc, 0, 0, ddState->StatusText, 132);

    ddState->DDBackSurface->ReleaseDC(hdc);
  }

  hr = ddState->DDBackSurface->Unlock(NULL);

  DisplayFlip(systemState);
}
