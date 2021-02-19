#pragma once

#include <ddraw.h>

#include "defines.h"

typedef struct {
  //Global Variables for Direct Draw functions
  LPDIRECTDRAW        DD;             // The DirectDraw object
  LPDIRECTDRAWCLIPPER DDClipper;      // Clipper for primary surface
  LPDIRECTDRAWSURFACE DDSurface;      // Primary surface
  LPDIRECTDRAWSURFACE DDBackSurface;  // Back surface

  HWND hWndStatusBar;
  TCHAR TitleBarText[MAX_LOADSTRING];	// The title bar text
  TCHAR AppNameText[MAX_LOADSTRING];	// The title bar text

  RECT WindowDefaultSize;
  HINSTANCE hInstance;
  WNDCLASSEX Wcex;
  POINT RememberWinSize;

  int CmdShow;
  unsigned int StatusBarHeight;
  unsigned char InfoBand;
  unsigned char Resizeable;
  unsigned char ForceAspect;
  unsigned int Color;

  char StatusText[255];
} DirectDrawState;

extern "C" __declspec(dllexport) DirectDrawState * __cdecl GetDirectDrawState();
