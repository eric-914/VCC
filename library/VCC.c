#include <stdio.h>
#include <process.h>

#include "VCC.h"

#include "Config.h"
#include "Coco.h"
#include "PAKInterface.h"
#include "Keyboard.h"
#include "Graphics.h"
#include "Audio.h"
#include "MC6821.h"
#include "MC6809.h"
#include "HD6309.h"
#include "TC1014Registers.h"
#include "TC1014MMU.h"
#include "QuickLoad.h"
#include "DirectDraw.h"
#include "Throttle.h"

#include "cpudef.h"

VccState* InitializeInstance(VccState*);

static VccState* instance = InitializeInstance(new VccState());

extern "C" {
  __declspec(dllexport) VccState* __cdecl GetVccState() {
    return instance;
  }
}

VccState* InitializeInstance(VccState* p) {
  p->DialogOpen = false;

  p->AutoStart = 1;
  p->KB_save1 = 0;
  p->KB_save2 = 0;
  p->KeySaveToggle = 0;
  p->Qflag = 0;
  p->SC_save1 = 0;
  p->SC_save2 = 0;
  p->Throttle = 0;

  p->hEmuThread = NULL;
  p->FlagEmuStop = TH_RUNNING;

  strcpy(p->CpuName, "CPUNAME");
  strcpy(p->AppName, "");

  return p;
}

extern "C" {
  __declspec(dllexport) void __cdecl Reboot()
  {
    instance->SystemState.ResetPending = 2;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SaveConfig(void) {
    OPENFILENAME ofn;
    char curini[MAX_PATH];
    char newini[MAX_PATH + 4];  // Save room for '.ini' if needed

    GetIniFilePath(curini);  // EJJ get current ini file path
    strcpy(newini, curini);   // Let GetOpenFilename suggest it

    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = instance->SystemState.WindowHandle;
    ofn.lpstrFilter = "INI\0*.ini\0\0";      // filter string
    ofn.nFilterIndex = 1;                    // current filter index
    ofn.lpstrFile = newini;                  // contains full path on return
    ofn.nMaxFile = MAX_PATH;                 // sizeof lpstrFile
    ofn.lpstrFileTitle = NULL;               // filename and extension only
    ofn.nMaxFileTitle = MAX_PATH;            // sizeof lpstrFileTitle
    ofn.lpstrInitialDir = AppDirectory();    // EJJ initial directory
    ofn.lpstrTitle = TEXT("Save Vcc Config"); // title bar string
    ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
      if (ofn.nFileExtension == 0) {
        strcat(newini, ".ini");  //Add extension if none
      }

      WriteIniFile(); // Flush current config

      if (_stricmp(curini, newini) != 0) {
        if (!CopyFile(curini, newini, false)) { // Copy it to new file
          MessageBox(0, "Copy config failed", "error", 0);
        }
      }
    }
  }
}

