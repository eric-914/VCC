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

#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <richedit.h>
#include <iostream>
#include <direct.h>
#include <assert.h>

#include "resource.h"
#include "config.h"
#include "tcc1014graphics.h"
#include "mc6821.h"
#include "audio.h"
#include "pakinterface.h"
#include "vcc.h"
#include "DirectDrawInterface.h"
#include "keyboard.h"
#include "cassette.h"
#include "shlobj.h"

#include "library\commandline.h"
#include "library\configdef.h"
#include "library\fileoperations.h"
#include "library\joystickinput.h"

using namespace std;

//
// forward declarations
//
unsigned char XY2Disp(unsigned char, unsigned char);
void Disp2XY(unsigned char*, unsigned char*, unsigned char);
unsigned char TranslateDisp2Scan(LRESULT x);
unsigned char TranslateScan2Disp(int x);
void buildTransDisp2ScanTable();

int SelectFile(char*);
void RefreshJoystickStatus(void);

#define MAXCARDS 12
LRESULT CALLBACK CpuConfig(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK AudioConfig(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MiscConfig(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DisplayConfig(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK InputConfig(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK JoyStickConfig(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TapeConfig(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BitBanger(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Paths(HWND, UINT, WPARAM, LPARAM);

//
//	global variables
//
static unsigned short int	Ramchoice[4] = { IDC_128K, IDC_512K, IDC_2M, IDC_8M };
static unsigned int	LeftJoystickEmulation[3] = { IDC_LEFTSTANDARD, IDC_LEFTTHIRES, IDC_LEFTCCMAX };
static unsigned int	RightJoystickEmulation[3] = { IDC_RIGHTSTANDARD, IDC_RIGHTTHRES, IDC_RIGHTCCMAX };
static unsigned short int	Cpuchoice[2] = { IDC_6809, IDC_6309 };
static unsigned short int	Monchoice[2] = { IDC_COMPOSITE, IDC_RGB };
static unsigned short int PaletteChoice[2] = { IDC_ORG_PALETTE, IDC_UPD_PALETTE };
static HICON CpuIcons[2], MonIcons[2], JoystickIcons[4];
static unsigned char temp = 0, temp2 = 0;
static char IniFileName[] = "Vcc.ini";
static char IniFilePath[MAX_PATH] = "";
static char TapeFileName[MAX_PATH] = "";
static char ExecDirectory[MAX_PATH] = "";
static char SerialCaptureFile[MAX_PATH] = "";
static char TextMode = 1, PrtMon = 0;;
static unsigned char NumberofJoysticks = 0;
TCHAR AppDataPath[MAX_PATH];

char OutBuffer[MAX_PATH] = "";
char AppName[MAX_LOADSTRING] = "";
STRConfig CurrentConfig;
static STRConfig TempConfig;

extern SystemState EmuState;

static unsigned int TapeCounter = 0;
static unsigned char Tmode = STOP;
char Tmodes[4][10] = { "STOP","PLAY","REC","STOP" };
static int NumberOfSoundCards = 0;

static JoyStick TempLeft, TempRight;

static SndCardList SoundCards[MAXCARDS];
static HWND hDlgBar = NULL, hDlgTape = NULL;

CHARFORMAT CounterText;
CHARFORMAT ModeText;

/*
  for displaying key name
*/
char* const keyNames[] = { "","ESC","1","2","3","4","5","6","7","8","9","0","-","=","BackSp","Tab","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","[","]","Bkslash",";","'","Comma",".","/","CapsLk","Shift","Ctrl","Alt","Space","Enter","Insert","Delete","Home","End","PgUp","PgDown","Left","Right","Up","Down","F1","F2" };

char* getKeyName(int x)
{
  return keyNames[x];
}

#define SCAN_TRANS_COUNT	84

unsigned char _TranslateDisp2Scan[SCAN_TRANS_COUNT];
unsigned char _TranslateScan2Disp[SCAN_TRANS_COUNT] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,32,38,20,33,35,40,36,24,30,31,42,43,55,52,16,34,19,21,22,23,25,26,27,45,46,0,51,44,41,39,18,37,17,29,28,47,48,49,51,0,53,54,50,66,67,0,0,0,0,0,0,0,0,0,0,58,64,60,0,62,0,63,0,59,65,61,56,57 };

#define TABS 8

static HWND g_hWndConfig[TABS]; // Moved this outside the initialization function so that other functions could access the window handles when necessary.

void LoadConfig(SystemState* systemState, CmdLineArguments cmdArg)
{
  HANDLE hr = NULL;
  int lasterror;

  buildTransDisp2ScanTable();

  LoadString(NULL, IDS_APP_TITLE, AppName, MAX_LOADSTRING);
  GetModuleFileName(NULL, ExecDirectory, MAX_PATH);
  FilePathRemoveFileSpec(ExecDirectory);

  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, AppDataPath))) {
    OutputDebugString(AppDataPath);
  }

  strcpy(CurrentConfig.PathtoExe, ExecDirectory);

  strcat(AppDataPath, "\\VCC");

  if (_mkdir(AppDataPath) != 0) {
    OutputDebugString("Unable to create VCC config folder.");
  }

  if (*cmdArg.IniFile) {
    GetFullPathNameA(cmdArg.IniFile, MAX_PATH, IniFilePath, 0);
  }
  else {
    strcpy(IniFilePath, AppDataPath);
    strcat(IniFilePath, "\\");
    strcat(IniFilePath, IniFileName);
  }

  systemState->ScanLines = 0;
  NumberOfSoundCards = GetSoundCardList(SoundCards);
  ReadIniFile();
  CurrentConfig.RebootNow = 0;
  UpdateConfig();
  RefreshJoystickStatus();
  SoundInit(EmuState.WindowHandle, SoundCards[CurrentConfig.SndOutDev].Guid, CurrentConfig.AudioRate);

  //  Try to open the config file.  Create it if necessary.  Abort if failure.
  hr = CreateFile(IniFilePath,
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

unsigned char WriteIniFile(void)
{
  POINT tp = GetCurWindowSize();
  CurrentConfig.Resize = 1;
  GetCurrentModule(CurrentConfig.ModulePath);
  FileValidatePath(CurrentConfig.ModulePath);
  FileValidatePath(CurrentConfig.ExternalBasicImage);

  WritePrivateProfileString("Version", "Release", AppName, IniFilePath);
  FileWritePrivateProfileInt("CPU", "DoubleSpeedClock", CurrentConfig.CPUMultiplyer, IniFilePath);
  FileWritePrivateProfileInt("CPU", "FrameSkip", CurrentConfig.FrameSkip, IniFilePath);
  FileWritePrivateProfileInt("CPU", "Throttle", CurrentConfig.SpeedThrottle, IniFilePath);
  FileWritePrivateProfileInt("CPU", "CpuType", CurrentConfig.CpuType, IniFilePath);
  FileWritePrivateProfileInt("CPU", "MaxOverClock", CurrentConfig.MaxOverclock, IniFilePath);

  WritePrivateProfileString("Audio", "SndCard", CurrentConfig.SoundCardName, IniFilePath);
  FileWritePrivateProfileInt("Audio", "Rate", CurrentConfig.AudioRate, IniFilePath);

  FileWritePrivateProfileInt("Video", "MonitorType", CurrentConfig.MonitorType, IniFilePath);
  FileWritePrivateProfileInt("Video", "PaletteType", CurrentConfig.PaletteType, IniFilePath);
  FileWritePrivateProfileInt("Video", "ScanLines", CurrentConfig.ScanLines, IniFilePath);
  FileWritePrivateProfileInt("Video", "AllowResize", CurrentConfig.Resize, IniFilePath);
  FileWritePrivateProfileInt("Video", "ForceAspect", CurrentConfig.Aspect, IniFilePath);
  FileWritePrivateProfileInt("Video", "RememberSize", CurrentConfig.RememberSize, IniFilePath);
  FileWritePrivateProfileInt("Video", "WindowSizeX", tp.x, IniFilePath);
  FileWritePrivateProfileInt("Video", "WindowSizeY", tp.y, IniFilePath);

  FileWritePrivateProfileInt("Memory", "RamSize", CurrentConfig.RamSize, IniFilePath);
  WritePrivateProfileString("Memory", "ExternalBasicImage", CurrentConfig.ExternalBasicImage, IniFilePath);

  FileWritePrivateProfileInt("Misc", "AutoStart", CurrentConfig.AutoStart, IniFilePath);
  FileWritePrivateProfileInt("Misc", "CartAutoStart", CurrentConfig.CartAutoStart, IniFilePath);
  FileWritePrivateProfileInt("Misc", "KeyMapIndex", CurrentConfig.KeyMap, IniFilePath);

  WritePrivateProfileString("Module", "OnBoot", CurrentConfig.ModulePath, IniFilePath);

  FileWritePrivateProfileInt("LeftJoyStick", "UseMouse", Left.UseMouse, IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Left", Left.Left, IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Right", Left.Right, IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Up", Left.Up, IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Down", Left.Down, IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Fire1", Left.Fire1, IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Fire2", Left.Fire2, IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "DiDevice", Left.DiDevice, IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "HiResDevice", Left.HiRes, IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "UseMouse", Right.UseMouse, IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Left", Right.Left, IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Right", Right.Right, IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Up", Right.Up, IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Down", Right.Down, IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Fire1", Right.Fire1, IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Fire2", Right.Fire2, IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "DiDevice", Right.DiDevice, IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "HiResDevice", Right.HiRes, IniFilePath);

  //  Flush inifile
  WritePrivateProfileString(NULL, NULL, NULL, IniFilePath);

  return(0);
}

unsigned char ReadIniFile(void)
{
  HANDLE hr = NULL;
  unsigned char Index = 0;
  POINT p;

  //Loads the config structure from the hard disk
  CurrentConfig.CPUMultiplyer = GetProfileByte("CPU", "DoubleSpeedClock", 2);
  CurrentConfig.FrameSkip = GetProfileByte("CPU", "FrameSkip", 1);
  CurrentConfig.SpeedThrottle = GetProfileByte("CPU", "Throttle", 1);
  CurrentConfig.CpuType = GetProfileByte("CPU", "CpuType", 0);
  CurrentConfig.MaxOverclock = GetProfileShort("CPU", "MaxOverClock", 227);

  CurrentConfig.AudioRate = GetProfileShort("Audio", "Rate", 3);
  GetPrivateProfileString("Audio", "SndCard", "", CurrentConfig.SoundCardName, 63, IniFilePath);

  CurrentConfig.MonitorType = GetProfileByte("Video", "MonitorType", 1);
  CurrentConfig.PaletteType = GetProfileByte("Video", "PaletteType", 1);
  CurrentConfig.ScanLines = GetProfileByte("Video", "ScanLines", 0);

  CurrentConfig.Resize = GetProfileByte("Video", "AllowResize", 0);
  CurrentConfig.Aspect = GetProfileByte("Video", "ForceAspect", 0);
  CurrentConfig.RememberSize = GetProfileShort("Video", "RememberSize", 0);
  CurrentConfig.WindowSizeX = GetProfileShort("Video", "WindowSizeX", 640);
  CurrentConfig.WindowSizeY = GetProfileShort("Video", "WindowSizeY", 480);
  CurrentConfig.AutoStart = GetProfileByte("Misc", "AutoStart", 1);
  CurrentConfig.CartAutoStart = GetProfileByte("Misc", "CartAutoStart", 1);

  CurrentConfig.RamSize = GetProfileByte("Memory", "RamSize", 1);
  GetProfileText("Memory", "ExternalBasicImage", "", CurrentConfig.ExternalBasicImage);

  GetProfileText("Module", "OnBoot", "", CurrentConfig.ModulePath);

  CurrentConfig.KeyMap = GetProfileByte("Misc", "KeyMapIndex", 0);

  if (CurrentConfig.KeyMap > 3) {
    CurrentConfig.KeyMap = 0;	//Default to DECB Mapping
  }

  vccKeyboardBuildRuntimeTable((keyboardlayout_e)CurrentConfig.KeyMap);

  FileCheckPath(CurrentConfig.ModulePath);
  FileCheckPath(CurrentConfig.ExternalBasicImage);

  Left.UseMouse = GetProfileByte("LeftJoyStick", "UseMouse", 1);
  Left.Left = GetProfileByte("LeftJoyStick", "Left", 75);
  Left.Right = GetProfileByte("LeftJoyStick", "Right", 77);
  Left.Up = GetProfileByte("LeftJoyStick", "Up", 72);
  Left.Down = GetProfileByte("LeftJoyStick", "Down", 80);
  Left.Fire1 = GetProfileByte("LeftJoyStick", "Fire1", 59);
  Left.Fire2 = GetProfileByte("LeftJoyStick", "Fire2", 60);
  Left.DiDevice = GetProfileByte("LeftJoyStick", "DiDevice", 0);
  Left.HiRes = GetProfileByte("LeftJoyStick", "HiResDevice", 0);
  Right.UseMouse = GetProfileByte("RightJoyStick", "UseMouse", 1);
  Right.Left = GetProfileByte("RightJoyStick", "Left", 75);
  Right.Right = GetProfileByte("RightJoyStick", "Right", 77);
  Right.Up = GetProfileByte("RightJoyStick", "Up", 72);
  Right.Down = GetProfileByte("RightJoyStick", "Down", 80);
  Right.Fire1 = GetProfileByte("RightJoyStick", "Fire1", 59);
  Right.Fire2 = GetProfileByte("RightJoyStick", "Fire2", 60);
  Right.DiDevice = GetProfileByte("RightJoyStick", "DiDevice", 0);
  Right.HiRes = GetProfileByte("RightJoyStick", "HiResDevice", 0);

  GetProfileText("DefaultPaths", "CassPath", "", CurrentConfig.CassPath);
  GetProfileText("DefaultPaths", "FloppyPath", "", CurrentConfig.FloppyPath);
  GetProfileText("DefaultPaths", "COCO3ROMPath", "", CurrentConfig.COCO3ROMPath);

  for (Index = 0; Index < NumberOfSoundCards; Index++) {
    if (!strcmp(SoundCards[Index].CardName, CurrentConfig.SoundCardName)) {
      CurrentConfig.SndOutDev = Index;
    }
  }

  TempConfig = CurrentConfig;
  InsertModule(CurrentConfig.ModulePath);	// Should this be here?
  CurrentConfig.Resize = 1; //Checkbox removed. Remove this from the ini? 

  if (CurrentConfig.RememberSize) {
    p.x = CurrentConfig.WindowSizeX;
    p.y = CurrentConfig.WindowSizeY;
    SetWindowSize(p);
  }
  else {
    p.x = 640;
    p.y = 480;
    SetWindowSize(p);
  }

  return(0);
}

char* BasicRomName(void)
{
  return(CurrentConfig.ExternalBasicImage);
}

void GetIniFilePath(char* path)
{
  strcpy(path, IniFilePath);
}

void SetIniFilePath(char* path)
{
  //  Path must be to an existing ini file
  strcpy(IniFilePath, path);
}

char* AppDirectory()
{
  // This only works after LoadConfig has been called
  return AppDataPath;
}

void UpdateConfig(void)
{
  SetPaletteType();
  SetResize(CurrentConfig.Resize);
  SetAspect(CurrentConfig.Aspect);
  SetScanLines(CurrentConfig.ScanLines);
  SetFrameSkip(CurrentConfig.FrameSkip);
  SetAutoStart(CurrentConfig.AutoStart);
  SetSpeedThrottle(CurrentConfig.SpeedThrottle);
  SetCPUMultiplyer(CurrentConfig.CPUMultiplyer);
  SetRamSize(CurrentConfig.RamSize);
  SetCpuType(CurrentConfig.CpuType);
  SetMonitorType(CurrentConfig.MonitorType);
  SetCartAutoStart(CurrentConfig.CartAutoStart);

  if (CurrentConfig.RebootNow) {
    DoReboot();
  }

  CurrentConfig.RebootNow = 0;
}

/**
 * Increase the overclock speed, as seen after a POKE 65497,0.
 * Valid values are [2,100].
 */
void IncreaseOverclockSpeed()
{
  if (TempConfig.CPUMultiplyer >= CurrentConfig.MaxOverclock)
  {
    return;
  }

  TempConfig.CPUMultiplyer = (unsigned char)(TempConfig.CPUMultiplyer + 1);

  // Send updates to the dialog if it's open.
  if (EmuState.ConfigDialog != NULL)
  {
    HWND hDlg = g_hWndConfig[1];
    SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, TempConfig.CPUMultiplyer);
    sprintf(OutBuffer, "%2.3f Mhz", (float)TempConfig.CPUMultiplyer * 0.894);
    SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(OutBuffer), (LPARAM)(LPCSTR)OutBuffer);
  }

  CurrentConfig = TempConfig;
  EmuState.ResetPending = 4; // Without this, changing the config does nothing.
}

/**
 * Decrease the overclock speed, as seen after a POKE 65497,0.
 *
 * Setting this value to 0 will make the emulator pause.  Hence the minimum of 2.
 */
void DecreaseOverclockSpeed()
{
  if (TempConfig.CPUMultiplyer == 2)
  {
    return;
  }

  TempConfig.CPUMultiplyer = (unsigned char)(TempConfig.CPUMultiplyer - 1);

  // Send updates to the dialog if it's open.
  if (EmuState.ConfigDialog != NULL)
  {
    HWND hDlg = g_hWndConfig[1];
    SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, TempConfig.CPUMultiplyer);
    sprintf(OutBuffer, "%2.3f Mhz", (float)TempConfig.CPUMultiplyer * 0.894);
    SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(OutBuffer), (LPARAM)(LPCSTR)OutBuffer);
  }

  CurrentConfig = TempConfig;
  EmuState.ResetPending = 4;
}

void UpdateSoundBar(unsigned short left, unsigned short right)
{
  if (hDlgBar == NULL) {
    return;
  }

  SendDlgItemMessage(hDlgBar, IDC_PROGRESSLEFT, PBM_SETPOS, left >> 8, 0);
  SendDlgItemMessage(hDlgBar, IDC_PROGRESSRIGHT, PBM_SETPOS, right >> 8, 0);
}

void UpdateTapeCounter(unsigned int counter, unsigned char tapeMode)
{
  if (hDlgTape == NULL) {
    return;
  }

  TapeCounter = counter;
  Tmode = tapeMode;
  sprintf(OutBuffer, "%i", TapeCounter);
  SendDlgItemMessage(hDlgTape, IDC_TCOUNT, WM_SETTEXT, strlen(OutBuffer), (LPARAM)(LPCSTR)OutBuffer);
  SendDlgItemMessage(hDlgTape, IDC_MODE, WM_SETTEXT, strlen(Tmodes[Tmode]), (LPARAM)(LPCSTR)Tmodes[Tmode]);
  GetTapeName(TapeFileName);
  FilePathStripPath(TapeFileName);
  SendDlgItemMessage(hDlgTape, IDC_TAPEFILE, WM_SETTEXT, strlen(TapeFileName), (LPARAM)(LPCSTR)TapeFileName);

  switch (Tmode)
  {
  case REC:
    SendDlgItemMessage(hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0xAF, 0, 0));
    break;

  case PLAY:
    SendDlgItemMessage(hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0xAF, 0));
    break;

  default:
    SendDlgItemMessage(hDlgTape, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
    break;
  }
}

int GetKeyboardLayout() {
  return(CurrentConfig.KeyMap);
}

int GetPaletteType() {
  return(CurrentConfig.PaletteType);
}

int GetRememberSize() {
  return((int)CurrentConfig.RememberSize);
}

void SetWindowSize(POINT p) {
  int width = p.x + 16;
  int height = p.y + 81;
  HWND handle = GetActiveWindow();
  ::SetWindowPos(handle, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

POINT GetIniWindowSize() {
  POINT out;

  out.x = CurrentConfig.WindowSizeX;
  out.y = CurrentConfig.WindowSizeY;

  return(out);
}

void GetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString) {
  GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, MAX_PATH, IniFilePath);
}

void SetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString) {
  WritePrivateProfileString(lpAppName, lpKeyName, lpString, IniFilePath);
}

