#include "LockScreen.h"
#include "UnlockScreen.h"

#include "library/DirectDraw.h"
#include "library/systemstate.h"

void DoCls(SystemState* systemState)
{
  unsigned short x = 0, y = 0;

  DirectDrawState* ddState = GetDirectDrawState();

  if (LockScreen(systemState)) {
    return;
  }

  switch (systemState->BitDepth)
  {
  case 0:
    for (y = 0;y < 480; y++) {
      for (x = 0;x < 640; x++) {
        systemState->PTRsurface8[x + (y * systemState->SurfacePitch)] = ddState->Color | 128;
      }
    }
    break;

  case 1:
    for (y = 0;y < 480; y++) {
      for (x = 0;x < 640; x++) {
        systemState->PTRsurface16[x + (y * systemState->SurfacePitch)] = ddState->Color;
      }
    }
    break;

  case 2:
    for (y = 0;y < 480; y++) {
      for (x = 0;x < 640; x++)
      {
        systemState->PTRsurface8[(x * 3) + (y * systemState->SurfacePitch)] = (ddState->Color & 0xFF0000) >> 16;
        systemState->PTRsurface8[(x * 3) + 1 + (y * systemState->SurfacePitch)] = (ddState->Color & 0x00FF00) >> 8;
        systemState->PTRsurface8[(x * 3) + 2 + (y * systemState->SurfacePitch)] = (ddState->Color & 0xFF);
      }
    }
    break;

  case 3:
    for (y = 0;y < 480; y++) {
      for (x = 0;x < 640; x++) {
        systemState->PTRsurface32[x + (y * systemState->SurfacePitch)] = ddState->Color;
      }
    }
    break;

  default:
    return;
  }

  UnlockScreen(systemState);
}
