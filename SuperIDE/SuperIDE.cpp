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
#include "stdio.h"
#include <iostream>

#include "resource.h" 
#include "defines.h"
#include "Superide.h"
#include "idebus.h"
#include "cloud9.h"
#include "logger.h"

#include "..\fileops\fileops.h"

static char FileName[MAX_PATH]{ 0 };
static char IniFile[MAX_PATH]{ 0 };
static char SuperIDEPath[MAX_PATH];
typedef unsigned char (*MEMREAD8)(unsigned short);
typedef void (*MEMWRITE8)(unsigned char, unsigned short);
typedef void (*ASSERTINTERRUPT) (unsigned char, unsigned char);
typedef void (*DMAMEMPOINTERS) (MEMREAD8, MEMWRITE8);
typedef void (*DYNAMICMENUCALLBACK)(char*, int, int);
static void (*AssertInt)(unsigned char, unsigned char) = NULL;
static unsigned char (*MemRead8)(unsigned short);
static void (*MemWrite8)(unsigned char, unsigned short);
static unsigned char* Memory = NULL;
static void (*DynamicMenuCallback)(char*, int, int) = NULL;
static unsigned char BaseAddress = 0x50;
void BuildDynaMenu(void);
LRESULT CALLBACK Config(HWND, UINT, WPARAM, LPARAM);
void Select_Disk(unsigned char);
void SaveConfig();
void LoadConfig();
unsigned char BaseTable[4] = { 0x40,0x50,0x60,0x70 };

static unsigned char BaseAddr = 1, ClockEnabled = 1, ClockReadOnly = 1;
static unsigned char DataLatch = 0;
static HINSTANCE g_hinstDLL;

using namespace std;

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpReserved)   // reserved
{
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    g_hinstDLL = hinstDLL;
    break;

  case DLL_PROCESS_DETACH:
    break;
  }

  return TRUE;
}

extern "C"
{
  __declspec(dllexport) void ModuleName(char* moduleName, char* catNumber, DYNAMICMENUCALLBACK menuCallback)
  {
    LoadString(g_hinstDLL, IDS_MODULE_NAME, moduleName, MAX_LOADSTRING);
    LoadString(g_hinstDLL, IDS_CATNUMBER, catNumber, MAX_LOADSTRING);
    DynamicMenuCallback = menuCallback;
    IdeInit();

    if (DynamicMenuCallback != NULL)
      BuildDynaMenu();
  }
}

extern "C"
{
  __declspec(dllexport) void PackPortWrite(unsigned char port, unsigned char data)
  {
    if ((port >= BaseAddress) & (port <= (BaseAddress + 8)))
      switch (port - BaseAddress)
      {
      case 0x0:
        IdeRegWrite(port - BaseAddress, (DataLatch << 8) + data);
        break;

      case 0x8:
        DataLatch = data & 0xFF;		//Latch
        break;

      default:
        IdeRegWrite(port - BaseAddress, data);
        break;
      }
  }
}

extern "C"
{
  __declspec(dllexport) unsigned char PackPortRead(unsigned char port)
  {
    unsigned char RetVal = 0;
    unsigned short Temp = 0;

    if (((port == 0x78) | (port == 0x79) | (port == 0x7C)) & ClockEnabled)
      RetVal = ReadTime(port);

    if ((port >= BaseAddress) & (port <= (BaseAddress + 8)))
      switch (port - BaseAddress)
      {
      case 0x0:
        Temp = IdeRegRead(port - BaseAddress);

        RetVal = Temp & 0xFF;
        DataLatch = Temp >> 8;
        break;

      case 0x8:
        RetVal = DataLatch;		//Latch
        break;

      default:
        RetVal = IdeRegRead(port - BaseAddress) & 0xFF;
        break;
      }

    return(RetVal);
  }
}

extern "C"
{
  __declspec(dllexport) void ModuleStatus(char* status)
  {
    DiskStatus(status);
  }
}

