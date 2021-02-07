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
// This is an expansion module for the Vcc Emulator. It simulated the functions of the TRS-80 Multi-Pak Interface

#include <windows.h>
#include <iostream>
#include <commctrl.h>

#include "stdio.h"
#include "resource.h" 
#include "mpi.h"

#include "..\fileops\fileops.h"

#define MAXPAX 4
static void (*AssertInt)(unsigned char, unsigned char) = NULL;
static unsigned char (*MemRead8)(unsigned short) = NULL;
static void (*MemWrite8)(unsigned char, unsigned short) = NULL;

static void (*PakSetCart)(unsigned char) = NULL;
static HINSTANCE g_hinstDLL = NULL;
static char ModuleNames[MAXPAX][MAX_LOADSTRING] = { "Empty","Empty","Empty","Empty" };
static char CatNumber[MAXPAX][MAX_LOADSTRING] = { "","","","" };
static char SlotLabel[MAXPAX][MAX_LOADSTRING * 2] = { "Empty","Empty","Empty","Empty" };

static unsigned char PersistPaks = 0;
static char ModulePaths[MAXPAX][MAX_PATH] = { "","","","" };
static unsigned char* ExtRomPointers[MAXPAX] = { NULL,NULL,NULL,NULL };
static unsigned int BankedCartOffset[MAXPAX] = { 0,0,0,0 };
static unsigned char temp, temp2;
static char IniFile[MAX_PATH] = "";
static char MPIPath[MAX_PATH];

using namespace std;

//**************************************************************
//Array of fuction pointer for each Slot
static void (*GetModuleNameCalls[MAXPAX])(char*, char*, DYNAMICMENUCALLBACK) = { NULL,NULL,NULL,NULL };
static void (*ConfigModuleCalls[MAXPAX])(unsigned char) = { NULL,NULL,NULL,NULL };
static void (*HeartBeatCalls[MAXPAX])(void) = { NULL,NULL,NULL,NULL };
static void (*PakPortWriteCalls[MAXPAX])(unsigned char, unsigned char) = { NULL,NULL,NULL,NULL };
static unsigned char (*PakPortReadCalls[MAXPAX])(unsigned char) = { NULL,NULL,NULL,NULL };
static void (*PakMemWrite8Calls[MAXPAX])(unsigned char, unsigned short) = { NULL,NULL,NULL,NULL };
static unsigned char (*PakMemRead8Calls[MAXPAX])(unsigned short) = { NULL,NULL,NULL,NULL };
static void (*ModuleStatusCalls[MAXPAX])(char*) = { NULL,NULL,NULL,NULL };
static unsigned short (*ModuleAudioSampleCalls[MAXPAX])(void) = { NULL,NULL,NULL,NULL };
static void (*ModuleResetCalls[MAXPAX]) (void) = { NULL,NULL,NULL,NULL };

//Set callbacks for the DLL to call
static void (*SetInteruptCallPointerCalls[MAXPAX]) (ASSERTINTERUPT) = { NULL,NULL,NULL,NULL };
static void (*DmaMemPointerCalls[MAXPAX]) (MEMREAD8, MEMWRITE8) = { NULL,NULL,NULL,NULL };

static char MenuName0[64][512], MenuName1[64][512], MenuName2[64][512], MenuName3[64][512];
static int MenuId0[64], MenuId1[64], MenuId2[64], MenuId3[64];
static int Type0[64], Type1[64], Type2[64], Type3[64];

static int MenuIndex[4] = { 0,0,0,0 };

void SetCartSlot0(unsigned char);
void SetCartSlot1(unsigned char);
void SetCartSlot2(unsigned char);
void SetCartSlot3(unsigned char);
void BuildDynaMenu(void);
void DynamicMenuCallback0(char*, int, int);
void DynamicMenuCallback1(char*, int, int);
void DynamicMenuCallback2(char*, int, int);
void DynamicMenuCallback3(char*, int, int);
static unsigned char CartForSlot[MAXPAX] = { 0,0,0,0 };
static void (*SetCarts[MAXPAX])(unsigned char) = { SetCartSlot0,SetCartSlot1,SetCartSlot2,SetCartSlot3 };
static void (*DynamicMenuCallbackCalls[MAXPAX])(char*, int, int) = { DynamicMenuCallback0,DynamicMenuCallback1,DynamicMenuCallback2,DynamicMenuCallback3 };
static void (*SetCartCalls[MAXPAX])(SETCART) = { NULL,NULL,NULL,NULL };

