#pragma once

#include <ddraw.h>

#include "defines.h"
#include "systemstate.h"

#define NO_WARN_MBCS_MFC_DEPRECATION

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

extern "C" __declspec(dllexport) BOOL __cdecl InitInstance(HINSTANCE, HINSTANCE, int);

extern "C" __declspec(dllexport) POINT __cdecl GetCurrentWindowSize();

extern "C" __declspec(dllexport) bool __cdecl CreateDirectDrawWindow(SystemState*, WNDPROC);

extern "C" __declspec(dllexport) float __cdecl Static(SystemState*);

extern "C" __declspec(dllexport) unsigned char __cdecl LockScreen(SystemState*);
extern "C" __declspec(dllexport) unsigned char __cdecl SetAspect(unsigned char);
extern "C" __declspec(dllexport) unsigned char __cdecl SetInfoBand(unsigned char);
extern "C" __declspec(dllexport) unsigned char __cdecl SetResize(unsigned char);

extern "C" __declspec(dllexport) void __cdecl CheckSurfaces();
extern "C" __declspec(dllexport) void __cdecl Cls(unsigned int, SystemState*);
extern "C" __declspec(dllexport) void __cdecl DisplayFlip(SystemState*);
extern "C" __declspec(dllexport) void __cdecl DoCls(SystemState*);
extern "C" __declspec(dllexport) void __cdecl FullScreenToggle(WNDPROC);
extern "C" __declspec(dllexport) void __cdecl SetStatusBarText(char*, SystemState*);
extern "C" __declspec(dllexport) void __cdecl UnlockScreen(SystemState*);
