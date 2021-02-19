#include "directdrawdef.h"
#include <ddraw.h>
#include <commctrl.h>	// Windows common controls

#include "directdrawstate.h"
#include "resource.h"

#include "ConfigAccessors.h"

#include "library/systemstate.h"

bool CreateDDWindow(SystemState* systemState, WNDPROC WndProc)
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
  ddState->Wcex.hIconSm = LoadIcon(ddState->hInstance, (LPCTSTR)IDI_COCO3); //LoadIcon(ddState->Wcex.hInstance, (LPCTSTR)IDI_COCO3);
  ddState->Wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  ddState->Wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  ddState->Wcex.lpszMenuName = (LPCSTR)IDR_MENU;
  ddState->Wcex.lpszClassName = ddState->AppNameText;

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
