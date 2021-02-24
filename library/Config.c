#include "di.version.h"
#include <dinput.h>
#include <direct.h>
#include <assert.h>
#include <ShlObj.h>
#include <string>

#include "../resources/resource.h"

#include "Config.h"
#include "DirectDraw.h"
#include "PAKInterface.h"
#include "Keyboard.h"
#include "Cassette.h"
#include "Joystick.h"
#include "MC6821.h"
#include "VCC.h"
#include "Graphics.h"
#include "Audio.h"

#include "systemstate.h"
#include "fileoperations.h"
#include "commandline.h"

#include "macros.h"

using namespace std;

const unsigned short int Cpuchoice[2] = { IDC_6809, IDC_6309 };
const unsigned short int Monchoice[2] = { IDC_COMPOSITE, IDC_RGB };
const unsigned short int PaletteChoice[2] = { IDC_ORG_PALETTE, IDC_UPD_PALETTE };
const unsigned short int Ramchoice[4] = { IDC_128K, IDC_512K, IDC_2M, IDC_8M };
const unsigned int LeftJoystickEmulation[3] = { IDC_LEFTSTANDARD, IDC_LEFTTHIRES, IDC_LEFTCCMAX };
const unsigned int RightJoystickEmulation[3] = { IDC_RIGHTSTANDARD, IDC_RIGHTTHRES, IDC_RIGHTCCMAX };
const char Tmodes[4][10] = { "STOP","PLAY","REC","STOP" };
const unsigned char TranslateScan2Disp[SCAN_TRANS_COUNT] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,32,38,20,33,35,40,36,24,30,31,42,43,55,52,16,34,19,21,22,23,25,26,27,45,46,0,51,44,41,39,18,37,17,29,28,47,48,49,51,0,53,54,50,66,67,0,0,0,0,0,0,0,0,0,0,58,64,60,0,62,0,63,0,59,65,61,56,57 };

ConfigState* InitializeInstance(ConfigState*);

static ConfigState* instance = InitializeInstance(new ConfigState());

extern "C" {
  __declspec(dllexport) ConfigState* __cdecl GetConfigState() {
    return instance;
  }
}

ConfigState* InitializeInstance(ConfigState* p) {
  p->NumberOfSoundCards = 0;
  p->NumberofJoysticks = 0;
  p->PrtMon = 0;
  p->TapeCounter = 0;
  p->TextMode = 1;
  p->Tmode = STOP;

  p->hDlgBar = NULL;
  p->hDlgTape = NULL;

  strcpy(p->AppName, "");
  strcpy(p->ExecDirectory, "");
  strcpy(p->IniFilePath, "");
  strcpy(p->OutBuffer, "");
  strcpy(p->SerialCaptureFile, "");
  strcpy(p->TapeFileName, "");

  ARRAYCOPY(Cpuchoice);
  ARRAYCOPY(LeftJoystickEmulation);
  ARRAYCOPY(Monchoice);
  ARRAYCOPY(PaletteChoice);
  ARRAYCOPY(Ramchoice);
  ARRAYCOPY(RightJoystickEmulation);
  ARRAYCOPY(TranslateScan2Disp);

  STRARRAYCOPY(Tmodes);

  return p;
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl TranslateScan2Display(int x)
  {
    assert(x >= 0 && x < SCAN_TRANS_COUNT);

    return instance->TranslateScan2Disp[x];
  }
}