unsigned short GetProfileShort(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
  return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, IniFilePath);
}

unsigned char GetProfileByte(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
  return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, IniFilePath);
}

LRESULT CALLBACK Config(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static char TabTitles[TABS][10] = { "Audio","CPU","Display","Keyboard","Joysticks","Misc","Tape","BitBanger" };
  static unsigned char TabCount = 0, SelectedTab = 0;
  static HWND hWndTabDialog;
  TCITEM Tabs;
  switch (message)
  {
  case WM_INITDIALOG:
    InitCommonControls();
    TempConfig = CurrentConfig;
    CpuIcons[0] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_MOTO);
    CpuIcons[1] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_HITACHI2);
    MonIcons[0] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_COMPOSITE);
    MonIcons[1] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_RGB);
    hWndTabDialog = GetDlgItem(hDlg, IDC_CONFIGTAB); //get handle of Tabbed Dialog
    //get handles to all the sub panels in the control
    g_hWndConfig[0] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_AUDIO), hWndTabDialog, (DLGPROC)AudioConfig);
    g_hWndConfig[1] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_CPU), hWndTabDialog, (DLGPROC)CpuConfig);
    g_hWndConfig[2] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_DISPLAY), hWndTabDialog, (DLGPROC)DisplayConfig);
    g_hWndConfig[3] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_INPUT), hWndTabDialog, (DLGPROC)InputConfig);
    g_hWndConfig[4] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_JOYSTICK), hWndTabDialog, (DLGPROC)JoyStickConfig);
    g_hWndConfig[5] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_MISC), hWndTabDialog, (DLGPROC)MiscConfig);
    g_hWndConfig[6] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_CASSETTE), hWndTabDialog, (DLGPROC)TapeConfig);
    g_hWndConfig[7] = CreateDialog(EmuState.WindowInstance, MAKEINTRESOURCE(IDD_BITBANGER), hWndTabDialog, (DLGPROC)BitBanger);

    //Set the title text for all tabs
    for (TabCount = 0;TabCount < TABS;TabCount++)
    {
      Tabs.mask = TCIF_TEXT | TCIF_IMAGE;
      Tabs.iImage = -1;
      Tabs.pszText = TabTitles[TabCount];
      TabCtrl_InsertItem(hWndTabDialog, TabCount, &Tabs);
    }

    TabCtrl_SetCurSel(hWndTabDialog, 0);	//Set Initial Tab to 0

    for (TabCount = 0;TabCount < TABS;TabCount++)	//Hide All the Sub Panels
      ShowWindow(g_hWndConfig[TabCount], SW_HIDE);

    SetWindowPos(g_hWndConfig[0], HWND_TOP, 10, 30, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    RefreshJoystickStatus();
    break;

  case WM_NOTIFY:
    if ((LOWORD(wParam)) == IDC_CONFIGTAB)
    {
      SelectedTab = TabCtrl_GetCurSel(hWndTabDialog);

      for (TabCount = 0;TabCount < TABS;TabCount++)
        ShowWindow(g_hWndConfig[TabCount], SW_HIDE);

      SetWindowPos(g_hWndConfig[SelectedTab], HWND_TOP, 10, 30, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    }
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
      hDlgBar = NULL;
      hDlgTape = NULL;
      EmuState.ResetPending = 4;
      if ((CurrentConfig.RamSize != TempConfig.RamSize) || (CurrentConfig.CpuType != TempConfig.CpuType))
        EmuState.ResetPending = 2;

      if ((CurrentConfig.SndOutDev != TempConfig.SndOutDev) || (CurrentConfig.AudioRate != TempConfig.AudioRate))
        SoundInit(EmuState.WindowHandle, SoundCards[TempConfig.SndOutDev].Guid, TempConfig.AudioRate);

      CurrentConfig = TempConfig;

      vccKeyboardBuildRuntimeTable((keyboardlayout_e)CurrentConfig.KeyMap);

      Right = TempRight;
      Left = TempLeft;
      SetStickNumbers(Left.DiDevice, Right.DiDevice);

      for (temp = 0; temp < TABS; temp++)
      {
        DestroyWindow(g_hWndConfig[temp]);
      }

#ifdef CONFIG_DIALOG_MODAL
      EndDialog(hDlg, LOWORD(wParam));
#else
      DestroyWindow(hDlg);
#endif
      EmuState.ConfigDialog = NULL;
      break;

    case IDAPPLY:
      EmuState.ResetPending = 4;

      if ((CurrentConfig.RamSize != TempConfig.RamSize) || (CurrentConfig.CpuType != TempConfig.CpuType)) {
        EmuState.ResetPending = 2;
      }

      if ((CurrentConfig.SndOutDev != TempConfig.SndOutDev) || (CurrentConfig.AudioRate != TempConfig.AudioRate)) {
        SoundInit(EmuState.WindowHandle, SoundCards[TempConfig.SndOutDev].Guid, TempConfig.AudioRate);
      }

      CurrentConfig = TempConfig;

      vccKeyboardBuildRuntimeTable((keyboardlayout_e)CurrentConfig.KeyMap);

      Right = TempRight;
      Left = TempLeft;
      SetStickNumbers(Left.DiDevice, Right.DiDevice);
      break;

    case IDCANCEL:
      for (temp = 0; temp < TABS; temp++)
      {
        DestroyWindow(g_hWndConfig[temp]);
      }

#ifdef CONFIG_DIALOG_MODAL
      EndDialog(hDlg, LOWORD(wParam));
#else
      DestroyWindow(hDlg);
#endif

      EmuState.ConfigDialog = NULL;
      break;
    }

    break;
  }

  return FALSE;
}