// Save last two key down events
extern "C" {
  __declspec(dllexport) void __cdecl SaveLastTwoKeyDownEvents(unsigned char kb_char, unsigned char oemScan) {
    // Ignore zero scan code
    if (oemScan == 0) {
      return;
    }

    // Remember it
    instance->KeySaveToggle = !instance->KeySaveToggle;

    if (instance->KeySaveToggle) {
      instance->KB_save1 = kb_char;
      instance->SC_save1 = oemScan;
    }
    else {
      instance->KB_save2 = kb_char;
      instance->SC_save2 = oemScan;
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetAutoStart(unsigned char autostart)
  {
    if (autostart != QUERY) {
      instance->AutoStart = autostart;
    }

    return(instance->AutoStart);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetCPUMultiplayerFlag(unsigned char double_speed)
  {
    SetClockSpeed(1);

    instance->SystemState.DoubleSpeedFlag = double_speed;

    if (instance->SystemState.DoubleSpeedFlag) {
      SetClockSpeed(instance->SystemState.DoubleSpeedMultiplyer * instance->SystemState.TurboSpeedFlag);
    }

    instance->SystemState.CPUCurrentSpeed = .894;

    if (instance->SystemState.DoubleSpeedFlag) {
      instance->SystemState.CPUCurrentSpeed *= ((double)instance->SystemState.DoubleSpeedMultiplyer * (double)instance->SystemState.TurboSpeedFlag);
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetCPUMultiplayer(unsigned char multiplayer)
  {
    if (multiplayer != QUERY)
    {
      instance->SystemState.DoubleSpeedMultiplyer = multiplayer;

      SetCPUMultiplayerFlag(instance->SystemState.DoubleSpeedFlag);
    }

    return(instance->SystemState.DoubleSpeedMultiplyer);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetCpuType(unsigned char cpuType)
  {
    switch (cpuType)
    {
    case 0:
      instance->SystemState.CpuType = 0;

      strcpy(instance->CpuName, "MC6809");

      break;

    case 1:
      instance->SystemState.CpuType = 1;

      strcpy(instance->CpuName, "HD6309");

      break;
    }

    return(instance->SystemState.CpuType);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetFrameSkip(unsigned char skip)
  {
    if (skip != QUERY) {
      instance->SystemState.FrameSkip = skip;
    }

    return(instance->SystemState.FrameSkip);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetRamSize(unsigned char size)
  {
    if (size != QUERY) {
      instance->SystemState.RamSize = size;
    }

    return(instance->SystemState.RamSize);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetSpeedThrottle(unsigned char throttle)
  {
    if (throttle != QUERY) {
      instance->Throttle = throttle;
    }

    return(instance->Throttle);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetTurboMode(unsigned char data)
  {
    instance->SystemState.TurboSpeedFlag = (data & 1) + 1;

    SetClockSpeed(1);

    if (instance->SystemState.DoubleSpeedFlag) {
      SetClockSpeed(instance->SystemState.DoubleSpeedMultiplyer * instance->SystemState.TurboSpeedFlag);
    }

    instance->SystemState.CPUCurrentSpeed = .894;

    if (instance->SystemState.DoubleSpeedFlag) {
      instance->SystemState.CPUCurrentSpeed *= ((double)instance->SystemState.DoubleSpeedMultiplyer * (double)instance->SystemState.TurboSpeedFlag);
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned __cdecl CartLoad(void* dummy)
  {
    LoadCart(&(instance->SystemState));

    instance->SystemState.EmulationRunning = TRUE;
    instance->DialogOpen = false;

    return(NULL);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl LoadPack(void)
  {
    unsigned threadID;

    if (instance->DialogOpen) {
      return;
    }

    instance->DialogOpen = true;

    _beginthreadex(NULL, 0, &CartLoad, CreateEvent(NULL, FALSE, FALSE, NULL), 0, &threadID);
  }
}

// LoadIniFile allows user to browse for an ini file and reloads the config from it.
extern "C" {
  __declspec(dllexport) void __cdecl LoadIniFile(void)
  {
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    GetIniFilePath(szFileName); // EJJ load current ini file path

    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = instance->SystemState.WindowHandle;
    ofn.lpstrFilter = "INI\0*.ini\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrInitialDir = AppDirectory();
    ofn.lpstrTitle = TEXT("Load Vcc Config File");
    ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
      WriteIniFile();               // Flush current profile
      SetIniFilePath(szFileName);   // Set new ini file path
      ReadIniFile(&(instance->SystemState));                // Load it
      UpdateConfig(&(instance->SystemState));

      instance->SystemState.ResetPending = 2;
    }
  }
}

// Force keys up if main widow keyboard focus is lost.  Otherwise
// down keys will cause issues with OS-9 on return
// Send key up events to keyboard handler for saved keys
extern "C" {
  __declspec(dllexport) void __cdecl SendSavedKeyEvents() {

    if (instance->SC_save1) {
      vccKeyboardHandleKey(instance->KB_save1, instance->SC_save1, kEventKeyUp);
    }

    if (instance->SC_save2) {
      vccKeyboardHandleKey(instance->KB_save2, instance->SC_save2, kEventKeyUp);
    }

    instance->SC_save1 = 0;
    instance->SC_save2 = 0;
  }
}

void GimeReset(void)
{
  ResetGraphicsState();

  MakeRGBPalette();
  MakeCMPpalette(GetPaletteType());

  CocoReset();
  ResetAudio();
}

extern "C" {
  __declspec(dllexport) void __cdecl SoftReset(void)
  {
    MC6883Reset();
    MC6821_PiaReset();

    GetCPU()->CPUReset();

    GimeReset();
    MmuReset();
    CopyRom();
    ResetBus();

    instance->SystemState.TurboSpeedFlag = 1;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl HardReset(SystemState* systemState)
  {
    systemState->RamBuffer = MmuInit(systemState->RamSize);	//Allocate RAM/ROM & copy ROM Images from source
    systemState->WRamBuffer = (unsigned short*)systemState->RamBuffer;

    if (systemState->RamBuffer == NULL)
    {
      MessageBox(NULL, "Can't allocate enough RAM, Out of memory", "Error", 0);

      exit(0);
    }

    CPU* cpu = GetCPU();

    if (systemState->CpuType == 1)
    {
      cpu->CPUInit = HD6309Init;
      cpu->CPUExec = HD6309Exec;
      cpu->CPUReset = HD6309Reset;
      cpu->CPUAssertInterrupt = HD6309AssertInterrupt;
      cpu->CPUDeAssertInterrupt = HD6309DeAssertInterrupt;
      cpu->CPUForcePC = HD6309ForcePC;
    }
    else
    {
      cpu->CPUInit = MC6809Init;
      cpu->CPUExec = MC6809Exec;
      cpu->CPUReset = MC6809Reset;
      cpu->CPUAssertInterrupt = MC6809AssertInterrupt;
      cpu->CPUDeAssertInterrupt = MC6809DeAssertInterrupt;
      cpu->CPUForcePC = MC6809ForcePC;
    }

    MC6821_PiaReset();
    MC6883Reset();	//Captures interal rom pointer for CPU Interrupt Vectors

    cpu->CPUInit();
    cpu->CPUReset();		// Zero all CPU Registers and sets the PC to VRESET

    GimeReset();
    UpdateBusPointer();

    instance->SystemState.TurboSpeedFlag = 1;

    ResetBus();
    SetClockSpeed(1);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl EmuLoop() {
    static float fps;
    static unsigned int frameCounter = 0;

    while (true)
    {
      if (instance->FlagEmuStop == TH_REQWAIT)
      {
        instance->FlagEmuStop = TH_WAITING; //Signal Main thread we are waiting

        while (instance->FlagEmuStop == TH_WAITING) {
          Sleep(1);
        }
      }

      fps = 0;

      if ((instance->Qflag == 255) && (frameCounter == 30))
      {
        instance->Qflag = 0;

        QuickLoad(&(instance->SystemState), instance->QuickLoadFile);
      }

      StartRender();

      for (uint8_t frames = 1; frames <= instance->SystemState.FrameSkip; frames++)
      {
        frameCounter++;

        if (instance->SystemState.ResetPending != 0) {
          switch (instance->SystemState.ResetPending)
          {
          case 1:	//Soft Reset
            SoftReset();
            break;

          case 2:	//Hard Reset
            UpdateConfig(&(instance->SystemState));
            DoCls(&(instance->SystemState));
            HardReset(&(instance->SystemState));

            break;

          case 3:
            DoCls(&(instance->SystemState));
            break;

          case 4:
            UpdateConfig(&(instance->SystemState));
            DoCls(&(instance->SystemState));

            break;

          default:
            break;
          }

          instance->SystemState.ResetPending = 0;
        }

        if (instance->SystemState.EmulationRunning == 1) {
          fps += RenderFrame(&(instance->SystemState));
        }
        else {
          fps += Static(&(instance->SystemState));
        }
      }

      EndRender(instance->SystemState.FrameSkip);

      fps /= instance->SystemState.FrameSkip;

      GetModuleStatus(&(instance->SystemState));

      char ttbuff[256];

      snprintf(ttbuff, sizeof(ttbuff), "Skip:%2.2i | FPS:%3.0f | %s @ %2.2fMhz| %s", instance->SystemState.FrameSkip, fps, instance->CpuName, instance->SystemState.CPUCurrentSpeed, instance->SystemState.StatusLine);

      SetStatusBarText(ttbuff, &(instance->SystemState));

      if (instance->Throttle) { //Do nothing untill the frame is over returning unused time to OS
        FrameWait();
      }
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned __cdecl EmuLoopRun(void* dummy)
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
}