extern "C" {
  __declspec(dllexport) char* __cdecl BasicRomName(void)
  {
    return(instance->CurrentConfig.ExternalBasicImage);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GetIniFilePath(char* path)
  {
    strcpy(path, instance->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetIniFilePath(char* path)
  {
    //  Path must be to an existing ini file
    strcpy(instance->IniFilePath, path);
  }
}

extern "C" {
  __declspec(dllexport) char* __cdecl AppDirectory()
  {
    // This only works after LoadConfig has been called
    return instance->AppDataPath;
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetCurrentKeyboardLayout() {
    return(instance->CurrentConfig.KeyMap);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetPaletteType() {
    return(instance->CurrentConfig.PaletteType);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetRememberSize() {
    return((int)(instance->CurrentConfig.RememberSize));
  }
}

extern "C" {
  __declspec(dllexport) POINT __cdecl GetIniWindowSize() {
    POINT out = POINT();

    out.x = instance->CurrentConfig.WindowSizeX;
    out.y = instance->CurrentConfig.WindowSizeY;

    return(out);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString) {
    GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, MAX_PATH, instance->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString) {
    WritePrivateProfileString(lpAppName, lpKeyName, lpString, instance->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl GetProfileShort(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
    return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, instance->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl GetProfileByte(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
    return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, instance->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl TranslateDisplay2Scan(LRESULT x)
  {
    assert(x >= 0 && x < SCAN_TRANS_COUNT);

    return instance->TranslateDisp2Scan[x];
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl BuildTransDisp2ScanTable()
  {
    for (int i = 0; i < SCAN_TRANS_COUNT; i++) {
      for (int j = SCAN_TRANS_COUNT - 1; j >= 0; j--) {
        if (j == instance->TranslateScan2Disp[i]) {
          instance->TranslateDisp2Scan[j] = (unsigned char)i;
        }
      }
    }

    instance->TranslateDisp2Scan[0] = 0;

    // Left and Right Shift
    instance->TranslateDisp2Scan[51] = DIK_LSHIFT;
  }
}

/**
 * Decrease the overclock speed, as seen after a POKE 65497,0.
 *
 * Setting this value to 0 will make the emulator pause.  Hence the minimum of 2.
 */
extern "C" {
  __declspec(dllexport) void __cdecl DecreaseOverclockSpeed(SystemState* systemState)
  {
    if (instance->TempConfig.CPUMultiplyer == 2)
    {
      return;
    }

    instance->TempConfig.CPUMultiplyer = (unsigned char)(instance->TempConfig.CPUMultiplyer - 1);

    // Send updates to the dialog if it's open.
    if (systemState->ConfigDialog != NULL)
    {
      HWND hDlg = instance->hWndConfig[1];

      SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, instance->TempConfig.CPUMultiplyer);

      sprintf(instance->OutBuffer, "%2.3f Mhz", (float)(instance->TempConfig.CPUMultiplyer) * 0.894);

      SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(instance->OutBuffer), (LPARAM)(LPCSTR)(instance->OutBuffer));
    }

    instance->CurrentConfig = instance->TempConfig;

    systemState->ResetPending = 4;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl UpdateTapeCounter(unsigned int counter, unsigned char tapeMode)
  {
    if (instance->hDlgTape == NULL) {
      return;
    }

    instance->TapeCounter = counter;
    instance->Tmode = tapeMode;

    sprintf(instance->OutBuffer, "%i", instance->TapeCounter);

    SendDlgItemMessage(instance->hDlgTape, IDC_TCOUNT, WM_SETTEXT, strlen(instance->OutBuffer), (LPARAM)(LPCSTR)(instance->OutBuffer));
    SendDlgItemMessage(instance->hDlgTape, IDC_MODE, WM_SETTEXT, strlen(instance->Tmodes[instance->Tmode]), (LPARAM)(LPCSTR)(instance->Tmodes[instance->Tmode]));

    GetTapeName(instance->TapeFileName);
    FilePathStripPath(instance->TapeFileName);

    SendDlgItemMessage(instance->hDlgTape, IDC_TAPEFILE, WM_SETTEXT, strlen(instance->TapeFileName), (LPARAM)(LPCSTR)(instance->TapeFileName));

    switch (instance->Tmode)
    {
    case REC:
      SendDlgItemMessage(instance->hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0xAF, 0, 0));
      break;

    case PLAY:
      SendDlgItemMessage(instance->hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0xAF, 0));
      break;

    default:
      SendDlgItemMessage(instance->hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
      break;
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl WriteIniFile(void)
  {
    POINT tp = GetCurrentWindowSize();

    instance->CurrentConfig.Resize = 1;

    GetCurrentModule(instance->CurrentConfig.ModulePath);
    FileValidatePath(instance->CurrentConfig.ModulePath);
    FileValidatePath(instance->CurrentConfig.ExternalBasicImage);

    WritePrivateProfileString("Version", "Release", instance->AppName, instance->IniFilePath);

    FileWritePrivateProfileInt("CPU", "DoubleSpeedClock", instance->CurrentConfig.CPUMultiplyer, instance->IniFilePath);
    FileWritePrivateProfileInt("CPU", "FrameSkip", instance->CurrentConfig.FrameSkip, instance->IniFilePath);
    FileWritePrivateProfileInt("CPU", "Throttle", instance->CurrentConfig.SpeedThrottle, instance->IniFilePath);
    FileWritePrivateProfileInt("CPU", "CpuType", instance->CurrentConfig.CpuType, instance->IniFilePath);
    FileWritePrivateProfileInt("CPU", "MaxOverClock", instance->CurrentConfig.MaxOverclock, instance->IniFilePath);

    WritePrivateProfileString("Audio", "SndCard", instance->CurrentConfig.SoundCardName, instance->IniFilePath);
    FileWritePrivateProfileInt("Audio", "Rate", instance->CurrentConfig.AudioRate, instance->IniFilePath);

    FileWritePrivateProfileInt("Video", "MonitorType", instance->CurrentConfig.MonitorType, instance->IniFilePath);
    FileWritePrivateProfileInt("Video", "PaletteType", instance->CurrentConfig.PaletteType, instance->IniFilePath);
    FileWritePrivateProfileInt("Video", "ScanLines", instance->CurrentConfig.ScanLines, instance->IniFilePath);
    FileWritePrivateProfileInt("Video", "AllowResize", instance->CurrentConfig.Resize, instance->IniFilePath);
    FileWritePrivateProfileInt("Video", "ForceAspect", instance->CurrentConfig.Aspect, instance->IniFilePath);
    FileWritePrivateProfileInt("Video", "RememberSize", instance->CurrentConfig.RememberSize, instance->IniFilePath);
    FileWritePrivateProfileInt("Video", "WindowSizeX", tp.x, instance->IniFilePath);
    FileWritePrivateProfileInt("Video", "WindowSizeY", tp.y, instance->IniFilePath);

    FileWritePrivateProfileInt("Memory", "RamSize", instance->CurrentConfig.RamSize, instance->IniFilePath);

    WritePrivateProfileString("Memory", "ExternalBasicImage", instance->CurrentConfig.ExternalBasicImage, instance->IniFilePath);

    FileWritePrivateProfileInt("Misc", "AutoStart", instance->CurrentConfig.AutoStart, instance->IniFilePath);
    FileWritePrivateProfileInt("Misc", "CartAutoStart", instance->CurrentConfig.CartAutoStart, instance->IniFilePath);
    FileWritePrivateProfileInt("Misc", "KeyMapIndex", instance->CurrentConfig.KeyMap, instance->IniFilePath);

    WritePrivateProfileString("Module", "OnBoot", instance->CurrentConfig.ModulePath, instance->IniFilePath);

    JoystickState* joystickState = GetJoystickState();

    FileWritePrivateProfileInt("LeftJoyStick", "UseMouse", joystickState->Left.UseMouse, instance->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Left", joystickState->Left.Left, instance->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Right", joystickState->Left.Right, instance->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Up", joystickState->Left.Up, instance->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Down", joystickState->Left.Down, instance->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Fire1", joystickState->Left.Fire1, instance->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Fire2", joystickState->Left.Fire2, instance->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "DiDevice", joystickState->Left.DiDevice, instance->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "HiResDevice", joystickState->Left.HiRes, instance->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "UseMouse", joystickState->Right.UseMouse, instance->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Left", joystickState->Right.Left, instance->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Right", joystickState->Right.Right, instance->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Up", joystickState->Right.Up, instance->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Down", joystickState->Right.Down, instance->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Fire1", joystickState->Right.Fire1, instance->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Fire2", joystickState->Right.Fire2, instance->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "DiDevice", joystickState->Right.DiDevice, instance->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "HiResDevice", joystickState->Right.HiRes, instance->IniFilePath);

    //  Flush inifile
    WritePrivateProfileString(NULL, NULL, NULL, instance->IniFilePath);

    return(0);
  }
}

/**
 * Increase the overclock speed, as seen after a POKE 65497,0.
 * Valid values are [2,100].
 */
extern "C" {
  __declspec(dllexport) void __cdecl IncreaseOverclockSpeed(SystemState* systemState)
  {
    if (instance->TempConfig.CPUMultiplyer >= instance->CurrentConfig.MaxOverclock)
    {
      return;
    }

    instance->TempConfig.CPUMultiplyer = (unsigned char)(instance->TempConfig.CPUMultiplyer + 1);

    // Send updates to the dialog if it's open.
    if (systemState->ConfigDialog != NULL)
    {
      HWND hDlg = instance->hWndConfig[1];

      SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, instance->TempConfig.CPUMultiplyer);

      sprintf(instance->OutBuffer, "%2.3f Mhz", (float)(instance->TempConfig.CPUMultiplyer) * 0.894);

      SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(instance->OutBuffer), (LPARAM)(LPCSTR)(instance->OutBuffer));
    }

    instance->CurrentConfig = instance->TempConfig;

    systemState->ResetPending = 4; // Without this, changing the config does nothing.
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl UpdateSoundBar(unsigned short left, unsigned short right)
  {
    if (instance->hDlgBar == NULL) {
      return;
    }

    SendDlgItemMessage(instance->hDlgBar, IDC_PROGRESSLEFT, PBM_SETPOS, left >> 8, 0);
    SendDlgItemMessage(instance->hDlgBar, IDC_PROGRESSRIGHT, PBM_SETPOS, right >> 8, 0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl RefreshJoystickStatus(void)
  {
    bool temp = false;

    JoystickState* joystickState = GetJoystickState();

    instance->NumberofJoysticks = EnumerateJoysticks();

    for (unsigned char index = 0; index < instance->NumberofJoysticks; index++) {
      temp = InitJoyStick(index);
    }

    if (joystickState->Right.DiDevice > (instance->NumberofJoysticks - 1)) {
      joystickState->Right.DiDevice = 0;
    }

    if (joystickState->Left.DiDevice > (instance->NumberofJoysticks - 1)) {
      joystickState->Left.DiDevice = 0;
    }

    SetStickNumbers(joystickState->Left.DiDevice, joystickState->Right.DiDevice);

    if (instance->NumberofJoysticks == 0)	//Use Mouse input if no Joysticks present
    {
      if (joystickState->Left.UseMouse == 3) {
        joystickState->Left.UseMouse = 1;
      }

      if (joystickState->Right.UseMouse == 3) {
        joystickState->Right.UseMouse = 1;
      }
    }
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl SelectFile(SystemState* systemState, char* filename)
  {
    OPENFILENAME ofn;
    char dummy[MAX_PATH] = "";
    char tempFileName[MAX_PATH] = "";
    char capFilePath[MAX_PATH];

    GetProfileText("DefaultPaths", "CapFilePath", "", capFilePath);

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = systemState->WindowHandle; // GetTopWindow(NULL);
    ofn.Flags = OFN_HIDEREADONLY;
    ofn.hInstance = GetModuleHandle(0);
    ofn.lpstrDefExt = "txt";
    ofn.lpstrFilter = "Text File\0*.txt\0\0";
    ofn.nFilterIndex = 0;					      // current filter index
    ofn.lpstrFile = tempFileName;		    // contains full path and filename on return
    ofn.nMaxFile = MAX_PATH;			      // sizeof lpstrFile
    ofn.lpstrFileTitle = NULL;				  // filename and extension only
    ofn.nMaxFileTitle = MAX_PATH;			  // sizeof lpstrFileTitle
    ofn.lpstrInitialDir = capFilePath;  // initial directory
    ofn.lpstrTitle = "Open print capture file";		// title bar string

    if (GetOpenFileName(&ofn)) {
      if (!(MC6821_OpenPrintFile(tempFileName))) {
        MessageBox(0, "Can't Open File", "Can't open the file specified.", 0);
      }

      string tmp = ofn.lpstrFile;
      size_t idx = tmp.find_last_of("\\");

      tmp = tmp.substr(0, idx);

      strcpy(capFilePath, tmp.c_str());

      if (capFilePath != "") {
        WritePrivateProfileString("DefaultPaths", "CapFilePath", capFilePath, instance->IniFilePath);
      }
    }

    strcpy(filename, tempFileName);

    return(1);
  }
}

void SetWindowSize(POINT p) {
  int width = p.x + 16;
  int height = p.y + 81;

  HWND handle = GetActiveWindow();

  SetWindowPos(handle, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl ReadIniFile(SystemState* systemState)
  {
    HANDLE hr = NULL;
    POINT p = POINT();
    unsigned char index = 0;

    //Loads the config structure from the hard disk
    instance->CurrentConfig.CPUMultiplyer = GetProfileByte("CPU", "DoubleSpeedClock", 2);
    instance->CurrentConfig.FrameSkip = GetProfileByte("CPU", "FrameSkip", 1);
    instance->CurrentConfig.SpeedThrottle = GetProfileByte("CPU", "Throttle", 1);
    instance->CurrentConfig.CpuType = GetProfileByte("CPU", "CpuType", 0);
    instance->CurrentConfig.MaxOverclock = GetProfileShort("CPU", "MaxOverClock", 227);

    instance->CurrentConfig.AudioRate = GetProfileShort("Audio", "Rate", 3);

    GetPrivateProfileString("Audio", "SndCard", "", instance->CurrentConfig.SoundCardName, 63, instance->IniFilePath);

    instance->CurrentConfig.MonitorType = GetProfileByte("Video", "MonitorType", 1);
    instance->CurrentConfig.PaletteType = GetProfileByte("Video", "PaletteType", 1);
    instance->CurrentConfig.ScanLines = GetProfileByte("Video", "ScanLines", 0);

    instance->CurrentConfig.Resize = GetProfileByte("Video", "AllowResize", 0);
    instance->CurrentConfig.Aspect = GetProfileByte("Video", "ForceAspect", 0);
    instance->CurrentConfig.RememberSize = GetProfileShort("Video", "RememberSize", 0);
    instance->CurrentConfig.WindowSizeX = GetProfileShort("Video", "WindowSizeX", 640);
    instance->CurrentConfig.WindowSizeY = GetProfileShort("Video", "WindowSizeY", 480);
    instance->CurrentConfig.AutoStart = GetProfileByte("Misc", "AutoStart", 1);
    instance->CurrentConfig.CartAutoStart = GetProfileByte("Misc", "CartAutoStart", 1);

    instance->CurrentConfig.RamSize = GetProfileByte("Memory", "RamSize", 1);

    GetProfileText("Memory", "ExternalBasicImage", "", instance->CurrentConfig.ExternalBasicImage);

    GetProfileText("Module", "OnBoot", "", instance->CurrentConfig.ModulePath);

    instance->CurrentConfig.KeyMap = GetProfileByte("Misc", "KeyMapIndex", 0);

    if (instance->CurrentConfig.KeyMap > 3) {
      instance->CurrentConfig.KeyMap = 0;	//Default to DECB Mapping
    }

    vccKeyboardBuildRuntimeTable((keyboardlayout_e)(instance->CurrentConfig.KeyMap));

    FileCheckPath(instance->CurrentConfig.ModulePath);
    FileCheckPath(instance->CurrentConfig.ExternalBasicImage);

    JoystickState* joystickState = GetJoystickState();

    joystickState->Left.UseMouse = GetProfileByte("LeftJoyStick", "UseMouse", 1);
    joystickState->Left.Left = GetProfileByte("LeftJoyStick", "Left", 75);
    joystickState->Left.Right = GetProfileByte("LeftJoyStick", "Right", 77);
    joystickState->Left.Up = GetProfileByte("LeftJoyStick", "Up", 72);
    joystickState->Left.Down = GetProfileByte("LeftJoyStick", "Down", 80);
    joystickState->Left.Fire1 = GetProfileByte("LeftJoyStick", "Fire1", 59);
    joystickState->Left.Fire2 = GetProfileByte("LeftJoyStick", "Fire2", 60);
    joystickState->Left.DiDevice = GetProfileByte("LeftJoyStick", "DiDevice", 0);
    joystickState->Left.HiRes = GetProfileByte("LeftJoyStick", "HiResDevice", 0);
    joystickState->Right.UseMouse = GetProfileByte("RightJoyStick", "UseMouse", 1);
    joystickState->Right.Left = GetProfileByte("RightJoyStick", "Left", 75);
    joystickState->Right.Right = GetProfileByte("RightJoyStick", "Right", 77);
    joystickState->Right.Up = GetProfileByte("RightJoyStick", "Up", 72);
    joystickState->Right.Down = GetProfileByte("RightJoyStick", "Down", 80);
    joystickState->Right.Fire1 = GetProfileByte("RightJoyStick", "Fire1", 59);
    joystickState->Right.Fire2 = GetProfileByte("RightJoyStick", "Fire2", 60);
    joystickState->Right.DiDevice = GetProfileByte("RightJoyStick", "DiDevice", 0);
    joystickState->Right.HiRes = GetProfileByte("RightJoyStick", "HiResDevice", 0);

    GetProfileText("DefaultPaths", "CassPath", "", instance->CurrentConfig.CassPath);
    GetProfileText("DefaultPaths", "FloppyPath", "", instance->CurrentConfig.FloppyPath);
    GetProfileText("DefaultPaths", "COCO3ROMPath", "", instance->CurrentConfig.COCO3ROMPath);

    for (index = 0; index < instance->NumberOfSoundCards; index++) {
      if (!strcmp(instance->SoundCards[index].CardName, instance->CurrentConfig.SoundCardName)) {
        instance->CurrentConfig.SndOutDev = index;
      }
    }

    instance->TempConfig = instance->CurrentConfig;

    InsertModule(systemState, instance->CurrentConfig.ModulePath);	// Should this be here?

    instance->CurrentConfig.Resize = 1; //Checkbox removed. Remove this from the ini? 

    if (instance->CurrentConfig.RememberSize) {
      p.x = instance->CurrentConfig.WindowSizeX;
      p.y = instance->CurrentConfig.WindowSizeY;

      SetWindowSize(p);
    }
    else {
      p.x = 640;
      p.y = 480;

      SetWindowSize(p);
    }

    return(0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl UpdateConfig(SystemState* systemState)
  {
    SetPaletteType();
    SetResize(instance->CurrentConfig.Resize);
    SetAspect(instance->CurrentConfig.Aspect);
    SetScanLines(systemState, instance->CurrentConfig.ScanLines);
    SetFrameSkip(instance->CurrentConfig.FrameSkip);
    SetAutoStart(instance->CurrentConfig.AutoStart);
    SetSpeedThrottle(instance->CurrentConfig.SpeedThrottle);
    SetCPUMultiplayer(instance->CurrentConfig.CPUMultiplyer);
    SetRamSize(instance->CurrentConfig.RamSize);
    SetCpuType(instance->CurrentConfig.CpuType);
    SetMonitorType(instance->CurrentConfig.MonitorType);
    MC6821_SetCartAutoStart(instance->CurrentConfig.CartAutoStart);

    if (instance->CurrentConfig.RebootNow) {
      Reboot();
    }

    instance->CurrentConfig.RebootNow = 0;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl LoadConfig(SystemState* systemState, CmdLineArguments cmdArg)
  {
    HANDLE hr = NULL;
    int lasterror;
    char iniFileName[] = "Vcc.ini";

    BuildTransDisp2ScanTable();

    LoadString(systemState->Resources, IDS_APP_TITLE, instance->AppName, MAX_LOADSTRING);
    GetModuleFileName(NULL, instance->ExecDirectory, MAX_PATH);

    FilePathRemoveFileSpec(instance->ExecDirectory);

    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, instance->AppDataPath))) {
      OutputDebugString(instance->AppDataPath);
    }

    strcpy(instance->CurrentConfig.PathtoExe, instance->ExecDirectory);
    strcat(instance->AppDataPath, "\\VCC");

    if (_mkdir(instance->AppDataPath) != 0) {
      OutputDebugString("Unable to create VCC config folder.");
    }

    if (*cmdArg.IniFile) {
      GetFullPathNameA(cmdArg.IniFile, MAX_PATH, instance->IniFilePath, 0);
    }
    else {
      strcpy(instance->IniFilePath, instance->AppDataPath);
      strcat(instance->IniFilePath, "\\");
      strcat(instance->IniFilePath, iniFileName);
    }

    systemState->ScanLines = 0;

    instance->NumberOfSoundCards = GetSoundCardList(instance->SoundCards);

    ReadIniFile(systemState);

    instance->CurrentConfig.RebootNow = 0;

    UpdateConfig(systemState);
    RefreshJoystickStatus();

    SoundInit(systemState->WindowHandle, instance->SoundCards[instance->CurrentConfig.SndOutDev].Guid, instance->CurrentConfig.AudioRate);

    //  Try to open the config file.  Create it if necessary.  Abort if failure.
    hr = CreateFile(instance->IniFilePath,
      GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
      NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    lasterror = GetLastError();

    if (hr == INVALID_HANDLE_VALUE) { // Fatal could not open ini file
      MessageBox(0, "Could not open ini file", "Error", 0);

      exit(0);
    }
    else {
      CloseHandle(hr);

      if (lasterror != ERROR_ALREADY_EXISTS) WriteIniFile();  //!=183
    }
  }
}