LRESULT CALLBACK CpuConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETRANGE, TRUE, MAKELONG(2, CurrentConfig.MaxOverclock));	//Maximum overclock
    sprintf(OutBuffer, "%2.3f Mhz", (float)TempConfig.CPUMultiplyer * .894);
    SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(OutBuffer), (LPARAM)(LPCSTR)OutBuffer);
    SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, TempConfig.CPUMultiplyer);

    for (temp = 0; temp <= 3; temp++) {
      SendDlgItemMessage(hDlg, Ramchoice[temp], BM_SETCHECK, (temp == TempConfig.RamSize), 0);
    }

    for (temp = 0; temp <= 1; temp++) {
      SendDlgItemMessage(hDlg, Cpuchoice[temp], BM_SETCHECK, (temp == TempConfig.CpuType), 0);
    }

    SendDlgItemMessage(hDlg, IDC_CPUICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)CpuIcons[TempConfig.CpuType]);
    break;

  case WM_HSCROLL:
    TempConfig.CPUMultiplyer = (unsigned char)SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_GETPOS, (WPARAM)0, (WPARAM)0);
    sprintf(OutBuffer, "%2.3f Mhz", (float)TempConfig.CPUMultiplyer * .894);
    SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(OutBuffer), (LPARAM)(LPCSTR)OutBuffer);
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_128K:
    case IDC_512K:
    case IDC_2M:
    case IDC_8M:
      for (temp = 0; temp <= 3; temp++) {
        if (LOWORD(wParam) == Ramchoice[temp])
        {
          for (temp2 = 0;temp2 <= 3;temp2++)
            SendDlgItemMessage(hDlg, Ramchoice[temp2], BM_SETCHECK, 0, 0);

          SendDlgItemMessage(hDlg, Ramchoice[temp], BM_SETCHECK, 1, 0);
          TempConfig.RamSize = temp;
        }
      }

      break;

    case IDC_6809:
    case IDC_6309:
      for (temp = 0; temp <= 1; temp++) {
        if (LOWORD(wParam) == Cpuchoice[temp])
        {
          for (temp2 = 0; temp2 <= 1; temp2++) {
            SendDlgItemMessage(hDlg, Cpuchoice[temp2], BM_SETCHECK, 0, 0);
          }

          SendDlgItemMessage(hDlg, Cpuchoice[temp], BM_SETCHECK, 1, 0);
          TempConfig.CpuType = temp;
          SendDlgItemMessage(hDlg, IDC_CPUICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)CpuIcons[TempConfig.CpuType]);
        }
      }

      break;
    }

    break;
  }

  return(0);
}