extern "C"
{
  __declspec(dllexport) void ModuleConfig(unsigned char menuId)
  {
    switch (menuId)
    {
    case 10:
      Select_Disk(MASTER);
      BuildDynaMenu();
      SaveConfig();
      break;

    case 11:
      DropDisk(MASTER);
      BuildDynaMenu();
      SaveConfig();
      break;

    case 12:
      Select_Disk(SLAVE);
      BuildDynaMenu();
      SaveConfig();
      break;

    case 13:
      DropDisk(SLAVE);
      BuildDynaMenu();
      SaveConfig();
      break;

    case 14:
      DialogBox(g_hinstDLL, (LPCTSTR)IDD_CONFIG, NULL, (DLGPROC)Config);
      break;
    }
  }
}

extern "C"
{
  __declspec(dllexport) void SetIniPath(char* iniFilePath)
  {
    strcpy(IniFile, iniFilePath);
    LoadConfig();
  }
}

void BuildDynaMenu(void)
{
  char TempMsg[512] = "";
  char TempBuf[MAX_PATH] = "";
  DynamicMenuCallback("", 0, 0);
  DynamicMenuCallback("", 6000, 0);
  DynamicMenuCallback("IDE Master", 6000, HEAD);
  DynamicMenuCallback("Insert", 5010, SLAVE);
  QueryDisk(MASTER, TempBuf);
  strcpy(TempMsg, "Eject: ");
  PathStripPath(TempBuf);
  strcat(TempMsg, TempBuf);

  DynamicMenuCallback(TempMsg, 5011, SLAVE);
  DynamicMenuCallback("IDE Slave", 6000, HEAD);
  DynamicMenuCallback("Insert", 5012, SLAVE);
  QueryDisk(SLAVE, TempBuf);
  strcpy(TempMsg, "Eject: ");
  PathStripPath(TempBuf);
  strcat(TempMsg, TempBuf);
  DynamicMenuCallback(TempMsg, 5013, SLAVE);
  DynamicMenuCallback("IDE Config", 5014, STANDALONE);
  DynamicMenuCallback("", 1, 0);
}

LRESULT CALLBACK Config(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  unsigned char BTemp = 0;

  switch (message)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg, IDC_CLOCK, BM_SETCHECK, ClockEnabled, 0);
    SendDlgItemMessage(hDlg, IDC_READONLY, BM_SETCHECK, ClockReadOnly, 0);
    SendDlgItemMessage(hDlg, IDC_BASEADDR, CB_ADDSTRING, NULL, (LPARAM)"40");
    SendDlgItemMessage(hDlg, IDC_BASEADDR, CB_ADDSTRING, NULL, (LPARAM)"50");
    SendDlgItemMessage(hDlg, IDC_BASEADDR, CB_ADDSTRING, NULL, (LPARAM)"60");
    SendDlgItemMessage(hDlg, IDC_BASEADDR, CB_ADDSTRING, NULL, (LPARAM)"70");
    SendDlgItemMessage(hDlg, IDC_BASEADDR, CB_SETCURSEL, BaseAddr, (LPARAM)0);
    EnableWindow(GetDlgItem(hDlg, IDC_CLOCK), TRUE);

    if (BaseAddr == 3)
    {
      ClockEnabled = 0;
      SendDlgItemMessage(hDlg, IDC_CLOCK, BM_SETCHECK, ClockEnabled, 0);
      EnableWindow(GetDlgItem(hDlg, IDC_CLOCK), FALSE);
    }

    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
      BaseAddr = (unsigned char)SendDlgItemMessage(hDlg, IDC_BASEADDR, CB_GETCURSEL, 0, 0);
      ClockEnabled = (unsigned char)SendDlgItemMessage(hDlg, IDC_CLOCK, BM_GETCHECK, 0, 0);
      ClockReadOnly = (unsigned char)SendDlgItemMessage(hDlg, IDC_READONLY, BM_GETCHECK, 0, 0);
      BaseAddress = BaseTable[BaseAddr & 3];
      SetClockWrite(!ClockReadOnly);
      SaveConfig();
      EndDialog(hDlg, LOWORD(wParam));

      break;

    case IDCANCEL:
      EndDialog(hDlg, LOWORD(wParam));
      break;

    case IDC_CLOCK:
      break;

    case IDC_READONLY:
      break;

    case IDC_BASEADDR:
      BTemp = (unsigned char)SendDlgItemMessage(hDlg, IDC_BASEADDR, CB_GETCURSEL, 0, 0);
      EnableWindow(GetDlgItem(hDlg, IDC_CLOCK), TRUE);

      if (BTemp == 3)
      {
        ClockEnabled = 0;
        SendDlgItemMessage(hDlg, IDC_CLOCK, BM_SETCHECK, ClockEnabled, 0);
        EnableWindow(GetDlgItem(hDlg, IDC_CLOCK), FALSE);
      }

      break;
    }
  }

  return(0);
}

