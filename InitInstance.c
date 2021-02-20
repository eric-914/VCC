#include "library/nt.version.h"

#include <afxwin.h>

#include "resource.h"

#include "library/directdrawstate.h"

BOOL InitInstance(HINSTANCE hInstance, HINSTANCE hResources, int nCmdShow)
{
  DirectDrawState* ddState = GetDirectDrawState();

  ddState->hInstance = hInstance;
  ddState->CmdShow = nCmdShow;

  AfxInitRichEdit();

  LoadString(hResources, IDS_APP_TITLE, ddState->TitleBarText, MAX_LOADSTRING);
  LoadString(hResources, IDS_APP_TITLE, ddState->AppNameText, MAX_LOADSTRING);

  return TRUE;
}
