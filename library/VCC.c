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
  __declspec(dllexport) void __cdecl Reboot(void)
  {
    GetVccState()->SystemState.ResetPending = 2;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SaveConfig(void) {
    OPENFILENAME ofn;
    char curini[MAX_PATH];
    char newini[MAX_PATH + 4];  // Save room for '.ini' if needed

    VccState* vccState = GetVccState();

    GetIniFilePath(curini);  // EJJ get current ini file path
    strcpy(newini, curini);   // Let GetOpenFilename suggest it

    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = vccState->SystemState.WindowHandle;
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
    VccState* vccState = GetVccState();

    // Ignore zero scan code
    if (oemScan == 0) {
      return;
    }

    // Remember it
    vccState->KeySaveToggle = !vccState->KeySaveToggle;

    if (vccState->KeySaveToggle) {
      vccState->KB_save1 = kb_char;
      vccState->SC_save1 = oemScan;
    }
    else {
      vccState->KB_save2 = kb_char;
      vccState->SC_save2 = oemScan;
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetAutoStart(unsigned char autostart)
  {
    VccState* vccState = GetVccState();

    if (autostart != QUERY) {
      vccState->AutoStart = autostart;
    }

    return(vccState->AutoStart);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetCPUMultiplayerFlag(unsigned char double_speed)
  {
    VccState* vccState = GetVccState();

    SetClockSpeed(1);

    vccState->SystemState.DoubleSpeedFlag = double_speed;

    if (vccState->SystemState.DoubleSpeedFlag) {
      SetClockSpeed(vccState->SystemState.DoubleSpeedMultiplyer * vccState->SystemState.TurboSpeedFlag);
    }

    vccState->SystemState.CPUCurrentSpeed = .894;

    if (vccState->SystemState.DoubleSpeedFlag) {
      vccState->SystemState.CPUCurrentSpeed *= ((double)vccState->SystemState.DoubleSpeedMultiplyer * (double)vccState->SystemState.TurboSpeedFlag);
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetCPUMultiplayer(unsigned char multiplayer)
  {
    VccState* vccState = GetVccState();

    if (multiplayer != QUERY)
    {
      vccState->SystemState.DoubleSpeedMultiplyer = multiplayer;

      SetCPUMultiplayerFlag(vccState->SystemState.DoubleSpeedFlag);
    }

    return(vccState->SystemState.DoubleSpeedMultiplyer);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetCpuType(unsigned char cpuType)
  {
    VccState* vccState = GetVccState();

    switch (cpuType)
    {
    case 0:
      vccState->SystemState.CpuType = 0;

      strcpy(vccState->CpuName, "MC6809");

      break;

    case 1:
      vccState->SystemState.CpuType = 1;

      strcpy(vccState->CpuName, "HD6309");

      break;
    }

    return(vccState->SystemState.CpuType);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetFrameSkip(unsigned char skip)
  {
    VccState* vccState = GetVccState();

    if (skip != QUERY) {
      vccState->SystemState.FrameSkip = skip;
    }

    return(vccState->SystemState.FrameSkip);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetRamSize(unsigned char size)
  {
    VccState* vccState = GetVccState();

    if (size != QUERY) {
      vccState->SystemState.RamSize = size;
    }

    return(vccState->SystemState.RamSize);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetSpeedThrottle(unsigned char throttle)
  {
    VccState* vccState = GetVccState();

    if (throttle != QUERY) {
      vccState->Throttle = throttle;
    }

    return(vccState->Throttle);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetTurboMode(unsigned char data)
  {
    VccState* vccState = GetVccState();

    vccState->SystemState.TurboSpeedFlag = (data & 1) + 1;

    SetClockSpeed(1);

    if (vccState->SystemState.DoubleSpeedFlag) {
      SetClockSpeed(vccState->SystemState.DoubleSpeedMultiplyer * vccState->SystemState.TurboSpeedFlag);
    }

    vccState->SystemState.CPUCurrentSpeed = .894;

    if (vccState->SystemState.DoubleSpeedFlag) {
      vccState->SystemState.CPUCurrentSpeed *= ((double)vccState->SystemState.DoubleSpeedMultiplyer * (double)vccState->SystemState.TurboSpeedFlag);
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned __cdecl CartLoad(void* dummy)
  {
    VccState* vccState = GetVccState();

    LoadCart(&(vccState->SystemState));

    vccState->SystemState.EmulationRunning = TRUE;
    vccState->DialogOpen = false;

    return(NULL);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl LoadPack(void)
  {
    unsigned threadID;

    VccState* vccState = GetVccState();

    if (vccState->DialogOpen) {
      return;
    }

    vccState->DialogOpen = true;

    _beginthreadex(NULL, 0, &CartLoad, CreateEvent(NULL, FALSE, FALSE, NULL), 0, &threadID);
  }
}

// LoadIniFile allows user to browse for an ini file and reloads the config from it.
extern "C" {
  __declspec(dllexport) void __cdecl LoadIniFile(void)
  {
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    VccState* vccState = GetVccState();

    GetIniFilePath(szFileName); // EJJ load current ini file path

    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = vccState->SystemState.WindowHandle;
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
      ReadIniFile(&(vccState->SystemState));                // Load it
      UpdateConfig(&(vccState->SystemState));

      vccState->SystemState.ResetPending = 2;
    }
  }
}

// Force keys up if main widow keyboard focus is lost.  Otherwise
// down keys will cause issues with OS-9 on return
// Send key up events to keyboard handler for saved keys
extern "C" {
  __declspec(dllexport) void __cdecl SendSavedKeyEvents() {
    VccState* vccState = GetVccState();

    if (vccState->SC_save1) {
      vccKeyboardHandleKey(vccState->KB_save1, vccState->SC_save1, kEventKeyUp);
    }

    if (vccState->SC_save2) {
      vccKeyboardHandleKey(vccState->KB_save2, vccState->SC_save2, kEventKeyUp);
    }

    vccState->SC_save1 = 0;
    vccState->SC_save2 = 0;
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
    VccState* vccState = GetVccState();

    MC6883Reset();
    MC6821_PiaReset();

    GetCPU()->CPUReset();

    GimeReset();
    MmuReset();
    CopyRom();
    ResetBus();

    vccState->SystemState.TurboSpeedFlag = 1;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl HardReset(SystemState* const systemState)
  {
    VccState* vccState = GetVccState();

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

    vccState->SystemState.TurboSpeedFlag = 1;

    ResetBus();
    SetClockSpeed(1);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl EmuLoop() {
    static float fps;
    static unsigned int frameCounter = 0;

    VccState* vccState = GetVccState();

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

        QuickLoad(&(vccState->SystemState), vccState->QuickLoadFile);
      }

      StartRender();

      for (uint8_t frames = 1; frames <= vccState->SystemState.FrameSkip; frames++)
      {
        frameCounter++;

        if (vccState->SystemState.ResetPending != 0) {
          switch (vccState->SystemState.ResetPending)
          {
          case 1:	//Soft Reset
            SoftReset();
            break;

          case 2:	//Hard Reset
            UpdateConfig(&(vccState->SystemState));
            DoCls(&(vccState->SystemState));
            HardReset(&(vccState->SystemState));

            break;

          case 3:
            DoCls(&(vccState->SystemState));
            break;

          case 4:
            UpdateConfig(&(vccState->SystemState));
            DoCls(&(vccState->SystemState));

            break;

          default:
            break;
          }

          vccState->SystemState.ResetPending = 0;
        }

        if (vccState->SystemState.EmulationRunning == 1) {
          fps += RenderFrame(&(vccState->SystemState));
        }
        else {
          fps += Static(&(vccState->SystemState));
        }
      }

      EndRender(vccState->SystemState.FrameSkip);

      fps /= vccState->SystemState.FrameSkip;

      GetModuleStatus(&(vccState->SystemState));

      char ttbuff[256];

      snprintf(ttbuff, sizeof(ttbuff), "Skip:%2.2i | FPS:%3.0f | %s @ %2.2fMhz| %s", vccState->SystemState.FrameSkip, fps, vccState->CpuName, vccState->SystemState.CPUCurrentSpeed, vccState->SystemState.StatusLine);

      SetStatusBarText(ttbuff, &(vccState->SystemState));

      if (vccState->Throttle) { //Do nothing untill the frame is over returning unused time to OS
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