static void (*SetIniPathCalls[MAXPAX]) (char*) = { NULL,NULL,NULL,NULL };
static void (*DynamicMenuCallback)(char*, int, int) = NULL;

static HINSTANCE hinstLib[4] = { NULL,NULL,NULL,NULL };
static unsigned char ChipSelectSlot = 3, SpareSelectSlot = 3, SwitchSlot = 3, SlotRegister = 255;

//Function Prototypes for this module
LRESULT CALLBACK Config(HWND, UINT, WPARAM, LPARAM);

unsigned char MountModule(unsigned char, char*);
void UnloadModule(unsigned char);
void LoadCartDLL(unsigned char, char*);
void LoadConfig(void);
void WriteConfig(void);
void ReadModuleParms(unsigned char, char*);
int FileID(char*);

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
    WriteConfig();

    for (temp = 0; temp < 4; temp++)
      UnloadModule(temp);

    break;
  }

  return TRUE;
}

void MemWrite(unsigned char Data, unsigned short Address)
{
  MemWrite8(Data, Address);
}

unsigned char MemRead(unsigned short Address)
{
  return(MemRead8(Address));
}

extern "C"
{
  __declspec(dllexport) void ModuleName(char* moduleName, char* catNumber, DYNAMICMENUCALLBACK menuCallback)
  {
    LoadString(g_hinstDLL, IDS_MODULE_NAME, moduleName, MAX_LOADSTRING);
    LoadString(g_hinstDLL, IDS_CATNUMBER, catNumber, MAX_LOADSTRING);
    DynamicMenuCallback = menuCallback;
  }
}

extern "C"
{
  __declspec(dllexport) void ModuleConfig(unsigned char menuID)
  {
    if ((menuID >= 10) & (menuID <= 19)) //Local config menus
      switch (menuID)
      {
      case 10:
        LoadCartDLL(3, ModulePaths[3]);
        WriteConfig();
        BuildDynaMenu();
        break;

      case 11:
        UnloadModule(3);
        BuildDynaMenu();
        break;

      case 12:
        LoadCartDLL(2, ModulePaths[2]);
        WriteConfig();
        BuildDynaMenu();
        break;

      case 13:
        UnloadModule(2);
        BuildDynaMenu();
        break;

      case 14:
        LoadCartDLL(1, ModulePaths[1]);
        WriteConfig();
        BuildDynaMenu();
        break;

      case 15:
        UnloadModule(1);
        BuildDynaMenu();
        break;

      case 16:
        LoadCartDLL(0, ModulePaths[0]);
        WriteConfig();
        BuildDynaMenu();
        break;

      case 17:
        UnloadModule(0);
        BuildDynaMenu();
        break;

      case 18:
        DialogBox(g_hinstDLL, (LPCTSTR)IDD_DIALOG1, NULL, (DLGPROC)Config);
        break;

      case 19:
        break;
      }

    //Add calls to sub-modules here		
    if ((menuID >= 20) & (menuID <= 40))
      ConfigModuleCalls[0](menuID - 20);

    if ((menuID > 40) & (menuID <= 60))
      ConfigModuleCalls[1](menuID - 40);

    if ((menuID > 61) & (menuID <= 80))
      ConfigModuleCalls[2](menuID - 60);

    if ((menuID > 80) & (menuID <= 100))
      ConfigModuleCalls[3](menuID - 80);
  }
}

// This captures the Function transfer point for the CPU assert interupt 
extern "C"
{
  __declspec(dllexport) void AssertInterupt(ASSERTINTERUPT dummy)
  {
    AssertInt = dummy;
    for (temp = 0; temp < 4; temp++)
    {
      if (SetInteruptCallPointerCalls[temp] != NULL)
        SetInteruptCallPointerCalls[temp](AssertInt);
    }
  }
}

