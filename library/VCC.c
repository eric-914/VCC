#include "VCC.h"

#include "Config.h"
#include "Coco.h"

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

  p->hEMUThread = NULL;
  p->FlagEmuStop = TH_RUNNING;

  strcpy(p->CpuName, "CPUNAME");
  strcpy(p->AppName, "");

  return p;
}

extern "C" {
  __declspec(dllexport) void __cdecl Reboot(void)
  {
    GetVccState()->EmuState.ResetPending = 2;
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
    ofn.hwndOwner = vccState->EmuState.WindowHandle;
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

    vccState->EmuState.DoubleSpeedFlag = double_speed;

    if (vccState->EmuState.DoubleSpeedFlag) {
      SetClockSpeed(vccState->EmuState.DoubleSpeedMultiplyer * vccState->EmuState.TurboSpeedFlag);
    }

    vccState->EmuState.CPUCurrentSpeed = .894;

    if (vccState->EmuState.DoubleSpeedFlag) {
      vccState->EmuState.CPUCurrentSpeed *= ((double)vccState->EmuState.DoubleSpeedMultiplyer * (double)vccState->EmuState.TurboSpeedFlag);
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetCPUMultiplayer(unsigned char multiplayer)
  {
    VccState* vccState = GetVccState();

    if (multiplayer != QUERY)
    {
      vccState->EmuState.DoubleSpeedMultiplyer = multiplayer;

      SetCPUMultiplayerFlag(vccState->EmuState.DoubleSpeedFlag);
    }

    return(vccState->EmuState.DoubleSpeedMultiplyer);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetCpuType(unsigned char cpuType)
  {
    VccState* vccState = GetVccState();

    switch (cpuType)
    {
    case 0:
      vccState->EmuState.CpuType = 0;

      strcpy(vccState->CpuName, "MC6809");

      break;

    case 1:
      vccState->EmuState.CpuType = 1;

      strcpy(vccState->CpuName, "HD6309");

      break;
    }

    return(vccState->EmuState.CpuType);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetFrameSkip(unsigned char skip)
  {
    VccState* vccState = GetVccState();

    if (skip != QUERY) {
      vccState->EmuState.FrameSkip = skip;
    }

    return(vccState->EmuState.FrameSkip);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetRamSize(unsigned char size)
  {
    VccState* vccState = GetVccState();

    if (size != QUERY) {
      vccState->EmuState.RamSize = size;
    }

    return(vccState->EmuState.RamSize);
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

    vccState->EmuState.TurboSpeedFlag = (data & 1) + 1;

    SetClockSpeed(1);

    if (vccState->EmuState.DoubleSpeedFlag) {
      SetClockSpeed(vccState->EmuState.DoubleSpeedMultiplyer * vccState->EmuState.TurboSpeedFlag);
    }

    vccState->EmuState.CPUCurrentSpeed = .894;

    if (vccState->EmuState.DoubleSpeedFlag) {
      vccState->EmuState.CPUCurrentSpeed *= ((double)vccState->EmuState.DoubleSpeedMultiplyer * (double)vccState->EmuState.TurboSpeedFlag);
    }
  }
}
