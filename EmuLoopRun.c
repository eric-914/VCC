#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#include "library/VCC.h"
#include "library/Throttle.h"
#include "library/DirectDraw.h"
#include "library/PAKInterface.h"
#include "library/QuickLoad.h"
#include "library/Config.h"
#include "library/CoCo.h"


unsigned __stdcall EmuLoopRun(void* dummy)
{
  HANDLE hEvent = (HANDLE)dummy;

  //NOTE: This function isn't working in library.dll
  timeBeginPeriod(1);	//Needed to get max resolution from the timer normally its 10Ms
  CalibrateThrottle();
  timeEndPeriod(1);

  Sleep(30);
  SetEvent(hEvent);

  EmuLoop();

  return(NULL);
}