LRESULT CALLBACK MiscConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg, IDC_AUTOSTART, BM_SETCHECK, TempConfig.AutoStart, 0);
    SendDlgItemMessage(hDlg, IDC_AUTOCART, BM_SETCHECK, TempConfig.CartAutoStart, 0);
    break;

  case WM_COMMAND:
    TempConfig.AutoStart = (unsigned char)SendDlgItemMessage(hDlg, IDC_AUTOSTART, BM_GETCHECK, 0, 0);
    TempConfig.CartAutoStart = (unsigned char)SendDlgItemMessage(hDlg, IDC_AUTOCART, BM_GETCHECK, 0, 0);
    break;
  }

  return(0);
}

LRESULT CALLBACK TapeConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  CounterText.cbSize = sizeof(CHARFORMAT);
  CounterText.dwMask = CFM_BOLD | CFM_COLOR;
  CounterText.dwEffects = CFE_BOLD;
  CounterText.crTextColor = RGB(255, 255, 255);

  ModeText.cbSize = sizeof(CHARFORMAT);
  ModeText.dwMask = CFM_BOLD | CFM_COLOR;
  ModeText.dwEffects = CFE_BOLD;
  ModeText.crTextColor = RGB(255, 0, 0);

  switch (message)
  {
  case WM_INITDIALOG:
    TapeCounter = GetTapeCounter();
    sprintf(OutBuffer, "%i", TapeCounter);
    SendDlgItemMessage(hDlg, IDC_TCOUNT, WM_SETTEXT, strlen(OutBuffer), (LPARAM)(LPCSTR)OutBuffer);
    SendDlgItemMessage(hDlg, IDC_MODE, WM_SETTEXT, strlen(Tmodes[Tmode]), (LPARAM)(LPCSTR)Tmodes[Tmode]);
    GetTapeName(TapeFileName);
    SendDlgItemMessage(hDlg, IDC_TAPEFILE, WM_SETTEXT, strlen(TapeFileName), (LPARAM)(LPCSTR)TapeFileName);
    SendDlgItemMessage(hDlg, IDC_TCOUNT, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
    SendDlgItemMessage(hDlg, IDC_TCOUNT, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&CounterText);
    SendDlgItemMessage(hDlg, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
    SendDlgItemMessage(hDlg, IDC_MODE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&CounterText);
    hDlgTape = hDlg;
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_PLAY:
      Tmode = PLAY;
      SetTapeMode(Tmode);
      break;

    case IDC_REC:
      Tmode = REC;
      SetTapeMode(Tmode);
      break;

    case IDC_STOP:
      Tmode = STOP;
      SetTapeMode(Tmode);
      break;

    case IDC_EJECT:
      Tmode = EJECT;
      SetTapeMode(Tmode);
      break;

    case IDC_RESET:
      TapeCounter = 0;
      SetTapeCounter(TapeCounter);
      break;

    case IDC_TBROWSE:
      LoadTape();
      TapeCounter = 0;
      SetTapeCounter(TapeCounter);
      break;
    }

    break;
  }

  return(0);
}

