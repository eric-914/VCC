/*
Copyright 2015 by Joseph Forgione
This file is part of VCC (Virtual Color Computer).

    VCC (Virtual Color Computer) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VCC (Virtual Color Computer) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VCC (Virtual Color Computer).  If not, see <http://www.gnu.org/licenses/>.
*/

#include "directdrawdef.h"

#include <afxwin.h>
#include <commctrl.h>	// Windows common controls
#include <iostream>
#include <string>

#include "ddraw.h"
#include "directdrawstate.h"

#include "DirectDrawInterface.h"
#include "resource.h"

#include "ConfigAccessors.h"
#include "CalculateFPS.h"

#include "library/defines.h"

//Function Prototypes for this module
extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //Callback for the main window

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  DirectDrawState* ddState = GetDirectDrawState();

  ddState->CmdShow = nCmdShow;

  AfxInitRichEdit();

  LoadString(hInstance, IDS_APP_TITLE, ddState->TitleBarText, MAX_LOADSTRING);
  LoadString(hInstance, IDS_APP_TITLE, ddState->AppNameText, MAX_LOADSTRING);

  return TRUE;
}

bool CreateDDWindow(SystemState* systemState)
{
  HRESULT hr;
  DDSURFACEDESC ddsd;				// A structure to describe the surfaces we want
  RECT rStatBar = RECT();
  RECT rc = RECT();
  PALETTEENTRY pal[256];
  IDirectDrawPalette* ddPalette;		  //Needed for 8bit Palette mode
  unsigned char ColorValues[4] = { 0,85,170,255 };

  DirectDrawState* ddState = GetDirectDrawState();

  memset(&ddsd, 0, sizeof(ddsd));	// Clear all members of the structure to 0

  ddsd.dwSize = sizeof(ddsd);		  // The first parameter of the structure must contain the size of the structure
  rc.top = 0;
  rc.left = 0;

  if (GetRememberSize()) {
    POINT pp = GetIniWindowSize();

    rc.right = pp.x;
    rc.bottom = pp.y;
  }
  else {
    rc.right = systemState->WindowSize.x;
    rc.bottom = systemState->WindowSize.y;
  }

  if (systemState->WindowHandle != NULL) //If its go a value it must be a mode switch
  {
    if (ddState->DD != NULL) {
      ddState->DD->Release();	//Destroy the current Window
    }

    DestroyWindow(systemState->WindowHandle);

    UnregisterClass(ddState->Wcex.lpszClassName, ddState->Wcex.hInstance);
  }

  ddState->Wcex.cbSize = sizeof(WNDCLASSEX);	//And Rebuilt it from scratch
  ddState->Wcex.style = CS_HREDRAW | CS_VREDRAW;
  ddState->Wcex.lpfnWndProc = (WNDPROC)WndProc;
  ddState->Wcex.cbClsExtra = 0;
  ddState->Wcex.cbWndExtra = 0;
  ddState->Wcex.hInstance = ddState->hInstance;
  ddState->Wcex.hIcon = LoadIcon(ddState->hInstance, (LPCTSTR)IDI_COCO3);
  ddState->Wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  ddState->Wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  ddState->Wcex.lpszMenuName = (LPCSTR)IDR_MENU;
  ddState->Wcex.lpszClassName = ddState->AppNameText;
  ddState->Wcex.hIconSm = LoadIcon(ddState->Wcex.hInstance, (LPCTSTR)IDI_COCO3);

  if (systemState->FullScreen)
  {
    ddState->Wcex.lpszMenuName = NULL;	//Fullscreen has no Menu Bar and no Mouse pointer
    ddState->Wcex.hCursor = LoadCursor(ddState->hInstance, MAKEINTRESOURCE(IDC_NONE));
  }

  if (!RegisterClassEx(&(ddState->Wcex))) {
    return FALSE;
  }

  switch (systemState->FullScreen)
  {
  case 0: //Windowed Mode
    // Calculates the required size of the window rectangle, based on the desired client-rectangle size
    // The window rectangle can then be passed to the CreateWindow function to create a window whose client area is the desired size.
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

    // We create the Main window 
    systemState->WindowHandle = CreateWindow(ddState->AppNameText, ddState->TitleBarText, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
      rc.right - rc.left, rc.bottom - rc.top,
      NULL, NULL, ddState->hInstance, NULL);

    if (!systemState->WindowHandle) {	// Can't create window
      return FALSE;
    }

    // Create the Status Bar Window at the bottom
    ddState->hWndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CCS_BOTTOM, "Ready", systemState->WindowHandle, 2);

    if (!ddState->hWndStatusBar) { // Can't create Status bar
      return FALSE;
    }

    // Retrieves the dimensions of the bounding rectangle of the specified window
    // The dimensions are given in screen coordinates that are relative to the upper-left corner of the screen.
    GetWindowRect(ddState->hWndStatusBar, &rStatBar); // Get the size of the Status bar

    ddState->StatusBarHeight = rStatBar.bottom - rStatBar.top; // Calculate its height

    // Get the size of main window and add height of status bar then resize Main
    // re-using rStatBar RECT even though it's the main window
    GetWindowRect(systemState->WindowHandle, &rStatBar);
    MoveWindow(systemState->WindowHandle, rStatBar.left, rStatBar.top, // using MoveWindow to resize 
      rStatBar.right - rStatBar.left, (rStatBar.bottom + ddState->StatusBarHeight) - rStatBar.top,
      1);

    SendMessage(ddState->hWndStatusBar, WM_SIZE, 0, 0); // Redraw Status bar in new position

    GetWindowRect(systemState->WindowHandle, &(ddState->WindowDefaultSize));	// And save the Final size of the Window 
    ShowWindow(systemState->WindowHandle, ddState->CmdShow);
    UpdateWindow(systemState->WindowHandle);

    // Create an instance of a DirectDraw object
    hr = DirectDrawCreate(NULL, &(ddState->DD), NULL);

    if (FAILED(hr)) {
      return FALSE;
    }

    // Initialize the DirectDraw object
    hr = ddState->DD->SetCooperativeLevel(systemState->WindowHandle, DDSCL_NORMAL);	// Set DDSCL_NORMAL to use windowed mode
    if (FAILED(hr)) {
      return FALSE;
    }

    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    // Create our Primary Surface
    hr = ddState->DD->CreateSurface(&ddsd, &(ddState->DDSurface), NULL);

    if (FAILED(hr)) {
      return FALSE;
    }

    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth = systemState->WindowSize.x;								// Make our off-screen surface 
    ddsd.dwHeight = systemState->WindowSize.y;
    ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;				// Try to create back buffer in video RAM
    hr = ddState->DD->CreateSurface(&ddsd, &(ddState->DDBackSurface), NULL);

    if (FAILED(hr)) {													// If not enough Video Ram 			
      ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;			// Try to create back buffer in System RAM
      hr = ddState->DD->CreateSurface(&ddsd, &(ddState->DDBackSurface), NULL);

      if (FAILED(hr)) {
        return FALSE;								//Giving Up
      }

      MessageBox(0, "Creating Back Buffer in System Ram!\nThis will be slower", "Performance Warning", 0);
    }

    hr = ddState->DD->GetDisplayMode(&ddsd);

    if (FAILED(hr)) {
      return FALSE;
    }

    hr = ddState->DD->CreateClipper(0, &(ddState->DDClipper), NULL);		// Create the clipper using the DirectDraw object

    if (FAILED(hr)) {
      return FALSE;
    }

    hr = ddState->DDClipper->SetHWnd(0, systemState->WindowHandle);	// Assign your window's HWND to the clipper

    if (FAILED(hr)) {
      return FALSE;
    }

    hr = ddState->DDSurface->SetClipper(ddState->DDClipper);					      // Attach the clipper to the primary surface

    if (FAILED(hr)) {
      return FALSE;
    }

    hr = ddState->DDBackSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

    if (FAILED(hr)) {
      return FALSE;
    }

    hr = ddState->DDBackSurface->Unlock(NULL);						// Unlock surface

    if (FAILED(hr)) {
      return FALSE;
    }

    break;

  case 1:	//Full Screen Mode
    ddsd.lPitch = 0;
    ddsd.ddpfPixelFormat.dwRGBBitCount = 0;

    systemState->WindowHandle = CreateWindow(ddState->AppNameText, NULL, WS_POPUP | WS_VISIBLE, 0, 0, systemState->WindowSize.x, systemState->WindowSize.y, NULL, NULL, ddState->hInstance, NULL);

    if (!systemState->WindowHandle) {
      return FALSE;
    }

    GetWindowRect(systemState->WindowHandle, &(ddState->WindowDefaultSize));
    ShowWindow(systemState->WindowHandle, ddState->CmdShow);
    UpdateWindow(systemState->WindowHandle);

    hr = DirectDrawCreate(NULL, &(ddState->DD), NULL);		// Initialize DirectDraw

    if (FAILED(hr)) {
      return FALSE;
    }

    hr = ddState->DD->SetCooperativeLevel(systemState->WindowHandle, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_NOWINDOWCHANGES);

    if (FAILED(hr)) {
      return FALSE;
    }

    hr = ddState->DD->SetDisplayMode(systemState->WindowSize.x, systemState->WindowSize.y, 32);	// Set 640x480x32 Bit full-screen mode

    if (FAILED(hr)) {
      return FALSE;
    }

    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
    ddsd.dwBackBufferCount = 1;

    hr = ddState->DD->CreateSurface(&ddsd, &(ddState->DDSurface), NULL);

    if (FAILED(hr)) {
      return FALSE;
    }

    ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

    ddState->DDSurface->GetAttachedSurface(&ddsd.ddsCaps, &(ddState->DDBackSurface));

    hr = ddState->DD->GetDisplayMode(&ddsd);

    if (FAILED(hr)) {
      return FALSE;
    }

    for (unsigned short i = 0;i <= 63;i++)
    {
      pal[i + 128].peBlue = ColorValues[(i & 8) >> 2 | (i & 1)];
      pal[i + 128].peGreen = ColorValues[(i & 16) >> 3 | (i & 2) >> 1];
      pal[i + 128].peRed = ColorValues[(i & 32) >> 4 | (i & 4) >> 2];
      pal[i + 128].peFlags = PC_RESERVED | PC_NOCOLLAPSE;
    }

    ddState->DD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, pal, &ddPalette, NULL);
    ddState->DDSurface->SetPalette(ddPalette); // Set pallete for Primary surface

    break;
  }

  return TRUE;
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

