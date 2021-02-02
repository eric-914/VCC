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

#include <windows.h>
#include "throttle.h"
#include "audio.h"
#include "defines.h"
#include "vcc.h"

static _LARGE_INTEGER StartTime, EndTime, OneFrame, CurrentTime, SleepRes, TargetTime, OneMs;
static _LARGE_INTEGER MasterClock, Now;
static unsigned char FrameSkip = 0;
static float fMasterClock = 0;

void CalibrateThrottle(void)
{
  timeBeginPeriod(1);	//Needed to get max resolution from the timer normally its 10Ms
  QueryPerformanceFrequency(&MasterClock);
  OneFrame.QuadPart = MasterClock.QuadPart / (TARGETFRAMERATE);
  OneMs.QuadPart = MasterClock.QuadPart / 1000;
  fMasterClock = (float)MasterClock.QuadPart;
}

void StartRender(void)
{
  QueryPerformanceCounter(&StartTime);
}

void EndRender(unsigned char Skip)
{
  FrameSkip = Skip;
  TargetTime.QuadPart = (StartTime.QuadPart + (OneFrame.QuadPart * FrameSkip));
}

void FrameWait(void)
{
  QueryPerformanceCounter(&CurrentTime);

  while ((TargetTime.QuadPart - CurrentTime.QuadPart) > (OneMs.QuadPart * 2))	//If we have more that 2Ms till the end of the frame
  {
    Sleep(1);	//Give about 1Ms back to the system
    QueryPerformanceCounter(&CurrentTime);	//And check again
  }

  if (GetSoundStatus())	//Lean on the sound card a bit for timing
  {
    PurgeAuxBuffer();
    
    if (FrameSkip == 1)
    {
      if (GetFreeBlockCount() > AUDIOBUFFERS / 2)		//Dont let the buffer get lest that half full
        return;

      while (GetFreeBlockCount() < 1);	// Dont let it fill up either
    }
  }

  while (CurrentTime.QuadPart < TargetTime.QuadPart)	//Poll Untill frame end.
    QueryPerformanceCounter(&CurrentTime);
}

float CalculateFPS(void) //Done at end of render;
{
  static unsigned short FrameCount = 0;
  static float fps = 0, fNow = 0, fLast = 0;

  if (++FrameCount != FRAMEINTERVAL)
    return(fps);

  QueryPerformanceCounter(&Now);
  fNow = (float)Now.QuadPart;
  fps = (fNow - fLast) / fMasterClock;
  fLast = fNow;
  FrameCount = 0;
  fps = FRAMEINTERVAL / fps;

  return(fps);
}