LRESULT CALLBACK AudioConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  unsigned char Index = 0;

  switch (message)
  {
  case WM_INITDIALOG:
    hDlgBar = hDlg;	//Save the handle to update sound bars
    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETPOS, 0, 0);
    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETRANGE32, 0, 0x7F);
    SendDlgItemMessage(hDlg, IDC_PROGRESSRIGHT, PBM_SETPOS, 0, 0);
    SendDlgItemMessage(hDlg, IDC_PROGRESSRIGHT, PBM_SETRANGE32, 0, 0x7F);
    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETPOS, 0, 0);
    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETBARCOLOR, 0, 0xFFFF);
    SendDlgItemMessage(hDlg, IDC_PROGRESSRIGHT, PBM_SETBARCOLOR, 0, 0xFFFF);
    SendDlgItemMessage(hDlg, IDC_PROGRESSLEFT, PBM_SETBKCOLOR, 0, 0);
    SendDlgItemMessage(hDlg, IDC_PROGRESSRIGHT, PBM_SETBKCOLOR, 0, 0);

    for (Index = 0;Index < NumberOfSoundCards;Index++)
      SendDlgItemMessage(hDlg, IDC_SOUNDCARD, CB_ADDSTRING, (WPARAM)0, (LPARAM)SoundCards[Index].CardName);

    for (Index = 0;Index < 4;Index++) {
      SendDlgItemMessage(hDlg, IDC_RATE, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetRateList(Index));
    }

    SendDlgItemMessage(hDlg, IDC_RATE, CB_SETCURSEL, (WPARAM)TempConfig.AudioRate, (LPARAM)0);
    TempConfig.SndOutDev = 0;

    for (Index = 0;Index < NumberOfSoundCards;Index++) {
      if (!strcmp(SoundCards[Index].CardName, TempConfig.SoundCardName)) {
        TempConfig.SndOutDev = Index;
      }
    }

    SendDlgItemMessage(hDlg, IDC_SOUNDCARD, CB_SETCURSEL, (WPARAM)TempConfig.SndOutDev, (LPARAM)0);

    break;

  case WM_COMMAND:
    TempConfig.SndOutDev = (unsigned char)SendDlgItemMessage(hDlg, IDC_SOUNDCARD, CB_GETCURSEL, 0, 0);
    TempConfig.AudioRate = (unsigned char)SendDlgItemMessage(hDlg, IDC_RATE, CB_GETCURSEL, 0, 0);
    strcpy(TempConfig.SoundCardName, SoundCards[TempConfig.SndOutDev].CardName);

    break;
  }

  return(0);
}

