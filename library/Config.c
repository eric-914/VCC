#include "di.version.h"
#include <dinput.h>
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

#include "systemstate.h"
#include "fileoperations.h"

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
  __declspec(dllexport) unsigned char __cdecl TranslateScan2Display(int x)
  {
    ConfigState* configState = GetConfigState();

    assert(x >= 0 && x < SCAN_TRANS_COUNT);

    return configState->TranslateScan2Disp[x];
  }
}


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
  __declspec(dllexport) char* __cdecl BasicRomName(void)
  {
    return(GetConfigState()->CurrentConfig.ExternalBasicImage);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GetIniFilePath(char* path)
  {
    strcpy(path, GetConfigState()->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetIniFilePath(char* path)
  {
    //  Path must be to an existing ini file
    strcpy(GetConfigState()->IniFilePath, path);
  }
}

extern "C" {
  __declspec(dllexport) char* __cdecl AppDirectory()
  {
    // This only works after LoadConfig has been called
    return GetConfigState()->AppDataPath;
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetCurrentKeyboardLayout() {
    return(GetConfigState()->CurrentConfig.KeyMap);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetPaletteType() {
    return(GetConfigState()->CurrentConfig.PaletteType);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetRememberSize() {
    return((int)(GetConfigState()->CurrentConfig.RememberSize));
  }
}

extern "C" {
  __declspec(dllexport) POINT __cdecl GetIniWindowSize() {
    POINT out = POINT();

    ConfigState* configState = GetConfigState();

    out.x = configState->CurrentConfig.WindowSizeX;
    out.y = configState->CurrentConfig.WindowSizeY;

    return(out);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString) {
    ConfigState* configState = GetConfigState();

    GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, MAX_PATH, configState->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString) {
    ConfigState* configState = GetConfigState();

    WritePrivateProfileString(lpAppName, lpKeyName, lpString, configState->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl GetProfileShort(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
    ConfigState* configState = GetConfigState();

    return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, configState->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl GetProfileByte(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
    ConfigState* configState = GetConfigState();

    return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, configState->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl TranslateDisplay2Scan(LRESULT x)
  {
    ConfigState* configState = GetConfigState();

    assert(x >= 0 && x < SCAN_TRANS_COUNT);

    return configState->TranslateDisp2Scan[x];
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl BuildTransDisp2ScanTable()
  {
    ConfigState* configState = GetConfigState();

    for (int i = 0; i < SCAN_TRANS_COUNT; i++) {
      for (int j = SCAN_TRANS_COUNT - 1; j >= 0; j--) {
        if (j == configState->TranslateScan2Disp[i]) {
          configState->TranslateDisp2Scan[j] = (unsigned char)i;
        }
      }
    }

    configState->TranslateDisp2Scan[0] = 0;

    // Left and Right Shift
    configState->TranslateDisp2Scan[51] = DIK_LSHIFT;
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
    ConfigState* configState = GetConfigState();

    if (configState->TempConfig.CPUMultiplyer == 2)
    {
      return;
    }

    configState->TempConfig.CPUMultiplyer = (unsigned char)(configState->TempConfig.CPUMultiplyer - 1);

    // Send updates to the dialog if it's open.
    if (systemState->ConfigDialog != NULL)
    {
      HWND hDlg = configState->hWndConfig[1];

      SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, configState->TempConfig.CPUMultiplyer);

      sprintf(configState->OutBuffer, "%2.3f Mhz", (float)(configState->TempConfig.CPUMultiplyer) * 0.894);

      SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));
    }

    configState->CurrentConfig = configState->TempConfig;

    systemState->ResetPending = 4;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl UpdateTapeCounter(unsigned int counter, unsigned char tapeMode)
  {
    ConfigState* configState = GetConfigState();

    if (configState->hDlgTape == NULL) {
      return;
    }

    configState->TapeCounter = counter;
    configState->Tmode = tapeMode;

    sprintf(configState->OutBuffer, "%i", configState->TapeCounter);

    SendDlgItemMessage(configState->hDlgTape, IDC_TCOUNT, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));
    SendDlgItemMessage(configState->hDlgTape, IDC_MODE, WM_SETTEXT, strlen(configState->Tmodes[configState->Tmode]), (LPARAM)(LPCSTR)(configState->Tmodes[configState->Tmode]));

    GetTapeName(configState->TapeFileName);
    FilePathStripPath(configState->TapeFileName);

    SendDlgItemMessage(configState->hDlgTape, IDC_TAPEFILE, WM_SETTEXT, strlen(configState->TapeFileName), (LPARAM)(LPCSTR)(configState->TapeFileName));

    switch (configState->Tmode)
    {
    case REC:
      SendDlgItemMessage(configState->hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0xAF, 0, 0));
      break;

    case PLAY:
      SendDlgItemMessage(configState->hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0xAF, 0));
      break;

    default:
      SendDlgItemMessage(configState->hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
      break;
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl WriteIniFile(void)
  {
    ConfigState* configState = GetConfigState();

    POINT tp = GetCurrentWindowSize();
    configState->CurrentConfig.Resize = 1;

    GetCurrentModule(configState->CurrentConfig.ModulePath);
    FileValidatePath(configState->CurrentConfig.ModulePath);
    FileValidatePath(configState->CurrentConfig.ExternalBasicImage);

    WritePrivateProfileString("Version", "Release", configState->AppName, configState->IniFilePath);

    FileWritePrivateProfileInt("CPU", "DoubleSpeedClock", configState->CurrentConfig.CPUMultiplyer, configState->IniFilePath);
    FileWritePrivateProfileInt("CPU", "FrameSkip", configState->CurrentConfig.FrameSkip, configState->IniFilePath);
    FileWritePrivateProfileInt("CPU", "Throttle", configState->CurrentConfig.SpeedThrottle, configState->IniFilePath);
    FileWritePrivateProfileInt("CPU", "CpuType", configState->CurrentConfig.CpuType, configState->IniFilePath);
    FileWritePrivateProfileInt("CPU", "MaxOverClock", configState->CurrentConfig.MaxOverclock, configState->IniFilePath);

    WritePrivateProfileString("Audio", "SndCard", configState->CurrentConfig.SoundCardName, configState->IniFilePath);
    FileWritePrivateProfileInt("Audio", "Rate", configState->CurrentConfig.AudioRate, configState->IniFilePath);

    FileWritePrivateProfileInt("Video", "MonitorType", configState->CurrentConfig.MonitorType, configState->IniFilePath);
    FileWritePrivateProfileInt("Video", "PaletteType", configState->CurrentConfig.PaletteType, configState->IniFilePath);
    FileWritePrivateProfileInt("Video", "ScanLines", configState->CurrentConfig.ScanLines, configState->IniFilePath);
    FileWritePrivateProfileInt("Video", "AllowResize", configState->CurrentConfig.Resize, configState->IniFilePath);
    FileWritePrivateProfileInt("Video", "ForceAspect", configState->CurrentConfig.Aspect, configState->IniFilePath);
    FileWritePrivateProfileInt("Video", "RememberSize", configState->CurrentConfig.RememberSize, configState->IniFilePath);
    FileWritePrivateProfileInt("Video", "WindowSizeX", tp.x, configState->IniFilePath);
    FileWritePrivateProfileInt("Video", "WindowSizeY", tp.y, configState->IniFilePath);

    FileWritePrivateProfileInt("Memory", "RamSize", configState->CurrentConfig.RamSize, configState->IniFilePath);

    WritePrivateProfileString("Memory", "ExternalBasicImage", configState->CurrentConfig.ExternalBasicImage, configState->IniFilePath);

    FileWritePrivateProfileInt("Misc", "AutoStart", configState->CurrentConfig.AutoStart, configState->IniFilePath);
    FileWritePrivateProfileInt("Misc", "CartAutoStart", configState->CurrentConfig.CartAutoStart, configState->IniFilePath);
    FileWritePrivateProfileInt("Misc", "KeyMapIndex", configState->CurrentConfig.KeyMap, configState->IniFilePath);

    WritePrivateProfileString("Module", "OnBoot", configState->CurrentConfig.ModulePath, configState->IniFilePath);

    JoystickState* joystickState = GetJoystickState();

    FileWritePrivateProfileInt("LeftJoyStick", "UseMouse", joystickState->Left.UseMouse, configState->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Left", joystickState->Left.Left, configState->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Right", joystickState->Left.Right, configState->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Up", joystickState->Left.Up, configState->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Down", joystickState->Left.Down, configState->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Fire1", joystickState->Left.Fire1, configState->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "Fire2", joystickState->Left.Fire2, configState->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "DiDevice", joystickState->Left.DiDevice, configState->IniFilePath);
    FileWritePrivateProfileInt("LeftJoyStick", "HiResDevice", joystickState->Left.HiRes, configState->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "UseMouse", joystickState->Right.UseMouse, configState->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Left", joystickState->Right.Left, configState->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Right", joystickState->Right.Right, configState->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Up", joystickState->Right.Up, configState->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Down", joystickState->Right.Down, configState->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Fire1", joystickState->Right.Fire1, configState->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "Fire2", joystickState->Right.Fire2, configState->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "DiDevice", joystickState->Right.DiDevice, configState->IniFilePath);
    FileWritePrivateProfileInt("RightJoyStick", "HiResDevice", joystickState->Right.HiRes, configState->IniFilePath);

    //  Flush inifile
    WritePrivateProfileString(NULL, NULL, NULL, configState->IniFilePath);

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
    ConfigState* configState = GetConfigState();

    if (configState->TempConfig.CPUMultiplyer >= configState->CurrentConfig.MaxOverclock)
    {
      return;
    }

    configState->TempConfig.CPUMultiplyer = (unsigned char)(configState->TempConfig.CPUMultiplyer + 1);

    // Send updates to the dialog if it's open.
    if (systemState->ConfigDialog != NULL)
    {
      HWND hDlg = configState->hWndConfig[1];

      SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, configState->TempConfig.CPUMultiplyer);

      sprintf(configState->OutBuffer, "%2.3f Mhz", (float)(configState->TempConfig.CPUMultiplyer) * 0.894);

      SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));
    }

    configState->CurrentConfig = configState->TempConfig;

    systemState->ResetPending = 4; // Without this, changing the config does nothing.
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl UpdateSoundBar(unsigned short left, unsigned short right)
  {
    ConfigState* configState = GetConfigState();

    if (configState->hDlgBar == NULL) {
      return;
    }

    SendDlgItemMessage(configState->hDlgBar, IDC_PROGRESSLEFT, PBM_SETPOS, left >> 8, 0);
    SendDlgItemMessage(configState->hDlgBar, IDC_PROGRESSRIGHT, PBM_SETPOS, right >> 8, 0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl RefreshJoystickStatus(void)
  {
    bool temp = false;

    ConfigState* configState = GetConfigState();
    JoystickState* joystickState = GetJoystickState();

    configState->NumberofJoysticks = EnumerateJoysticks();

    for (unsigned char index = 0; index < configState->NumberofJoysticks; index++) {
      temp = InitJoyStick(index);
    }

    if (joystickState->Right.DiDevice > (configState->NumberofJoysticks - 1)) {
      joystickState->Right.DiDevice = 0;
    }

    if (joystickState->Left.DiDevice > (configState->NumberofJoysticks - 1)) {
      joystickState->Left.DiDevice = 0;
    }

    SetStickNumbers(joystickState->Left.DiDevice, joystickState->Right.DiDevice);

    if (configState->NumberofJoysticks == 0)	//Use Mouse input if no Joysticks present
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

    ConfigState* configState = GetConfigState();

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
      if (!(OpenPrintFile(tempFileName))) {
        MessageBox(0, "Can't Open File", "Can't open the file specified.", 0);
      }

      string tmp = ofn.lpstrFile;
      size_t idx = tmp.find_last_of("\\");

      tmp = tmp.substr(0, idx);

      strcpy(capFilePath, tmp.c_str());

      if (capFilePath != "") {
        WritePrivateProfileString("DefaultPaths", "CapFilePath", capFilePath, configState->IniFilePath);
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

    ConfigState* configState = GetConfigState();

    //Loads the config structure from the hard disk
    configState->CurrentConfig.CPUMultiplyer = GetProfileByte("CPU", "DoubleSpeedClock", 2);
    configState->CurrentConfig.FrameSkip = GetProfileByte("CPU", "FrameSkip", 1);
    configState->CurrentConfig.SpeedThrottle = GetProfileByte("CPU", "Throttle", 1);
    configState->CurrentConfig.CpuType = GetProfileByte("CPU", "CpuType", 0);
    configState->CurrentConfig.MaxOverclock = GetProfileShort("CPU", "MaxOverClock", 227);

    configState->CurrentConfig.AudioRate = GetProfileShort("Audio", "Rate", 3);

    GetPrivateProfileString("Audio", "SndCard", "", configState->CurrentConfig.SoundCardName, 63, configState->IniFilePath);

    configState->CurrentConfig.MonitorType = GetProfileByte("Video", "MonitorType", 1);
    configState->CurrentConfig.PaletteType = GetProfileByte("Video", "PaletteType", 1);
    configState->CurrentConfig.ScanLines = GetProfileByte("Video", "ScanLines", 0);

    configState->CurrentConfig.Resize = GetProfileByte("Video", "AllowResize", 0);
    configState->CurrentConfig.Aspect = GetProfileByte("Video", "ForceAspect", 0);
    configState->CurrentConfig.RememberSize = GetProfileShort("Video", "RememberSize", 0);
    configState->CurrentConfig.WindowSizeX = GetProfileShort("Video", "WindowSizeX", 640);
    configState->CurrentConfig.WindowSizeY = GetProfileShort("Video", "WindowSizeY", 480);
    configState->CurrentConfig.AutoStart = GetProfileByte("Misc", "AutoStart", 1);
    configState->CurrentConfig.CartAutoStart = GetProfileByte("Misc", "CartAutoStart", 1);

    configState->CurrentConfig.RamSize = GetProfileByte("Memory", "RamSize", 1);

    GetProfileText("Memory", "ExternalBasicImage", "", configState->CurrentConfig.ExternalBasicImage);

    GetProfileText("Module", "OnBoot", "", configState->CurrentConfig.ModulePath);

    configState->CurrentConfig.KeyMap = GetProfileByte("Misc", "KeyMapIndex", 0);

    if (configState->CurrentConfig.KeyMap > 3) {
      configState->CurrentConfig.KeyMap = 0;	//Default to DECB Mapping
    }

    vccKeyboardBuildRuntimeTable((keyboardlayout_e)(configState->CurrentConfig.KeyMap));

    FileCheckPath(configState->CurrentConfig.ModulePath);
    FileCheckPath(configState->CurrentConfig.ExternalBasicImage);

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

    GetProfileText("DefaultPaths", "CassPath", "", configState->CurrentConfig.CassPath);
    GetProfileText("DefaultPaths", "FloppyPath", "", configState->CurrentConfig.FloppyPath);
    GetProfileText("DefaultPaths", "COCO3ROMPath", "", configState->CurrentConfig.COCO3ROMPath);

    for (index = 0; index < configState->NumberOfSoundCards; index++) {
      if (!strcmp(configState->SoundCards[index].CardName, configState->CurrentConfig.SoundCardName)) {
        configState->CurrentConfig.SndOutDev = index;
      }
    }

    configState->TempConfig = configState->CurrentConfig;

    InsertModule(systemState, configState->CurrentConfig.ModulePath);	// Should this be here?

    configState->CurrentConfig.Resize = 1; //Checkbox removed. Remove this from the ini? 

    if (configState->CurrentConfig.RememberSize) {
      p.x = configState->CurrentConfig.WindowSizeX;
      p.y = configState->CurrentConfig.WindowSizeY;

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