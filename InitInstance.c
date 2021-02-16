#include "library/nt.version.h"

#include <afxwin.h>

#include "directdrawstate.h"

#include "resource.h"

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  DirectDrawState* ddState = GetDirectDrawState();

  ddState->hInstance = hInstance;
  ddState->CmdShow = nCmdShow;

  AfxInitRichEdit();

  LoadString(hInstance, IDS_APP_TITLE, ddState->TitleBarText, MAX_LOADSTRING);
  LoadString(hInstance, IDS_APP_TITLE, ddState->AppNameText, MAX_LOADSTRING);

  return TRUE;
}