LRESULT CALLBACK DisplayConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static bool isRGB;
  switch (message)
  {
  case WM_INITDIALOG:

    SendDlgItemMessage(hDlg, IDC_FRAMESKIP, TBM_SETRANGE, TRUE, MAKELONG(1, 6));
    SendDlgItemMessage(hDlg, IDC_SCANLINES, BM_SETCHECK, TempConfig.ScanLines, 0);
    SendDlgItemMessage(hDlg, IDC_THROTTLE, BM_SETCHECK, TempConfig.SpeedThrottle, 0);
    SendDlgItemMessage(hDlg, IDC_FRAMESKIP, TBM_SETPOS, TRUE, TempConfig.FrameSkip);
    SendDlgItemMessage(hDlg, IDC_RESIZE, BM_SETCHECK, TempConfig.Resize, 0);
    SendDlgItemMessage(hDlg, IDC_ASPECT, BM_SETCHECK, TempConfig.Aspect, 0);
    SendDlgItemMessage(hDlg, IDC_REMEMBER_SIZE, BM_SETCHECK, TempConfig.RememberSize, 0);
    sprintf(OutBuffer, "%i", TempConfig.FrameSkip);
    SendDlgItemMessage(hDlg, IDC_FRAMEDISPLAY, WM_SETTEXT, strlen(OutBuffer), (LPARAM)(LPCSTR)OutBuffer);

    for (temp = 0; temp <= 1; temp++) {
      SendDlgItemMessage(hDlg, Monchoice[temp], BM_SETCHECK, (temp == TempConfig.MonitorType), 0);
    }

    if (TempConfig.MonitorType == 1) { //If RGB monitor is chosen, gray out palette choice
      isRGB = TRUE;
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETDONTCLICK, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETDONTCLICK, 1, 0);
    }

    SendDlgItemMessage(hDlg, IDC_MONTYPE, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)MonIcons[TempConfig.MonitorType]);

    for (temp = 0; temp <= 1; temp++) {
      SendDlgItemMessage(hDlg, PaletteChoice[temp], BM_SETCHECK, (temp == TempConfig.PaletteType), 0);
    }

    break;

  case WM_HSCROLL:
    TempConfig.FrameSkip = (unsigned char)SendDlgItemMessage(hDlg, IDC_FRAMESKIP, TBM_GETPOS, (WPARAM)0, (WPARAM)0);
    sprintf(OutBuffer, "%i", TempConfig.FrameSkip);
    SendDlgItemMessage(hDlg, IDC_FRAMEDISPLAY, WM_SETTEXT, strlen(OutBuffer), (LPARAM)(LPCSTR)OutBuffer);
    break;

  case WM_COMMAND:
    TempConfig.Resize = 1;
    TempConfig.Aspect = (unsigned char)SendDlgItemMessage(hDlg, IDC_ASPECT, BM_GETCHECK, 0, 0);
    TempConfig.ScanLines = (unsigned char)SendDlgItemMessage(hDlg, IDC_SCANLINES, BM_GETCHECK, 0, 0);
    TempConfig.SpeedThrottle = (unsigned char)SendDlgItemMessage(hDlg, IDC_THROTTLE, BM_GETCHECK, 0, 0);
    TempConfig.RememberSize = (unsigned char)SendDlgItemMessage(hDlg, IDC_REMEMBER_SIZE, BM_GETCHECK, 0, 0);

    //POINT p = { 640,480 };
    switch (LOWORD(wParam))
    {
    case IDC_REMEMBER_SIZE:
      TempConfig.Resize = 1;
      SendDlgItemMessage(hDlg, IDC_RESIZE, BM_GETCHECK, 1, 0);
      break;

    case IDC_COMPOSITE:
      isRGB = FALSE;
      for (temp = 0; temp <= 1; temp++) { //This finds the current Monitor choice, then sets both buttons in the nested loop.
        if (LOWORD(wParam) == Monchoice[temp])
        {
          for (temp2 = 0; temp2 <= 1; temp2++)
            SendDlgItemMessage(hDlg, Monchoice[temp2], BM_SETCHECK, 0, 0);

          SendDlgItemMessage(hDlg, Monchoice[temp], BM_SETCHECK, 1, 0);
          TempConfig.MonitorType = temp;
        }
      }

      SendDlgItemMessage(hDlg, IDC_MONTYPE, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)MonIcons[TempConfig.MonitorType]);
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 0, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 0, 0);
      break;

    case IDC_RGB:
      isRGB = TRUE;

      for (temp = 0; temp <= 1; temp++) { //This finds the current Monitor choice, then sets both buttons in the nested loop.
        if (LOWORD(wParam) == Monchoice[temp])
        {
          for (temp2 = 0; temp2 <= 1; temp2++)
            SendDlgItemMessage(hDlg, Monchoice[temp2], BM_SETCHECK, 0, 0);

          SendDlgItemMessage(hDlg, Monchoice[temp], BM_SETCHECK, 1, 0);
          TempConfig.MonitorType = temp;
        }
      }

      SendDlgItemMessage(hDlg, IDC_MONTYPE, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)MonIcons[TempConfig.MonitorType]);
      //If RGB is chosen, disable palette buttons.
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETDONTCLICK, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETDONTCLICK, 1, 0);
      break;

    case IDC_ORG_PALETTE:
      if (!isRGB) {
        //Original Composite palette
        SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETCHECK, 0, 0);
        TempConfig.PaletteType = 0;
      }
      else {
        SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 1, 0);
      }
      break;

    case IDC_UPD_PALETTE:
      if (!isRGB) {
        //New Composite palette
        SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETCHECK, 0, 0);
        TempConfig.PaletteType = 1;
      }
      else {
        SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 1, 0);
      }
      break;
    }
    break;
  }

  return(0);
}

LRESULT CALLBACK InputConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
    // copy keyboard layout names to the pull-down menu
    for (int x = 0; x < kKBLayoutCount; x++)
    {
      SendDlgItemMessage(hDlg, IDC_KBCONFIG, CB_ADDSTRING, (WPARAM)0, (LPARAM)k_keyboardLayoutNames[x]);
    }

    // select the current layout
    SendDlgItemMessage(hDlg, IDC_KBCONFIG, CB_SETCURSEL, (WPARAM)CurrentConfig.KeyMap, (LPARAM)0);
    break;

  case WM_COMMAND:
    TempConfig.KeyMap = (unsigned char)SendDlgItemMessage(hDlg, IDC_KBCONFIG, CB_GETCURSEL, 0, 0);
    break;
  }

  return(0);
}

