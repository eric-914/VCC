#include <windows.h>
#include <process.h>

#include "library/Audio.h"
#include "library/CoCo.h"
#include "library/DirectDraw.h"
#include "library/VCC.h"
#include "library/fileoperations.h"
#include "library/Graphics.h"
#include "library/PAKInterface.h"
#include "library/Config.h"

#include "ProcessMessage.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ INT nCmdShow) {
  MSG  msg;
  HANDLE hEvent;
  HANDLE OleInitialize(NULL); //Work around fixs app crashing in "Open file" system dialogs (related to Adobe acrobat 7+
  char temp1[MAX_PATH] = "";
  char temp2[MAX_PATH] = " Running on ";
  unsigned threadID;

  HMODULE hResources = LoadLibrary("..\\resources\\resources.dll");

  VccState* vccState = GetVccState();

  vccState->EmuState.Resources = hResources;

  //LoadString(hInstance, IDS_APP_TITLE, vccState->AppName, MAX_LOADSTRING);

  GetCmdLineArgs(lpCmdLine, &(vccState->CmdArg)); //Parse command line

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

  vccState->EmuState.WindowSize.x = 640;
  vccState->EmuState.WindowSize.y = 480;

  InitInstance(hInstance, hResources, nCmdShow);

  if (!CreateDirectDrawWindow(&(vccState->EmuState), WndProc))
  {
    MessageBox(0, "Can't create primary Window", "Error", 0);

    exit(0);
  }

  //NOTE: Sound is lost if this isn't done after CreateDDWindow
  LoadConfig(&(vccState->EmuState), vccState->CmdArg);			//Loads the default config file Vcc.ini from the exec directory

  Cls(0, &(vccState->EmuState));
  DynamicMenuCallback(&(vccState->EmuState), "", 0, 0);
  DynamicMenuCallback(&(vccState->EmuState), "", 1, 0);

  vccState->EmuState.ResetPending = 2;

  SetClockSpeed(1);	//Default clock speed .89 MHZ	

  vccState->BinaryRunning = true;
  vccState->EmuState.EmulationRunning = vccState->AutoStart;

  if (strlen(vccState->CmdArg.QLoadFile) != 0)
  {
    vccState->Qflag = 255;
    vccState->EmuState.EmulationRunning = 1;
  }

  hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  if (hEvent == NULL)
  {
    MessageBox(0, "Can't create Thread!!", "Error", 0);

    return(0);
  }

  vccState->hEMUThread = (HANDLE)_beginthreadex(NULL, 0, &EmuLoopRun, hEvent, 0, &threadID);

  if (vccState->hEMUThread == NULL)
  {
    MessageBox(0, "Can't Start main Emulation Thread!", "Ok", 0);

    return(0);
  }

  WaitForSingleObject(hEvent, INFINITE);
  SetThreadPriority(vccState->hEMUThread, THREAD_PRIORITY_NORMAL);

  while (vccState->BinaryRunning)
  {
    if (vccState->FlagEmuStop == TH_WAITING)		//Need to stop the EMU thread for screen mode change
    {								                  //As it holds the Secondary screen buffer open while running
      FullScreenToggle(WndProc);

      vccState->FlagEmuStop = TH_RUNNING;
    }

    GetMessage(&msg, NULL, 0, 0);		//Seems if the main loop stops polling for Messages the child threads stall

    TranslateMessage(&msg);

    DispatchMessage(&msg);
  }

  CloseHandle(hEvent);
  CloseHandle(vccState->hEMUThread);
  timeEndPeriod(1);
  UnloadDll(&(vccState->EmuState));
  SoundDeInit();
  WriteIniFile(); //Save Any changes to ini File

  return (INT)msg.wParam;
}

/*--------------------------------------------------------------------------*/
// The Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  ProcessMessage(hWnd, message, wParam, lParam);

  return DefWindowProc(hWnd, message, wParam, lParam);
}
