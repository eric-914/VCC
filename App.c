#include <windows.h>
#include <process.h>

#include "library/VCC.h"
#include "library/DirectDraw.h"
#include "library/Config.h"
#include "library/CoCo.h"
#include "library/PAKInterface.h"
#include "library/Audio.h"

#include "library/ProcessMessage.h"
#include "library/fileoperations.h"

/*--------------------------------------------------------------------------*/
// The Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  ProcessMessage(hWnd, message, wParam, lParam);

  return DefWindowProc(hWnd, message, wParam, lParam);
}

HMODULE LoadResources() {
  HMODULE hResources = LoadLibrary("..\\resources\\resources.dll");

  VccState* vccState = GetVccState();

  vccState->SystemState.Resources = hResources;

  return hResources;
}

void CheckQuickLoad() {
  char temp1[MAX_PATH] = "";
  char temp2[MAX_PATH] = " Running on ";

  VccState* vccState = GetVccState();

  if (strlen(vccState->CmdArg.QLoadFile) != 0)
  {
    strcpy(vccState->QuickLoadFile, vccState->CmdArg.QLoadFile);
    strcpy(temp1, vccState->CmdArg.QLoadFile);

    FilePathStripPath(temp1);

    _strlwr(temp1);

    temp1[0] = toupper(temp1[0]);

    strcat(temp1, temp2);
    strcat(temp1, vccState->AppName);
    strcpy(vccState->AppName, temp1);
  }
};

HANDLE CreateEventHandle() {
  HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  if (hEvent == NULL)
  {
    MessageBox(0, "Can't create event thread!!", "Error", 0);

    exit(0);
  }

  return hEvent;
}

void CreatePrimaryWindow() {
  VccState* vccState = GetVccState();

  if (!CreateDirectDrawWindow(&(vccState->SystemState), WndProc))
  {
    MessageBox(0, "Can't create primary window", "Error", 0);

    exit(0);
  }
}

HANDLE CreateThreadHandle(HANDLE hEvent) {
  unsigned threadID;

  HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &EmuLoopRun, hEvent, 0, &threadID);

  if (hThread == NULL)
  {
    MessageBox(0, "Can't Start main Emulation Thread!", "Ok", 0);

    exit(0);
  }

  return hThread;
}

void AppStartup(HINSTANCE hInstance, PSTR lpCmdLine, INT nCmdShow) {
  HANDLE OleInitialize(NULL); //Work around fixs app crashing in "Open file" system dialogs (related to Adobe acrobat 7+
  HMODULE hResources = LoadResources();

  VccState* vccState = GetVccState();
  CmdLineArguments* cmdArg = &(vccState->CmdArg);
  SystemState* systemState = &(vccState->SystemState);

  GetCmdLineArgs(lpCmdLine, cmdArg); //Parse command line

  CheckQuickLoad();
  InitInstance(hInstance, hResources, nCmdShow);

  CreatePrimaryWindow();

  //NOTE: Sound is lost if this isn't done after CreatePrimaryWindow();
  LoadConfig(systemState, *cmdArg);			//Loads the default config file Vcc.ini from the exec directory

  Cls(0, systemState);
  DynamicMenuCallback(systemState, "", 0, 0);
  DynamicMenuCallback(systemState, "", 1, 0);

  SetClockSpeed(1);	//Default clock speed .89 MHZ	

  (*systemState).ResetPending = 2;
  (*systemState).EmulationRunning = vccState->AutoStart;
  vccState->BinaryRunning = true;

  if (strlen((*cmdArg).QLoadFile) != 0)
  {
    vccState->Qflag = 255;
    vccState->SystemState.EmulationRunning = 1;
  }

  vccState->hEventThread = CreateEventHandle();
  vccState->hEmuThread = CreateThreadHandle(vccState->hEventThread);

  WaitForSingleObject(vccState->hEventThread, INFINITE);
  SetThreadPriority(vccState->hEmuThread, THREAD_PRIORITY_NORMAL);
}

void AppRun() {
  VccState* vccState = GetVccState();

  MSG* msg = &(vccState->msg);

  while (vccState->BinaryRunning)
  {
    if (vccState->FlagEmuStop == TH_WAITING)		//Need to stop the EMU thread for screen mode change
    {								                  //As it holds the Secondary screen buffer open while running
      FullScreenToggle(WndProc);

      vccState->FlagEmuStop = TH_RUNNING;
    }

    GetMessage(msg, NULL, 0, 0);		//Seems if the main loop stops polling for Messages the child threads stall

    TranslateMessage(msg);

    DispatchMessage(msg);
  }
}

INT AppShutdown() {
  VccState* vccState = GetVccState();

  CloseHandle(vccState->hEventThread);
  CloseHandle(vccState->hEmuThread);
  UnloadDll(&(vccState->SystemState));
  SoundDeInit();
  WriteIniFile(); //Save Any changes to ini File

  return (INT)(vccState->msg.wParam);
}