LRESULT CALLBACK JoyStickConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static int LeftJoyStick[6] = { IDC_LEFT_LEFT,IDC_LEFT_RIGHT,IDC_LEFT_UP,IDC_LEFT_DOWN,IDC_LEFT_FIRE1,IDC_LEFT_FIRE2 };
  static int RightJoyStick[6] = { IDC_RIGHT_LEFT,IDC_RIGHT_RIGHT,IDC_RIGHT_UP,IDC_RIGHT_DOWN,IDC_RIGHT_FIRE1,IDC_RIGHT_FIRE2 };
  static int LeftRadios[4] = { IDC_LEFT_KEYBOARD,IDC_LEFT_USEMOUSE,IDC_LEFTAUDIO,IDC_LEFTJOYSTICK };
  static int RightRadios[4] = { IDC_RIGHT_KEYBOARD,IDC_RIGHT_USEMOUSE,IDC_RIGHTAUDIO,IDC_RIGHTJOYSTICK };

  switch (message)
  {
  case WM_INITDIALOG:
    JoystickIcons[0] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_KEYBOARD);
    JoystickIcons[1] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_MOUSE);
    JoystickIcons[2] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_AUDIO);
    JoystickIcons[3] = LoadIcon(EmuState.WindowInstance, (LPCTSTR)IDI_JOYSTICK);

    for (temp = 0; temp < 68; temp++)
    {
      for (temp2 = 0; temp2 < 6; temp2++)
      {
        SendDlgItemMessage(hDlg, LeftJoyStick[temp2], CB_ADDSTRING, (WPARAM)0, (LPARAM)getKeyName(temp));
        SendDlgItemMessage(hDlg, RightJoyStick[temp2], CB_ADDSTRING, (WPARAM)0, (LPARAM)getKeyName(temp));
      }
    }

    for (temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, LeftJoyStick[temp]), (Left.UseMouse == 0));
    }

    for (temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, RightJoyStick[temp]), (Right.UseMouse == 0));
    }

    for (temp = 0; temp <= 2; temp++)
    {
      SendDlgItemMessage(hDlg, LeftJoystickEmulation[temp], BM_SETCHECK, (temp == Left.HiRes), 0);
      SendDlgItemMessage(hDlg, RightJoystickEmulation[temp], BM_SETCHECK, (temp == Right.HiRes), 0);
    }

    EnableWindow(GetDlgItem(hDlg, IDC_LEFTAUDIODEVICE), (Left.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTAUDIODEVICE), (Right.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_LEFTJOYSTICKDEVICE), (Left.UseMouse == 3));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTJOYSTICKDEVICE), (Right.UseMouse == 3));

    EnableWindow(GetDlgItem(hDlg, IDC_LEFTJOYSTICK), (NumberofJoysticks > 0));		//Grey the Joystick Radios if
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTJOYSTICK), (NumberofJoysticks > 0));	  //No Joysticks are present

    //populate joystick combo boxs
    for (unsigned char index = 0; index < NumberofJoysticks; index++)
    {
      SendDlgItemMessage(hDlg, IDC_RIGHTJOYSTICKDEVICE, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetStickName(index));
      SendDlgItemMessage(hDlg, IDC_LEFTJOYSTICKDEVICE, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetStickName(index));
    }

    SendDlgItemMessage(hDlg, IDC_RIGHTJOYSTICKDEVICE, CB_SETCURSEL, (WPARAM)Right.DiDevice, (LPARAM)0);
    SendDlgItemMessage(hDlg, IDC_LEFTJOYSTICKDEVICE, CB_SETCURSEL, (WPARAM)Left.DiDevice, (LPARAM)0);

    SendDlgItemMessage(hDlg, LeftJoyStick[0], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Left.Left), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[1], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Left.Right), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[2], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Left.Up), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[3], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Left.Down), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[4], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Left.Fire1), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[5], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Left.Fire2), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[0], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Right.Left), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[1], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Right.Right), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[2], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Right.Up), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[3], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Right.Down), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[4], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Right.Fire1), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[5], CB_SETCURSEL, (WPARAM)TranslateScan2Disp(Right.Fire2), (LPARAM)0);

    for (temp = 0; temp <= 3; temp++)
    {
      SendDlgItemMessage(hDlg, LeftRadios[temp], BM_SETCHECK, temp == Left.UseMouse, 0);
    }

    for (temp = 0; temp <= 3; temp++)
    {
      SendDlgItemMessage(hDlg, RightRadios[temp], BM_SETCHECK, temp == Right.UseMouse, 0);
    }

    TempLeft = Left;
    TempRight = Right;
    SendDlgItemMessage(hDlg, IDC_LEFTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)JoystickIcons[Left.UseMouse]);
    SendDlgItemMessage(hDlg, IDC_RIGHTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)JoystickIcons[Right.UseMouse]);
    break;

  case WM_COMMAND:
    for (temp = 0; temp <= 3; temp++)
    {
      if (LOWORD(wParam) == LeftRadios[temp])
      {
        for (temp2 = 0; temp2 <= 3; temp2++)
          SendDlgItemMessage(hDlg, LeftRadios[temp2], BM_SETCHECK, 0, 0);

        SendDlgItemMessage(hDlg, LeftRadios[temp], BM_SETCHECK, 1, 0);
        TempLeft.UseMouse = temp;
      }
    }

    for (temp = 0; temp <= 3; temp++) {
      if (LOWORD(wParam) == RightRadios[temp])
      {
        for (temp2 = 0; temp2 <= 3; temp2++) {
          SendDlgItemMessage(hDlg, RightRadios[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, RightRadios[temp], BM_SETCHECK, 1, 0);
        TempRight.UseMouse = temp;
      }
    }

    for (temp = 0; temp <= 2; temp++) {
      if (LOWORD(wParam) == LeftJoystickEmulation[temp])
      {
        for (temp2 = 0; temp2 <= 2; temp2++) {
          SendDlgItemMessage(hDlg, LeftJoystickEmulation[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, LeftJoystickEmulation[temp], BM_SETCHECK, 1, 0);
        TempLeft.HiRes = temp;
      }
    }

    for (temp = 0; temp <= 2; temp++)
    {
      if (LOWORD(wParam) == RightJoystickEmulation[temp])
      {
        for (temp2 = 0; temp2 <= 2; temp2++) {
          SendDlgItemMessage(hDlg, RightJoystickEmulation[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, RightJoystickEmulation[temp], BM_SETCHECK, 1, 0);
        TempRight.HiRes = temp;
      }
    }

    for (temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, LeftJoyStick[temp]), (TempLeft.UseMouse == 0));
    }

    for (temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, RightJoyStick[temp]), (TempRight.UseMouse == 0));
    }

    EnableWindow(GetDlgItem(hDlg, IDC_LEFTAUDIODEVICE), (TempLeft.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTAUDIODEVICE), (TempRight.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_LEFTJOYSTICKDEVICE), (TempLeft.UseMouse == 3));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTJOYSTICKDEVICE), (TempRight.UseMouse == 3));

    TempLeft.Left = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[0], CB_GETCURSEL, 0, 0));
    TempLeft.Right = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[1], CB_GETCURSEL, 0, 0));
    TempLeft.Up = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[2], CB_GETCURSEL, 0, 0));
    TempLeft.Down = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[3], CB_GETCURSEL, 0, 0));
    TempLeft.Fire1 = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[4], CB_GETCURSEL, 0, 0));
    TempLeft.Fire2 = TranslateDisp2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[5], CB_GETCURSEL, 0, 0));

    TempRight.Left = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[0], CB_GETCURSEL, 0, 0));
    TempRight.Right = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[1], CB_GETCURSEL, 0, 0));
    TempRight.Up = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[2], CB_GETCURSEL, 0, 0));
    TempRight.Down = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[3], CB_GETCURSEL, 0, 0));
    TempRight.Fire1 = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[4], CB_GETCURSEL, 0, 0));
    TempRight.Fire2 = TranslateDisp2Scan(SendDlgItemMessage(hDlg, RightJoyStick[5], CB_GETCURSEL, 0, 0));

    TempRight.DiDevice = (unsigned char)SendDlgItemMessage(hDlg, IDC_RIGHTJOYSTICKDEVICE, CB_GETCURSEL, 0, 0);
    TempLeft.DiDevice = (unsigned char)SendDlgItemMessage(hDlg, IDC_LEFTJOYSTICKDEVICE, CB_GETCURSEL, 0, 0);	//Fix Me;

    SendDlgItemMessage(hDlg, IDC_LEFTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)JoystickIcons[TempLeft.UseMouse]);
    SendDlgItemMessage(hDlg, IDC_RIGHTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)JoystickIcons[TempRight.UseMouse]);
    break;
  }

  return(0);
}