extern "C"
{
  __declspec(dllexport) void PackPortWrite(unsigned char port, unsigned char data)
  {
    if (port == 0x7F) //Addressing the Multi-Pak
    {
      SpareSelectSlot = (data & 3);
      ChipSelectSlot = ((data & 0x30) >> 4);
      SlotRegister = data;
      PakSetCart(0);

      if (CartForSlot[SpareSelectSlot] == 1)
        PakSetCart(1);

      return;
    }

    for (unsigned char Temp = 0;Temp < 4;Temp++)
      if (PakPortWriteCalls[Temp] != NULL)
        PakPortWriteCalls[Temp](port, data);
  }
}

extern "C"
{
  __declspec(dllexport) unsigned char PackPortRead(unsigned char port)
  {
    if (port == 0x7F)
    {
      SlotRegister &= 0xCC;
      SlotRegister |= (SpareSelectSlot | (ChipSelectSlot << 4));

      return(SlotRegister);
    }

    temp2 = 0;

    for (temp = 0; temp < 4; temp++)
    {
      if (PakPortReadCalls[temp] != NULL)
      {
        temp2 = PakPortReadCalls[temp](port); //Find a Module that return a value 

        if (temp2 != 0)
          return(temp2);
      }
    }

    return(0);
  }
}

extern "C"
{
  __declspec(dllexport) void HeartBeat(void)
  {
    for (temp = 0; temp < 4; temp++)
      if (HeartBeatCalls[temp] != NULL)
        HeartBeatCalls[temp]();
  }
}

//This captures the pointers to the MemRead8 and MemWrite8 functions. This allows the DLL to do DMA xfers with CPU ram.
extern "C"
{
  __declspec(dllexport) void MemPointers(MEMREAD8 readPointer, MEMWRITE8 writePointer)
  {
    MemRead8 = readPointer;
    MemWrite8 = writePointer;
  }
}

extern "C"
{
  __declspec(dllexport) unsigned char PakMemRead8(unsigned short address)
  {
    if (ExtRomPointers[ChipSelectSlot] != NULL)
      return(ExtRomPointers[ChipSelectSlot][(address & 32767) + BankedCartOffset[ChipSelectSlot]]); //Bank Select ???

    if (PakMemRead8Calls[ChipSelectSlot] != NULL)
      return(PakMemRead8Calls[ChipSelectSlot](address));

    return(NULL);
  }
}

extern "C"
{
  __declspec(dllexport) void PakMemWrite8(unsigned char data, unsigned short address)
  {
  }
}

extern "C"
{
  __declspec(dllexport) void ModuleStatus(char* status)
  {
    char TempStatus[64] = "";
    sprintf(status, "MPI:%i,%i", ChipSelectSlot, SpareSelectSlot);

    for (temp = 0; temp < 4; temp++)
    {
      strcpy(TempStatus, "");

      if (ModuleStatusCalls[temp] != NULL)
      {
        ModuleStatusCalls[temp](TempStatus);
        strcat(status, "|");
        strcat(status, TempStatus);
      }
    }
  }
}

// This gets called at the end of every scan line 262 Lines * 60 Frames = 15780 Hz 15720
extern "C"
{
  __declspec(dllexport) unsigned short ModuleAudioSample(void)
  {
    unsigned short TempSample = 0;
    for (temp = 0; temp < 4; temp++)
      if (ModuleAudioSampleCalls[temp] != NULL)
        TempSample += ModuleAudioSampleCalls[temp]();

    return(TempSample);
  }
}

