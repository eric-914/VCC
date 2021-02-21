#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#include "library/VCC.h"
#include "library/Throttle.h"
#include "library/DirectDrawAccessors.h"
#include "library/PAKInterface.h"

#include "quickload.h"
#include "HardReset.h"
#include "SoftReset.h"
#include "UpdateConfig.h"
#include "DoCls.h"
#include "RenderFrame.h"
#include "Static.h"

unsigned __stdcall EmuLoop(void* dummy)
{
  HANDLE hEvent = (HANDLE)dummy;
  static float fps;
  static unsigned int frameCounter = 0;

  VccState* vccState = GetVccState();

  //NOTE: This function isn't working in library.dll
  timeBeginPeriod(1);	//Needed to get max resolution from the timer normally its 10Ms
  CalibrateThrottle();
  timeEndPeriod(1);

  Sleep(30);
  SetEvent(hEvent);

  while (true)
  {
    if (vccState->FlagEmuStop == TH_REQWAIT)
    {
      vccState->FlagEmuStop = TH_WAITING; //Signal Main thread we are waiting

      while (vccState->FlagEmuStop == TH_WAITING) {
        Sleep(1);
      }
    }

    fps = 0;

    if ((vccState->Qflag == 255) && (frameCounter == 30))
    {
      vccState->Qflag = 0;

      QuickLoad(&(vccState->EmuState), vccState->QuickLoadFile);
    }

    StartRender();

    for (uint8_t frames = 1; frames <= vccState->EmuState.FrameSkip; frames++)
    {
      frameCounter++;

      if (vccState->EmuState.ResetPending != 0) {
        switch (vccState->EmuState.ResetPending)
        {
        case 1:	//Soft Reset
          SoftReset();
          break;

        case 2:	//Hard Reset
          UpdateConfig(&(vccState->EmuState));
          DoCls(&(vccState->EmuState));
          HardReset(&(vccState->EmuState));

          break;

        case 3:
          DoCls(&(vccState->EmuState));
          break;

        case 4:
          UpdateConfig(&(vccState->EmuState));
          DoCls(&(vccState->EmuState));

          break;

        default:
          break;
        }

        vccState->EmuState.ResetPending = 0;
      }

      if (vccState->EmuState.EmulationRunning == 1) {
        fps += RenderFrame(&(vccState->EmuState));
      }
      else {
        fps += Static(&(vccState->EmuState));
      }
    }

    EndRender(vccState->EmuState.FrameSkip);

    fps /= vccState->EmuState.FrameSkip;

    GetModuleStatus(&(vccState->EmuState));

    char ttbuff[256];

    snprintf(ttbuff, sizeof(ttbuff), "Skip:%2.2i | FPS:%3.0f | %s @ %2.2fMhz| %s", vccState->EmuState.FrameSkip, fps, vccState->CpuName, vccState->EmuState.CPUCurrentSpeed, vccState->EmuState.StatusLine);

    SetStatusBarText(ttbuff, &(vccState->EmuState));

    if (vccState->Throttle) { //Do nothing untill the frame is over returning unused time to OS
      FrameWait();
    }
  }

  return(NULL);
}

