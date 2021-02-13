#include <math.h>

#include "cocostate.h"
#include "mc6821def.h"

#include "library/cpudef.h"
#include "library/defines.h"
#include "library/keyboarddef.h"

extern void GimeAssertHorzInterrupt(void);
extern void PakTimer(void);
extern void irq_hs(int);
extern void GimeAssertTimerInterrupt(void);
extern bool ClipboardEmpty();
extern void PopClipboard();
extern char PeekClipboard();
extern int GetCurrentKeyMap();
extern void SetPaste(bool);
extern unsigned char SetSpeedThrottle(unsigned char);

extern "C" void vccKeyboardBuildRuntimeTable(keyboardlayout_e keyBoardLayout);
extern "C" void vccKeyboardHandleKey(unsigned char, unsigned char, keyevent_e keyState);

/* _inline */ int CPUCycle(void)
{
  CPU* cpu = GetCPU();
  CoCoState* coco = GetCoCoState();

  if (coco->HorzInterruptEnabled) {
    GimeAssertHorzInterrupt();
  }

  irq_hs(ANY);
  PakTimer();

  coco->PicosThisLine += coco->PicosPerLine;

  while (coco->PicosThisLine > 1)
  {
    coco->StateSwitch = 0;

    if ((coco->PicosToInterrupt <= coco->PicosThisLine) && coco->IntEnable) {	//Does this iteration need to Timer Interrupt
      coco->StateSwitch = 1;
    }

    if ((coco->PicosToSoundSample <= coco->PicosThisLine) && coco->SndEnable) { //Does it need to collect an Audio sample
      coco->StateSwitch += 2;
    }

    switch (coco->StateSwitch)
    {
    case 0:		//No interrupts this line
      coco->CyclesThisLine = coco->CycleDrift + (coco->PicosThisLine * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

      if (coco->CyclesThisLine >= 1) {	//Avoid un-needed CPU engine calls
        coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
      }
      else {
        coco->CycleDrift = coco->CyclesThisLine;
      }

      coco->PicosToInterrupt -= coco->PicosThisLine;
      coco->PicosToSoundSample -= coco->PicosThisLine;
      coco->PicosThisLine = 0;

      break;

    case 1:		//Only Interrupting
      coco->PicosThisLine -= coco->PicosToInterrupt;
      coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToInterrupt * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

      if (coco->CyclesThisLine >= 1) {
        coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
      }
      else {
        coco->CycleDrift = coco->CyclesThisLine;
      }

      GimeAssertTimerInterrupt();

      coco->PicosToSoundSample -= coco->PicosToInterrupt;
      coco->PicosToInterrupt = coco->MasterTickCounter;

      break;

    case 2:		//Only Sampling
      coco->PicosThisLine -= coco->PicosToSoundSample;
      coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToSoundSample * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

      if (coco->CyclesThisLine >= 1) {
        coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
      }
      else {
        coco->CycleDrift = coco->CyclesThisLine;
      }

      coco->AudioEvent();

      coco->PicosToInterrupt -= coco->PicosToSoundSample;
      coco->PicosToSoundSample = coco->SoundInterrupt;

      break;

    case 3:		//Interrupting and Sampling
      if (coco->PicosToSoundSample < coco->PicosToInterrupt)
      {
        coco->PicosThisLine -= coco->PicosToSoundSample;
        coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToSoundSample * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

        if (coco->CyclesThisLine >= 1) {
          coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
        }
        else {
          coco->CycleDrift = coco->CyclesThisLine;
        }

        coco->AudioEvent();

        coco->PicosToInterrupt -= coco->PicosToSoundSample;
        coco->PicosToSoundSample = coco->SoundInterrupt;
        coco->PicosThisLine -= coco->PicosToInterrupt;

        coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToInterrupt * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

        if (coco->CyclesThisLine >= 1) {
          coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
        }
        else {
          coco->CycleDrift = coco->CyclesThisLine;
        }

        GimeAssertTimerInterrupt();

        coco->PicosToSoundSample -= coco->PicosToInterrupt;
        coco->PicosToInterrupt = coco->MasterTickCounter;

        break;
      }

      if (coco->PicosToSoundSample > coco->PicosToInterrupt)
      {
        coco->PicosThisLine -= coco->PicosToInterrupt;
        coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToInterrupt * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

        if (coco->CyclesThisLine >= 1) {
          coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
        }
        else {
          coco->CycleDrift = coco->CyclesThisLine;
        }

        GimeAssertTimerInterrupt();

        coco->PicosToSoundSample -= coco->PicosToInterrupt;
        coco->PicosToInterrupt = coco->MasterTickCounter;
        coco->PicosThisLine -= coco->PicosToSoundSample;
        coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToSoundSample * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

        if (coco->CyclesThisLine >= 1) {
          coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
        }
        else {
          coco->CycleDrift = coco->CyclesThisLine;
        }

        coco->AudioEvent();

        coco->PicosToInterrupt -= coco->PicosToSoundSample;
        coco->PicosToSoundSample = coco->SoundInterrupt;

        break;
      }

      //They are the same (rare)
      coco->PicosThisLine -= coco->PicosToInterrupt;
      coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToSoundSample * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

      if (coco->CyclesThisLine > 1) {
        coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
      }
      else {
        coco->CycleDrift = coco->CyclesThisLine;
      }

      GimeAssertTimerInterrupt();

      coco->AudioEvent();

      coco->PicosToInterrupt = coco->MasterTickCounter;
      coco->PicosToSoundSample = coco->SoundInterrupt;
    }
  }

  if (!ClipboardEmpty()) {
    char tmp[] = { 0x00 };
    char kbstate = 2;
    int z = 0;
    char key;
    const char SHIFT = 0x36;

    //Remember the original throttle setting.
    //Set it to off. We need speed for this!
    if (coco->Throttle == 0) {
      coco->Throttle = SetSpeedThrottle(QUERY);

      if (coco->Throttle == 0) {
        coco->Throttle = 2; // 2 = No throttle.
      }
    }

    SetSpeedThrottle(0);

    if (coco->ClipCycle == 1) {
      key = PeekClipboard();

      if (key == SHIFT) {
        vccKeyboardHandleKey(SHIFT, SHIFT, kEventKeyDown);  //Press shift and...
        PopClipboard();
        key = PeekClipboard();
      }

      vccKeyboardHandleKey(key, key, kEventKeyDown);

      coco->WaitCycle = key == 0x1c ? 6000 : 2000;
    }
    else if (coco->ClipCycle == 500) {
      key = PeekClipboard();

      vccKeyboardHandleKey(SHIFT, SHIFT, kEventKeyUp);
      vccKeyboardHandleKey(0x42, key, kEventKeyUp);
      PopClipboard();

      if (ClipboardEmpty()) { //Finished?
        SetPaste(false);

        //Done pasting. Reset throttle to original state
        if (coco->Throttle == 2) {
          SetSpeedThrottle(0);
        }
        else {
          SetSpeedThrottle(1);
        }

        //...and reset the keymap to the original state
        vccKeyboardBuildRuntimeTable((keyboardlayout_e)GetCurrentKeyMap());

        coco->Throttle = 0;
      }
    }

    coco->ClipCycle++;

    if (coco->ClipCycle > coco->WaitCycle) {
      coco->ClipCycle = 1;
    }
  }

  return(0);
}
