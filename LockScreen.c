#include "directdrawstate.h"

#include "DirectDrawAccessors.h"

#include "library/systemstate.h"

unsigned char LockScreen(SystemState* systemState)
{
  HRESULT	hr;
  DDSURFACEDESC ddsd;				      // A structure to describe the surfaces we want

  DirectDrawState* ddState = GetDirectDrawState();

  memset(&ddsd, 0, sizeof(ddsd));	// Clear all members of the structure to 0
  ddsd.dwSize = sizeof(ddsd);		  // The first parameter of the structure must contain the size of the structure

  CheckSurfaces();

  // Lock entire surface, wait if it is busy, return surface memory pointer
  hr = ddState->DDBackSurface->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);

  if (FAILED(hr))
  {
    return(1);
  }

  switch (ddsd.ddpfPixelFormat.dwRGBBitCount)
  {
  case 8:
    systemState->SurfacePitch = ddsd.lPitch;
    systemState->BitDepth = 0;
    break;

  case 15:
  case 16:
    systemState->SurfacePitch = ddsd.lPitch / 2;
    systemState->BitDepth = 1;
    break;

  case 24:
    MessageBox(0, "24 Bit color is currnetly unsupported", "Ok", 0);

    exit(0);

    systemState->SurfacePitch = ddsd.lPitch;
    systemState->BitDepth = 2;
    break;

  case 32:
    systemState->SurfacePitch = ddsd.lPitch / 4;
    systemState->BitDepth = 3;
    break;

  default:
    MessageBox(0, "Unsupported Color Depth!", "Error", 0);
    return 1;
  }

  if (ddsd.lpSurface == NULL) {
    MessageBox(0, "Returning NULL!!", "ok", 0);
  }

  systemState->PTRsurface8 = (unsigned char*)ddsd.lpSurface;
  systemState->PTRsurface16 = (unsigned short*)ddsd.lpSurface;
  systemState->PTRsurface32 = (unsigned int*)ddsd.lpSurface;

  return(0);
}