LRESULT CALLBACK BitBanger(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
    if (!strlen(SerialCaptureFile)) {
      strcpy(SerialCaptureFile, "No Capture File");
    }

    SendDlgItemMessage(hDlg, IDC_SERIALFILE, WM_SETTEXT, strlen(SerialCaptureFile), (LPARAM)(LPCSTR)SerialCaptureFile);
    SendDlgItemMessage(hDlg, IDC_LF, BM_SETCHECK, TextMode, 0);
    SendDlgItemMessage(hDlg, IDC_PRINTMON, BM_SETCHECK, PrtMon, 0);
    break;

  case WM_COMMAND:

    switch (LOWORD(wParam))
    {
    case IDC_OPEN:
      SelectFile(SerialCaptureFile);
      SendDlgItemMessage(hDlg, IDC_SERIALFILE, WM_SETTEXT, strlen(SerialCaptureFile), (LPARAM)(LPCSTR)SerialCaptureFile);
      break;

    case IDC_CLOSE:
      ClosePrintFile();
      strcpy(SerialCaptureFile, "No Capture File");
      SendDlgItemMessage(hDlg, IDC_SERIALFILE, WM_SETTEXT, strlen(SerialCaptureFile), (LPARAM)(LPCSTR)SerialCaptureFile);
      PrtMon = FALSE;
      SetMonState(PrtMon);
      SendDlgItemMessage(hDlg, IDC_PRINTMON, BM_SETCHECK, PrtMon, 0);
      break;

    case IDC_LF:
      TextMode = (char)SendDlgItemMessage(hDlg, IDC_LF, BM_GETCHECK, 0, 0);
      SetSerialParams(TextMode);
      break;

    case IDC_PRINTMON:
      PrtMon = (char)SendDlgItemMessage(hDlg, IDC_PRINTMON, BM_GETCHECK, 0, 0);
      SetMonState(PrtMon);
    }

    break;
  }

  return(0);
}

LRESULT CALLBACK Paths(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  return 1;
}

int SelectFile(char* filename)
{
  OPENFILENAME ofn;
  char Dummy[MAX_PATH] = "";
  char TempFileName[MAX_PATH] = "";
  char CapFilePath[MAX_PATH];
  GetProfileText("DefaultPaths", "CapFilePath", "", CapFilePath);

  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = EmuState.WindowHandle; // GetTopWindow(NULL);
  ofn.Flags = OFN_HIDEREADONLY;
  ofn.hInstance = GetModuleHandle(0);
  ofn.lpstrDefExt = "txt";
  ofn.lpstrFilter = "Text File\0*.txt\0\0";
  ofn.nFilterIndex = 0;					      // current filter index
  ofn.lpstrFile = TempFileName;		    // contains full path and filename on return
  ofn.nMaxFile = MAX_PATH;			      // sizeof lpstrFile
  ofn.lpstrFileTitle = NULL;				  // filename and extension only
  ofn.nMaxFileTitle = MAX_PATH;			  // sizeof lpstrFileTitle
  ofn.lpstrInitialDir = CapFilePath;  // initial directory
  ofn.lpstrTitle = "Open print capture file";		// title bar string

  if (GetOpenFileName(&ofn)) {
    if (!(OpenPrintFile(TempFileName))) {
      MessageBox(0, "Can't Open File", "Can't open the file specified.", 0);
    }

    string tmp = ofn.lpstrFile;
    size_t idx;
    idx = tmp.find_last_of("\\");
    tmp = tmp.substr(0, idx);
    strcpy(CapFilePath, tmp.c_str());

    if (CapFilePath != "") {
      WritePrivateProfileString("DefaultPaths", "CapFilePath", CapFilePath, IniFilePath);
    }
  }

  strcpy(filename, TempFileName);

  return(1);
}

unsigned char XY2Disp(unsigned char row, unsigned char column)
{
  switch (row)
  {
  case 0:
    return(0);

  case 1:
    return(1 + column);

  case 2:
    return(9 + column);

  case 4:
    return(17 + column);

  case 8:
    return (25 + column);

  case 16:
    return(33 + column);

  case 32:
    return (41 + column);

  case 64:
    return (49 + column);

  default:
    return (0);
  }
}

void Disp2XY(unsigned char* column, unsigned char* row, unsigned char display)
{
  unsigned char temp = 0;

  if (display == 0)
  {
    column = 0;
    row = 0;
    return;
  }

  display -= 1;
  temp = display & 56;
  temp = temp >> 3;
  *row = 1 << temp;
  *column = display & 7;
}

void RefreshJoystickStatus(void)
{
  bool temp = false;

  NumberofJoysticks = EnumerateJoysticks();

  for (unsigned char index = 0; index < NumberofJoysticks; index++) {
    temp = InitJoyStick(index);
  }

  if (Right.DiDevice > (NumberofJoysticks - 1))
    Right.DiDevice = 0;

  if (Left.DiDevice > (NumberofJoysticks - 1))
    Left.DiDevice = 0;

  SetStickNumbers(Left.DiDevice, Right.DiDevice);

  if (NumberofJoysticks == 0)	//Use Mouse input if no Joysticks present
  {
    if (Left.UseMouse == 3)
      Left.UseMouse = 1;

    if (Right.UseMouse == 3)
      Right.UseMouse = 1;
  }
}

unsigned char TranslateDisp2Scan(LRESULT x)
{
  assert(x >= 0 && x < SCAN_TRANS_COUNT);

  return _TranslateDisp2Scan[x];
}

unsigned char TranslateScan2Disp(int x)
{
  assert(x >= 0 && x < SCAN_TRANS_COUNT);

  return _TranslateScan2Disp[x];
}

void buildTransDisp2ScanTable()
{
  for (int Index = 0; Index < SCAN_TRANS_COUNT; Index++) {
    for (int Index2 = SCAN_TRANS_COUNT - 1; Index2 >= 0; Index2--) {
      if (Index2 == _TranslateScan2Disp[Index]) {
        _TranslateDisp2Scan[Index2] = (unsigned char)Index;
      }
    }
  }

  _TranslateDisp2Scan[0] = 0;

  // Left and Right Shift
  _TranslateDisp2Scan[51] = DIK_LSHIFT;
}