void Select_Disk(unsigned char disk)
{
  OPENFILENAME ofn;

  char TempFileName[MAX_PATH] = "";

  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = NULL;
  ofn.Flags = OFN_HIDEREADONLY;
  ofn.hInstance = GetModuleHandle(0);
  ofn.lpstrDefExt = "IMG";
  ofn.lpstrFilter = "Hard Disk Images\0*.img;*.vhd;*.os9\0All files\0*.*\0\0";	// filter string "Disks\0*.DSK\0\0";
  ofn.nFilterIndex = 0;								          // current filter index
  ofn.lpstrFile = TempFileName;					        // contains full path and filename on return
  ofn.nMaxFile = MAX_PATH;						          // sizeof lpstrFile
  ofn.lpstrFileTitle = NULL;							      // filename and extension only
  ofn.nMaxFileTitle = MAX_PATH;						      // sizeof lpstrFileTitle
  ofn.lpstrInitialDir = SuperIDEPath;						// initial directory
  ofn.lpstrTitle = "Mount IDE hard Disk Image";	// title bar string

  if (GetOpenFileName(&ofn)) {
    if (!(MountDisk(TempFileName, disk)))
      MessageBox(0, "Can't Open File", "Can't open the Image specified.", 0);

    string tmp = ofn.lpstrFile;
    size_t idx;
    idx = tmp.find_last_of("\\");
    tmp = tmp.substr(0, idx);
    strcpy(SuperIDEPath, tmp.c_str());
  }
}

void SaveConfig(void)
{
  unsigned char Index = 0;
  char ModName[MAX_LOADSTRING] = "";

  LoadString(g_hinstDLL, IDS_MODULE_NAME, ModName, MAX_LOADSTRING);
  QueryDisk(MASTER, FileName);
  WritePrivateProfileString(ModName, "Master", FileName, IniFile);
  QueryDisk(SLAVE, FileName);
  WritePrivateProfileString(ModName, "Slave", FileName, IniFile);
  WritePrivateProfileInt(ModName, "BaseAddr", BaseAddr, IniFile);
  WritePrivateProfileInt(ModName, "ClkEnable", ClockEnabled, IniFile);
  WritePrivateProfileInt(ModName, "ClkRdOnly", ClockReadOnly, IniFile);

  if (SuperIDEPath != "") { WritePrivateProfileString("DefaultPaths", "SuperIDEPath", SuperIDEPath, IniFile); }
}

void LoadConfig(void)
{
  char ModName[MAX_LOADSTRING] = "";
  unsigned char Index = 0;
  char Temp[16] = "";
  char DiskName[MAX_PATH] = "";
  unsigned int RetVal = 0;
  HANDLE hr = NULL;

  LoadString(g_hinstDLL, IDS_MODULE_NAME, ModName, MAX_LOADSTRING);
  GetPrivateProfileString("DefaultPaths", "SuperIDEPath", "", SuperIDEPath, MAX_PATH, IniFile);
  GetPrivateProfileString(ModName, "Master", "", FileName, MAX_PATH, IniFile);
  MountDisk(FileName, MASTER);
  GetPrivateProfileString(ModName, "Slave", "", FileName, MAX_PATH, IniFile);
  BaseAddr = GetPrivateProfileInt(ModName, "BaseAddr", 1, IniFile);
  ClockEnabled = GetPrivateProfileInt(ModName, "ClkEnable", 1, IniFile);
  ClockReadOnly = GetPrivateProfileInt(ModName, "ClkRdOnly", 1, IniFile);
  BaseAddr &= 3;

  if (BaseAddr == 3)
    ClockEnabled = 0;

  BaseAddress = BaseTable[BaseAddr];
  SetClockWrite(!ClockReadOnly);
  MountDisk(FileName, SLAVE);
  BuildDynaMenu();
}
