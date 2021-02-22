#include "library/Throttle.h"
#include "library/DirectDraw.h"
#include "library/systemstate.h"

#include "LockScreen.h"
#include "UnlockScreen.h"

float Static(SystemState* systemState)
{
  unsigned short x = 0;
  static unsigned short y = 0;
  unsigned char temp = 0;
  static unsigned short textX = 0, textY = 0;
  static unsigned char counter = 0, counter1 = 32;
  static char phase = 1;
  static char message[] = " Signal Missing! Press F9";
  static unsigned char greyScales[4] = { 128, 135, 184, 191 };
  HDC hdc;

  DirectDrawState* ddState = GetDirectDrawState();

  LockScreen(systemState);

  if (systemState->PTRsurface32 == NULL) {
    return(0);
  }

  switch (systemState->BitDepth)
  {
  case 0:
    for (y = 0;y < 480;y += 2) {
      for (x = 0;x < 160; x++) {
        temp = rand() & 3;

        systemState->PTRsurface32[x + (y * systemState->SurfacePitch >> 2)] = greyScales[temp] | (greyScales[temp] << 8) | (greyScales[temp] << 16) | (greyScales[temp] << 24);
        systemState->PTRsurface32[x + ((y + 1) * systemState->SurfacePitch >> 2)] = greyScales[temp] | (greyScales[temp] << 8) | (greyScales[temp] << 16) | (greyScales[temp] << 24);
      }
    }
    break;

  case 1:
    for (y = 0;y < 480;y += 2) {
      for (x = 0;x < 320; x++) {
        temp = rand() & 31;

        systemState->PTRsurface32[x + (y * systemState->SurfacePitch >> 1)] = temp | (temp << 6) | (temp << 11) | (temp << 16) | (temp << 22) | (temp << 27);
        systemState->PTRsurface32[x + ((y + 1) * systemState->SurfacePitch >> 1)] = temp | (temp << 6) | (temp << 11) | (temp << 16) | (temp << 22) | (temp << 27);
      }
    }
    break;

  case 2:
    for (y = 0;y < 480; y++) {
      for (x = 0;x < 640; x++) {
        systemState->PTRsurface8[(x * 3) + (y * systemState->SurfacePitch)] = temp;
        systemState->PTRsurface8[(x * 3) + 1 + (y * systemState->SurfacePitch)] = temp << 8;
        systemState->PTRsurface8[(x * 3) + 2 + (y * systemState->SurfacePitch)] = temp << 16;
      }
    }
    break;

  case 3:
    for (y = 0;y < 480; y++) {
      for (x = 0;x < 640; x++) {
        temp = rand() & 255;

        systemState->PTRsurface32[x + (y * systemState->SurfacePitch)] = temp | (temp << 8) | (temp << 16);
      }
    }
    break;

  default:
    return(0);
  }

  ddState->DDBackSurface->GetDC(&hdc);

  SetBkColor(hdc, 0);
  SetTextColor(hdc, RGB(counter1 << 2, counter1 << 2, counter1 << 2));

  TextOut(hdc, textX, textY, message, (int)strlen(message));

  counter++;
  counter1 += phase;

  if ((counter1 == 60) || (counter1 == 20)) {
    phase = -phase;
  }

  counter %= 60; //about 1 seconds

  if (!counter)
  {
    textX = rand() % 580;
    textY = rand() % 470;
  }

  ddState->DDBackSurface->ReleaseDC(hdc);

  UnlockScreen(systemState);

  return(CalculateFPS());
}
