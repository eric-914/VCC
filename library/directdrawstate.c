#include "directdrawstate.h"

const unsigned int StatusBarHeight = 0;
const unsigned char InfoBand = 1;
const unsigned char Resizeable = 1;
const unsigned char ForceAspect = 1;
const unsigned int Color = 0;
const char StatusText[255] = "";

DirectDrawState* InitializeInstance(DirectDrawState*);

static DirectDrawState* instance = InitializeInstance(new DirectDrawState());

extern "C" {
  __declspec(dllexport) DirectDrawState* __cdecl GetDirectDrawState() {
    return instance;
  }
}

DirectDrawState* InitializeInstance(DirectDrawState* d) {
  d->DD = NULL;
  d->DDClipper = NULL;
  d->DDSurface = NULL;
  d->DDBackSurface = NULL;
  d->hWndStatusBar = NULL;

  d->StatusBarHeight = StatusBarHeight;
  d->InfoBand = InfoBand;
  d->Resizeable = Resizeable;
  d->ForceAspect = ForceAspect;
  d->Color = Color;

  strcpy(d->StatusText, StatusText);

  return d;
}
