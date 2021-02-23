#include "resources/resource.h"

#include "library/DirectDraw.h"

BOOL InitInstance(HINSTANCE hInstance, HINSTANCE hResources, int nCmdShow)
{
  DirectDrawState* ddState = GetDirectDrawState();

  ddState->hInstance = hInstance;
  ddState->CmdShow = nCmdShow;

  LoadString(hResources, IDS_APP_TITLE, ddState->TitleBarText, MAX_LOADSTRING);
  LoadString(hResources, IDS_APP_TITLE, ddState->AppNameText, MAX_LOADSTRING);

  return TRUE;
}
