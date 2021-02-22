#include <windows.h>

#include "library/DirectDraw.h"
#include "library/systemstate.h"

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