void DisplayFlip(SystemState* systemState)	// Double buffering flip
{
  using namespace std;

  static HRESULT hr;
  static RECT    rcSrc;  // source blit rectangle
  static RECT    rcDest; // destination blit rectangle
  static RECT	   rect;
  static POINT   p = POINT();

  DirectDrawState* ddState = GetDirectDrawState();

  if (systemState->FullScreen) {	// if we're windowed do the blit, else just Flip
    hr = ddState->DDSurface->Flip(NULL, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT); //DDFLIP_WAIT
  }
  else
  {
    p.x = 0; p.y = 0;

    // The ClientToScreen function converts the client-area coordinates of a specified point to screen coordinates.
    // in other word the client rectangle of the main windows 0, 0 (upper-left corner) 
    // in a screen x,y coords which is put back into p  
    ClientToScreen(systemState->WindowHandle, &p);  // find out where on the primary surface our window lives

    // get the actual client rectangle, which is always 0,0 - w,h
    GetClientRect(systemState->WindowHandle, &rcDest);

    // The OffsetRect function moves the specified rectangle by the specified offsets
    // add the delta screen point we got above, which gives us the client rect in screen coordinates.
    OffsetRect(&rcDest, p.x, p.y);

    // our destination rectangle is going to be 
    SetRect(&rcSrc, 0, 0, systemState->WindowSize.x, systemState->WindowSize.y);

    if (ddState->Resizeable)
    {
      rcDest.bottom -= ddState->StatusBarHeight;

      if (ddState->ForceAspect) // Adjust the Aspect Ratio if window is resized
      {
        float srcWidth = (float)systemState->WindowSize.x;
        float srcHeight = (float)systemState->WindowSize.y;
        float srcRatio = srcWidth / srcHeight;

        // change this to use the existing rcDest and the calc, w = right-left & h = bottom-top, 
        //                         because rcDest has already been converted to screen cords, right?   
        static RECT rcClient;

        GetClientRect(systemState->WindowHandle, &rcClient);  // x,y is always 0,0 so right, bottom is w,h

        rcClient.bottom -= ddState->StatusBarHeight;

        float clientWidth = (float)rcClient.right;
        float clientHeight = (float)rcClient.bottom;
        float clientRatio = clientWidth / clientHeight;

        float dstWidth = 0, dstHeight = 0;

        if (clientRatio > srcRatio)
        {
          dstWidth = srcWidth * clientHeight / srcHeight;
          dstHeight = clientHeight;
        }
        else
        {
          dstWidth = clientWidth;
          dstHeight = srcHeight * clientWidth / srcWidth;
        }

        float dstX = (clientWidth - dstWidth) / 2;
        float dstY = (clientHeight - dstHeight) / 2;

        static POINT pDstLeftTop = POINT();

        pDstLeftTop.x = (long)dstX; pDstLeftTop.y = (long)dstY;

        ClientToScreen(systemState->WindowHandle, &pDstLeftTop);

        static POINT pDstRightBottom = POINT();

        pDstRightBottom.x = (long)(dstX + dstWidth); pDstRightBottom.y = (long)(dstY + dstHeight);

        ClientToScreen(systemState->WindowHandle, &pDstRightBottom);

        SetRect(&rcDest, pDstLeftTop.x, pDstLeftTop.y, pDstRightBottom.x, pDstRightBottom.y);
      }
    }
    else
    {
      // this does not seem ideal, it lets you begin to resize and immediately resizes it back ... causing a lot of flicker.
      rcDest.right = rcDest.left + systemState->WindowSize.x;
      rcDest.bottom = rcDest.top + systemState->WindowSize.y;

      GetWindowRect(systemState->WindowHandle, &rect);
      MoveWindow(systemState->WindowHandle, rect.left, rect.top, ddState->WindowDefaultSize.right - ddState->WindowDefaultSize.left, ddState->WindowDefaultSize.bottom - ddState->WindowDefaultSize.top, 1);
    }

    if (ddState->DDBackSurface == NULL) {
      MessageBox(0, "Odd", "Error", 0); // yes, odd error indeed!! (??) especially since we go ahead and use it below!
    }

    hr = ddState->DDSurface->Blt(&rcDest, ddState->DDBackSurface, &rcSrc, DDBLT_WAIT, NULL); // DDBLT_WAIT
  }

  static RECT CurScreen;

  GetClientRect(systemState->WindowHandle, &CurScreen);

  int clientWidth = (int)CurScreen.right;
  int clientHeight = (int)CurScreen.bottom;

  ddState->RememberWinSize.x = clientWidth; // Used for saving new window size to the ini file.
  ddState->RememberWinSize.y = clientHeight - ddState->StatusBarHeight;
}

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

void UnlockScreen(SystemState* USState)
{
  static HRESULT hr;
  static size_t index = 0;
  static HDC hdc;

  DirectDrawState* ddState = GetDirectDrawState();

  if (USState->FullScreen & ddState->InfoBand) //Put StatusText for full screen here
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

  DisplayFlip(USState);
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

void Cls(unsigned int ClsColor, SystemState* CLState)
{
  DirectDrawState* ddState = GetDirectDrawState();

  CLState->ResetPending = 3; //Tell Main loop to hold Emu
  ddState->Color = ClsColor;
}

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

POINT GetCurWindowSize() {
  DirectDrawState* ddState = GetDirectDrawState();

  return (ddState->RememberWinSize);
}