extern "C"
{
  __declspec(dllexport) unsigned char ModuleReset(void)
  {
    ChipSelectSlot = SwitchSlot;
    SpareSelectSlot = SwitchSlot;

    for (temp = 0; temp < 4; temp++)
    {
      BankedCartOffset[temp] = 0; //Do I need to keep independant selects?

      if (ModuleResetCalls[temp] != NULL)
        ModuleResetCalls[temp]();
    }

    PakSetCart(0);

    if (CartForSlot[SpareSelectSlot] == 1)
      PakSetCart(1);

    return(NULL);
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

void CPUAssertInterupt(unsigned char interrupt, unsigned char latencey)
{
  AssertInt(interrupt, latencey);
}

extern "C"
{
  __declspec(dllexport) void SetCart(SETCART pointer)
  {
    PakSetCart = pointer;
  }
}

LRESULT CALLBACK Config(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  unsigned short EDITBOXS[4] = { IDC_EDIT1,IDC_EDIT2,IDC_EDIT3,IDC_EDIT4 };
  unsigned short INSERTBTN[4] = { ID_INSERT1,ID_INSERT2,ID_INSERT3,ID_INSERT4 };
  unsigned short REMOVEBTN[4] = { ID_REMOVE1,ID_REMOVE2,ID_REMOVE3,ID_REMOVE4 };
  unsigned short CONFIGBTN[4] = { ID_CONFIG1,ID_CONFIG2,ID_CONFIG3,ID_CONFIG4 };
  char ConfigText[1024] = "";

  unsigned char temp = 0;

  switch (message)
  {
  case WM_INITDIALOG:
    for (temp = 0; temp < 4; temp++)
      SendDlgItemMessage(hDlg, EDITBOXS[temp], WM_SETTEXT, 5, (LPARAM)(LPCSTR)SlotLabel[temp]);
    SendDlgItemMessage(hDlg, IDC_PAKSELECT, TBM_SETRANGE, TRUE, MAKELONG(0, 3));
    SendDlgItemMessage(hDlg, IDC_PAKSELECT, TBM_SETPOS, TRUE, SwitchSlot);
    ReadModuleParms(SwitchSlot, ConfigText);
    SendDlgItemMessage(hDlg, IDC_MODINFO, WM_SETTEXT, strlen(ConfigText), (LPARAM)(LPCSTR)ConfigText);
    SendDlgItemMessage(hDlg, IDC_PAK, BM_SETCHECK, PersistPaks, 0);

    return TRUE;
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
      PersistPaks = (unsigned char)SendDlgItemMessage(hDlg, IDC_PAK, BM_GETCHECK, 0, 0);
      EndDialog(hDlg, LOWORD(wParam));
      WriteConfig();

      return TRUE;
    }

    for (temp = 0; temp < 4; temp++)
    {
      if (LOWORD(wParam) == INSERTBTN[temp])
      {
        LoadCartDLL(temp, ModulePaths[temp]);

        for (temp = 0; temp < 4; temp++)
          SendDlgItemMessage(hDlg, EDITBOXS[temp], WM_SETTEXT, strlen(SlotLabel[temp]), (LPARAM)(LPCSTR)SlotLabel[temp]);
      }
    }

    for (temp = 0; temp < 4; temp++)
    {
      if (LOWORD(wParam) == REMOVEBTN[temp])
      {
        UnloadModule(temp);
        SendDlgItemMessage(hDlg, EDITBOXS[temp], WM_SETTEXT, strlen(SlotLabel[temp]), (LPARAM)(LPCSTR)SlotLabel[temp]);
      }
    }

    for (temp = 0; temp < 4; temp++)
    {
      if (LOWORD(wParam) == CONFIGBTN[temp])
      {
        if (ConfigModuleCalls[temp] != NULL)
          ConfigModuleCalls[temp](NULL);
      }
    }

    return TRUE;
    break;

  case WM_HSCROLL:
    SwitchSlot = (unsigned char)SendDlgItemMessage(hDlg, IDC_PAKSELECT, TBM_GETPOS, (WPARAM)0, (WPARAM)0);
    SpareSelectSlot = SwitchSlot;
    ChipSelectSlot = SwitchSlot;
    ReadModuleParms(SwitchSlot, ConfigText);
    SendDlgItemMessage(hDlg, IDC_MODINFO, WM_SETTEXT, strlen(ConfigText), (LPARAM)(LPCSTR)ConfigText);
    PakSetCart(0);

    if (CartForSlot[SpareSelectSlot] == 1)
      PakSetCart(1);

    break;
  }

  return FALSE;
}


unsigned char MountModule(unsigned char slot, char* moduleName)
{
  unsigned char ModuleType = 0;
  char ModuleName[MAX_PATH] = "";
  unsigned int index = 0;
  strcpy(ModuleName, moduleName);
  FILE* rom_handle;

  if (slot > 3)
    return(0);

  ModuleType = FileID(ModuleName);

  switch (ModuleType)
  {
  case 0: //File doesn't exist
    return(0);
    break;

  case 2: //ROM image
    UnloadModule(slot);
    ExtRomPointers[slot] = (unsigned char*)malloc(0x40000);

    if (ExtRomPointers[slot] == NULL)
    {
      MessageBox(0, "Rom pointer is NULL", "Error", 0);
      return(0); //Can Allocate RAM
    }

    rom_handle = fopen(ModuleName, "rb");

    if (rom_handle == NULL)
    {
      MessageBox(0, "File handle is NULL", "Error", 0);
      return(0);
    }

    while ((feof(rom_handle) == 0) & (index < 0x40000))
      ExtRomPointers[slot][index++] = fgetc(rom_handle);

    fclose(rom_handle);
    strcpy(ModulePaths[slot], ModuleName);
    PathStripPath(ModuleName);

    PathRemoveExtension(ModuleName);
    strcpy(ModuleNames[slot], ModuleName);
    strcpy(SlotLabel[slot], ModuleName); //JF
    CartForSlot[slot] = 1;

    return(1);
    break;

  case 1:	//DLL File
    UnloadModule(slot);
    strcpy(ModulePaths[slot], ModuleName);
    hinstLib[slot] = LoadLibrary(ModuleName);

    if (hinstLib[slot] == NULL)
      return(0);	//Error Can't open File

    GetModuleNameCalls[slot] = (GETNAME)GetProcAddress(hinstLib[slot], "ModuleName");
    ConfigModuleCalls[slot] = (CONFIGIT)GetProcAddress(hinstLib[slot], "ModuleConfig");
    PakPortWriteCalls[slot] = (PACKPORTWRITE)GetProcAddress(hinstLib[slot], "PackPortWrite");
    PakPortReadCalls[slot] = (PACKPORTREAD)GetProcAddress(hinstLib[slot], "PackPortRead");
    SetInteruptCallPointerCalls[slot] = (SETINTERUPTCALLPOINTER)GetProcAddress(hinstLib[slot], "AssertInterupt");

    DmaMemPointerCalls[slot] = (DMAMEMPOINTERS)GetProcAddress(hinstLib[slot], "MemPointers");
    SetCartCalls[slot] = (SETCARTPOINTER)GetProcAddress(hinstLib[slot], "SetCart"); //HERE

    HeartBeatCalls[slot] = (HEARTBEAT)GetProcAddress(hinstLib[slot], "HeartBeat");
    PakMemWrite8Calls[slot] = (MEMWRITE8)GetProcAddress(hinstLib[slot], "PakMemWrite8");
    PakMemRead8Calls[slot] = (MEMREAD8)GetProcAddress(hinstLib[slot], "PakMemRead8");
    ModuleStatusCalls[slot] = (MODULESTATUS)GetProcAddress(hinstLib[slot], "ModuleStatus");
    ModuleAudioSampleCalls[slot] = (MODULEAUDIOSAMPLE)GetProcAddress(hinstLib[slot], "ModuleAudioSample");
    ModuleResetCalls[slot] = (MODULERESET)GetProcAddress(hinstLib[slot], "ModuleReset");
    SetIniPathCalls[slot] = (SETINIPATH)GetProcAddress(hinstLib[slot], "SetIniPath");

    if (GetModuleNameCalls[slot] == NULL)
    {
      UnloadModule(slot);
      MessageBox(0, "Not a valid Module", "Ok", 0);

      return(0); //Error Not a Vcc Module 
    }

    GetModuleNameCalls[slot](ModuleNames[slot], CatNumber[slot], DynamicMenuCallbackCalls[slot]); //Need to add address of local Dynamic menu callback function!
    strcpy(SlotLabel[slot], ModuleNames[slot]);
    strcat(SlotLabel[slot], "  ");
    strcat(SlotLabel[slot], CatNumber[slot]);

    if (SetInteruptCallPointerCalls[slot] != NULL)
      SetInteruptCallPointerCalls[slot](AssertInt);

    if (DmaMemPointerCalls[slot] != NULL)
      DmaMemPointerCalls[slot](MemRead8, MemWrite8);

    if (SetIniPathCalls[slot] != NULL)
      SetIniPathCalls[slot](IniFile);

    if (SetCartCalls[slot] != NULL)
      SetCartCalls[slot](*SetCarts[slot]);	//Transfer the address of the SetCart routin to the pak
                                            //For the multpak there is 1 for each slot se we know where it came from
    if (ModuleResetCalls[slot] != NULL)
      ModuleResetCalls[slot]();

    return(1);
    break;
  }

  return(0);
}

void UnloadModule(unsigned char slot)
{
  GetModuleNameCalls[slot] = NULL;
  ConfigModuleCalls[slot] = NULL;
  PakPortWriteCalls[slot] = NULL;
  PakPortReadCalls[slot] = NULL;
  SetInteruptCallPointerCalls[slot] = NULL;
  DmaMemPointerCalls[slot] = NULL;
  HeartBeatCalls[slot] = NULL;
  PakMemWrite8Calls[slot] = NULL;
  PakMemRead8Calls[slot] = NULL;
  ModuleStatusCalls[slot] = NULL;
  ModuleAudioSampleCalls[slot] = NULL;
  ModuleResetCalls[slot] = NULL;
  SetIniPathCalls[slot] = NULL;

  strcpy(ModulePaths[slot], "");
  strcpy(ModuleNames[slot], "Empty");
  strcpy(CatNumber[slot], "");
  strcpy(SlotLabel[slot], "Empty");

  if (hinstLib[slot] != NULL)
    FreeLibrary(hinstLib[slot]);

  if (ExtRomPointers[slot] != NULL)
    free(ExtRomPointers[slot]);

  hinstLib[slot] = NULL;
  ExtRomPointers[slot] = NULL;
  CartForSlot[slot] = 0;
  MenuIndex[slot] = 0;
}

void LoadCartDLL(unsigned char slot, char* dllPath)
{
  OPENFILENAME ofn;
  unsigned char RetVal = 0;

  UnloadModule(slot);
  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = NULL;
  ofn.lpstrFilter = "Program Packs\0*.ROM;*.ccc;*.DLL;*.pak\0\0";			// filter string
  ofn.nFilterIndex = 1;								        // current filter index
  ofn.lpstrFile = dllPath;						        // contains full path and filename on return
  ofn.nMaxFile = MAX_PATH;						        // sizeof lpstrFile
  ofn.lpstrFileTitle = NULL;							    // filename and extension only
  ofn.nMaxFileTitle = MAX_PATH;						    // sizeof lpstrFileTitle
  ofn.lpstrInitialDir = MPIPath;							// initial directory
  ofn.lpstrTitle = TEXT("Load Program Pack");	// title bar string
  ofn.Flags = OFN_HIDEREADONLY;

  if (GetOpenFileName(&ofn))
  {
    RetVal = MountModule(slot, dllPath);
    string tmp = ofn.lpstrFile;
    size_t idx;
    idx = tmp.find_last_of("\\");
    tmp = tmp.substr(0, idx);
    strcpy(MPIPath, tmp.c_str());
  }
}

void LoadConfig(void)
{
  char ModName[MAX_LOADSTRING] = "";

  LoadString(g_hinstDLL, IDS_MODULE_NAME, ModName, MAX_LOADSTRING);
  PersistPaks = GetPrivateProfileInt(ModName, "PersistPaks", 1, IniFile);
  GetPrivateProfileString("DefaultPaths", "MPIPath", "", MPIPath, MAX_PATH, IniFile);
  SwitchSlot = GetPrivateProfileInt(ModName, "SWPOSITION", 3, IniFile);
  ChipSelectSlot = SwitchSlot;
  SpareSelectSlot = SwitchSlot;
  GetPrivateProfileString(ModName, "SLOT1", "", ModulePaths[0], MAX_PATH, IniFile);
  CheckPath(ModulePaths[0]);
  GetPrivateProfileString(ModName, "SLOT2", "", ModulePaths[1], MAX_PATH, IniFile);
  CheckPath(ModulePaths[1]);
  GetPrivateProfileString(ModName, "SLOT3", "", ModulePaths[2], MAX_PATH, IniFile);
  CheckPath(ModulePaths[2]);
  GetPrivateProfileString(ModName, "SLOT4", "", ModulePaths[3], MAX_PATH, IniFile);
  CheckPath(ModulePaths[3]);

  for (temp = 0; temp < 4; temp++)
    if (strlen(ModulePaths[temp]) != 0)
      MountModule(temp, ModulePaths[temp]);

  BuildDynaMenu();
}

void WriteConfig(void)
{
  char ModName[MAX_LOADSTRING] = "";

  if (MPIPath != "") { WritePrivateProfileString("DefaultPaths", "MPIPath", MPIPath, IniFile); }

  LoadString(g_hinstDLL, IDS_MODULE_NAME, ModName, MAX_LOADSTRING);
  WritePrivateProfileInt(ModName, "SWPOSITION", SwitchSlot, IniFile);
  WritePrivateProfileInt(ModName, "PesistPaks", PersistPaks, IniFile);
  ValidatePath(ModulePaths[0]);
  WritePrivateProfileString(ModName, "SLOT1", ModulePaths[0], IniFile);
  ValidatePath(ModulePaths[1]);
  WritePrivateProfileString(ModName, "SLOT2", ModulePaths[1], IniFile);
  ValidatePath(ModulePaths[2]);
  WritePrivateProfileString(ModName, "SLOT3", ModulePaths[2], IniFile);
  ValidatePath(ModulePaths[3]);
  WritePrivateProfileString(ModName, "SLOT4", ModulePaths[3], IniFile);
}

void ReadModuleParms(unsigned char slot, char* moduleText)
{
  strcat(moduleText, "Module Name: ");
  strcat(moduleText, ModuleNames[slot]);

  strcat(moduleText, "\r\n-----------------------------------------\r\n");

  if (ConfigModuleCalls[slot] != NULL)
    strcat(moduleText, "Has Configurable options\r\n");

  if (SetIniPathCalls[slot] != NULL)
    strcat(moduleText, "Saves Config Info\r\n");

  if (PakPortWriteCalls[slot] != NULL)
    strcat(moduleText, "Is IO writable\r\n");

  if (PakPortReadCalls[slot] != NULL)
    strcat(moduleText, "Is IO readable\r\n");

  if (SetInteruptCallPointerCalls[slot] != NULL)
    strcat(moduleText, "Generates Interupts\r\n");

  if (DmaMemPointerCalls[slot] != NULL)
    strcat(moduleText, "Generates DMA Requests\r\n");

  if (HeartBeatCalls[slot] != NULL)
    strcat(moduleText, "Needs Heartbeat\r\n");

  if (ModuleAudioSampleCalls[slot] != NULL)
    strcat(moduleText, "Analog Audio Outputs\r\n");

  if (PakMemWrite8Calls[slot] != NULL)
    strcat(moduleText, "Needs CS Write\r\n");

  if (PakMemRead8Calls[slot] != NULL)
    strcat(moduleText, "Needs CS Read (onboard ROM)\r\n");

  if (ModuleStatusCalls[slot] != NULL)
    strcat(moduleText, "Returns Status\r\n");

  if (ModuleResetCalls[slot] != NULL)
    strcat(moduleText, "Needs Reset Notification\r\n");
}

int FileID(char* filename)
{
  FILE* DummyHandle = NULL;
  char Temp[3] = "";
  DummyHandle = fopen(filename, "rb");

  if (DummyHandle == NULL)
    return(0);	//File Doesn't exist

  Temp[0] = fgetc(DummyHandle);
  Temp[1] = fgetc(DummyHandle);
  Temp[2] = 0;
  fclose(DummyHandle);

  if (strcmp(Temp, "MZ") == 0)
    return(1);	//DLL File

  return(2);		//Rom Image 
}

void SetCartSlot0(unsigned char cart)
{
  CartForSlot[0] = cart;
}

void SetCartSlot1(unsigned char cart)
{
  CartForSlot[1] = cart;
}

void SetCartSlot2(unsigned char cart)
{
  CartForSlot[2] = cart;
}

void SetCartSlot3(unsigned char cart)
{
  CartForSlot[3] = cart;
}

void BuildDynaMenu(void)	//STUB
{
  unsigned char tempIndex = 0;
  char TempMsg[512] = "";

  if (DynamicMenuCallback == NULL)
    MessageBox(0, "No good", "Ok", 0);

  DynamicMenuCallback("", 0, 0);
  DynamicMenuCallback("", 6000, 0);
  DynamicMenuCallback("MPI Slot 4", 6000, HEAD);
  DynamicMenuCallback("Insert", 5010, SLAVE);
  sprintf(TempMsg, "Eject: ");
  strcat(TempMsg, SlotLabel[3]);
  DynamicMenuCallback(TempMsg, 5011, SLAVE);
  DynamicMenuCallback("MPI Slot 3", 6000, HEAD);
  DynamicMenuCallback("Insert", 5012, SLAVE);
  sprintf(TempMsg, "Eject: ");
  strcat(TempMsg, SlotLabel[2]);
  DynamicMenuCallback(TempMsg, 5013, SLAVE);
  DynamicMenuCallback("MPI Slot 2", 6000, HEAD);
  DynamicMenuCallback("Insert", 5014, SLAVE);
  sprintf(TempMsg, "Eject: ");
  strcat(TempMsg, SlotLabel[1]);
  DynamicMenuCallback(TempMsg, 5015, SLAVE);
  DynamicMenuCallback("MPI Slot 1", 6000, HEAD);
  DynamicMenuCallback("Insert", 5016, SLAVE);
  sprintf(TempMsg, "Eject: ");
  strcat(TempMsg, SlotLabel[0]);
  DynamicMenuCallback(TempMsg, 5017, SLAVE);
  DynamicMenuCallback("MPI Config", 5018, STANDALONE);

  for (tempIndex = 0; tempIndex < MenuIndex[3]; tempIndex++)
    DynamicMenuCallback(MenuName3[tempIndex], MenuId3[tempIndex] + 80, Type3[tempIndex]);

  for (tempIndex = 0; tempIndex < MenuIndex[2]; tempIndex++)
    DynamicMenuCallback(MenuName2[tempIndex], MenuId2[tempIndex] + 60, Type2[tempIndex]);

  for (tempIndex = 0; tempIndex < MenuIndex[1]; tempIndex++)
    DynamicMenuCallback(MenuName1[tempIndex], MenuId1[tempIndex] + 40, Type1[tempIndex]);

  for (tempIndex = 0; tempIndex < MenuIndex[0]; tempIndex++)
    DynamicMenuCallback(MenuName0[tempIndex], MenuId0[tempIndex] + 20, Type0[tempIndex]);

  DynamicMenuCallback("", 1, 0);
}

void DynamicMenuCallback0(char* menuName, int menuId, int type)
{
  if (menuId == 0)
  {
    MenuIndex[0] = 0;
    return;
  }

  if (menuId == 1)
  {
    BuildDynaMenu();
    return;
  }

  strcpy(MenuName0[MenuIndex[0]], menuName);
  MenuId0[MenuIndex[0]] = menuId;
  Type0[MenuIndex[0]] = type;
  MenuIndex[0]++;
}

void DynamicMenuCallback1(char* menuName, int menuId, int type)
{
  if (menuId == 0)
  {
    MenuIndex[1] = 0;
  }

  if (menuId == 1)
  {
    BuildDynaMenu();
  }

  strcpy(MenuName1[MenuIndex[1]], menuName);
  MenuId1[MenuIndex[1]] = menuId;
  Type1[MenuIndex[1]] = type;
  MenuIndex[1]++;

  return;
}

void DynamicMenuCallback2(char* menuName, int menuId, int type)
{
  if (menuId == 0)
  {
    MenuIndex[2] = 0;
    return;
  }

  if (menuId == 1)
  {
    BuildDynaMenu();
    return;
  }

  strcpy(MenuName2[MenuIndex[2]], menuName);
  MenuId2[MenuIndex[2]] = menuId;
  Type2[MenuIndex[2]] = type;
  MenuIndex[2]++;
}

void DynamicMenuCallback3(char* menuName, int menuId, int type)
{
  if (menuId == 0)
  {
    MenuIndex[3] = 0;
    return;
  }

  if (menuId == 1)
  {
    BuildDynaMenu();
    return;
  }

  strcpy(MenuName3[MenuIndex[3]], menuName);
  MenuId3[MenuIndex[3]] = menuId;
  Type3[MenuIndex[3]] = type;
  MenuIndex[3]++;
}
