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

using namespace std;

#include <windows.h>
#include <iostream>
#include <string>

#include "pakinterfacedef.h"
#include "pakinterfacestate.h"
#include "pakinterface.h"
#include "tcc1014mmu.h"
#include "Vcc.h"
#include "mc6821.h"

#include "ConfigAccessors.h"

#include "library/cpudef.h"
#include "library/defines.h"
#include "library/fileoperations.h"

int load_ext_rom(SystemState* systemState, char filename[MAX_PATH]);
int FileID(char*);

void PakTimer(void)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->HeartBeat != NULL) {
    pakInterfaceState->HeartBeat();
  }
}

void ResetBus(void)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  pakInterfaceState->BankedCartOffset = 0;

  if (pakInterfaceState->ModuleReset != NULL) {
    pakInterfaceState->ModuleReset();
  }
}

void GetModuleStatus(SystemState* systemState)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->ModuleStatus != NULL) {
    pakInterfaceState->ModuleStatus(systemState->StatusLine);
  }
  else {
    sprintf(systemState->StatusLine, "");
  }
}

unsigned char PackPortRead(unsigned char port)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->PakPortRead != NULL) {
    return(pakInterfaceState->PakPortRead(port));
  }
  else {
    return(NULL);
  }
}

void PackPortWrite(unsigned char port, unsigned char data)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->PakPortWrite != NULL)
  {
    pakInterfaceState->PakPortWrite(port, data);
    return;
  }

  if ((port == 0x40) && (pakInterfaceState->RomPackLoaded == true)) {
    pakInterfaceState->BankedCartOffset = (data & 15) << 14;
  }
}

unsigned char PackMem8Read(unsigned short address)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->PakMemRead8 != NULL) {
    return(pakInterfaceState->PakMemRead8(address & 32767));
  }

  if (pakInterfaceState->ExternalRomBuffer != NULL) {
    return(pakInterfaceState->ExternalRomBuffer[(address & 32767) + pakInterfaceState->BankedCartOffset]);
  }

  return(0);
}

void PackMem8Write(unsigned char port, unsigned char data)
{
}

unsigned short PackAudioSample(void)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->ModuleAudioSample != NULL) {
    return(pakInterfaceState->ModuleAudioSample());
  }

  return(NULL);
}

int LoadCart(SystemState* systemState)
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH] = "";
  char temp[MAX_PATH];

  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  GetIniFilePath(temp);

  GetPrivateProfileString("DefaultPaths", "PakPath", "", pakInterfaceState->PakPath, MAX_PATH, temp);

  memset(&ofn, 0, sizeof(ofn));

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = systemState->WindowHandle;
  ofn.lpstrFilter = "Program Packs\0*.ROM;*.ccc;*.DLL;*.pak\0\0";			// filter string
  ofn.nFilterIndex = 1;							          // current filter index
  ofn.lpstrFile = szFileName;				          // contains full path and filename on return
  ofn.nMaxFile = MAX_PATH;					          // sizeof lpstrFile
  ofn.lpstrFileTitle = NULL;						      // filename and extension only
  ofn.nMaxFileTitle = MAX_PATH;					      // sizeof lpstrFileTitle
  ofn.lpstrInitialDir = pakInterfaceState->PakPath;				      // initial directory
  ofn.lpstrTitle = TEXT("Load Program Pack");	// title bar string
  ofn.Flags = OFN_HIDEREADONLY;

  if (GetOpenFileName(&ofn)) {
    if (!InsertModule(systemState, szFileName)) {
      string tmp = ofn.lpstrFile;
      size_t idx = tmp.find_last_of("\\");
      tmp = tmp.substr(0, idx);

      strcpy(pakInterfaceState->PakPath, tmp.c_str());

      WritePrivateProfileString("DefaultPaths", "PakPath", pakInterfaceState->PakPath, temp);

      return(0);
    }
  }

  return(1);
}

/**
Load a ROM pack
return total bytes loaded, or 0 on failure
*/
int load_ext_rom(SystemState* systemState, char filename[MAX_PATH])
{
  constexpr size_t PAK_MAX_MEM = 0x40000;

  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  // If there is an existing ROM, ditch it
  if (pakInterfaceState->ExternalRomBuffer != nullptr) {
    free(pakInterfaceState->ExternalRomBuffer);
  }

  // Allocate memory for the ROM
  pakInterfaceState->ExternalRomBuffer = (uint8_t*)malloc(PAK_MAX_MEM);

  // If memory was unable to be allocated, fail
  if (pakInterfaceState->ExternalRomBuffer == nullptr) {
    MessageBox(0, "cant allocate ram", "Ok", 0);

    return 0;
  }

  // Open the ROM file, fail if unable to
  FILE* rom_handle = fopen(filename, "rb");

  if (rom_handle == nullptr) return 0;

  // Load the file, one byte at a time.. (TODO: Get size and read entire block)
  int index = 0;

  while ((feof(rom_handle) == 0) && (index < PAK_MAX_MEM)) {
    pakInterfaceState->ExternalRomBuffer[index++] = fgetc(rom_handle);
  }

  fclose(rom_handle);

  UnloadDll(systemState);

  pakInterfaceState->BankedCartOffset = 0;
  pakInterfaceState->RomPackLoaded = true;

  return index;
}

void UnloadDll(SystemState* systemState)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if ((pakInterfaceState->DialogOpen == true) && (systemState->EmulationRunning == 1))
  {
    MessageBox(0, "Close Configuration Dialog before unloading", "Ok", 0);

    return;
  }

  pakInterfaceState->GetModuleName = NULL;
  pakInterfaceState->ConfigModule = NULL;
  pakInterfaceState->PakPortWrite = NULL;
  pakInterfaceState->PakPortRead = NULL;
  pakInterfaceState->SetInterruptCallPointer = NULL;
  pakInterfaceState->DmaMemPointer = NULL;
  pakInterfaceState->HeartBeat = NULL;
  pakInterfaceState->PakMemWrite8 = NULL;
  pakInterfaceState->PakMemRead8 = NULL;
  pakInterfaceState->ModuleStatus = NULL;
  pakInterfaceState->ModuleAudioSample = NULL;
  pakInterfaceState->ModuleReset = NULL;

  if (pakInterfaceState->hInstLib != NULL) {
    FreeLibrary(pakInterfaceState->hInstLib);
  }

  pakInterfaceState->hInstLib = NULL;
  DynamicMenuCallback(systemState, "", 0, 0); //Refresh Menus
  DynamicMenuCallback(systemState, "", 1, 0);
}

void GetCurrentModule(char* defaultModule)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  strcpy(defaultModule, pakInterfaceState->DllPath);
}

void UpdateBusPointer(void)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->SetInterruptCallPointer != NULL) {
    pakInterfaceState->SetInterruptCallPointer(GetCPU()->CPUAssertInterrupt);
  }
}

void UnloadPack(SystemState* systemState)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  UnloadDll(systemState);

  strcpy(pakInterfaceState->DllPath, "");
  strcpy(pakInterfaceState->Modname, "Blank");

  pakInterfaceState->RomPackLoaded = false;

  SetCart(0);

  if (pakInterfaceState->ExternalRomBuffer != nullptr) {
    free(pakInterfaceState->ExternalRomBuffer);
  }

  pakInterfaceState->ExternalRomBuffer = nullptr;

  systemState->ResetPending = 2;

  DynamicMenuCallback(systemState, "", 0, 0); //Refresh Menus
  DynamicMenuCallback(systemState, "", 1, 0);
}

int FileID(char* filename)
{
  FILE* DummyHandle = NULL;
  char Temp[3] = "";

  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  DummyHandle = fopen(filename, "rb");

  if (DummyHandle == NULL) {
    return(0);	//File Doesn't exist
  }

  Temp[0] = fgetc(DummyHandle);
  Temp[1] = fgetc(DummyHandle);
  Temp[2] = 0;
  fclose(DummyHandle);

  if (strcmp(Temp, "MZ") == 0) {
    return(1);	//DLL File
  }

  return(2);		//Rom Image 
}

void DynamicMenuActivated(SystemState* systemState, unsigned char menuItem)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  switch (menuItem)
  {
  case 1:
    LoadPack();
    break;

  case 2:
    UnloadPack(systemState);
    break;

  default:
    if (pakInterfaceState->ConfigModule != NULL) {
      pakInterfaceState->ConfigModule(menuItem);
    }

    break;
  }
}

void DynamicMenuCallback(SystemState* systemState, char* menuName, int menuId, int type)
{
  char temp[256] = "";

  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  //MenuId=0 Flush Buffer MenuId=1 Done 
  switch (menuId)
  {
  case 0:
    pakInterfaceState->MenuIndex = 0;

    DynamicMenuCallback(systemState, "Cartridge", 6000, HEAD);	//Recursion is fun
    DynamicMenuCallback(systemState, "Load Cart", 5001, SLAVE);

    sprintf(temp, "Eject Cart: ");
    strcat(temp, pakInterfaceState->Modname);

    DynamicMenuCallback(systemState, temp, 5002, SLAVE);

    break;

  case 1:
    RefreshDynamicMenu(systemState);
    break;

  default:
    strcpy(pakInterfaceState->MenuItem[pakInterfaceState->MenuIndex].MenuName, menuName);

    pakInterfaceState->MenuItem[pakInterfaceState->MenuIndex].MenuId = menuId;
    pakInterfaceState->MenuItem[pakInterfaceState->MenuIndex].Type = type;

    pakInterfaceState->MenuIndex++;

    break;
  }
}

void DynamicMenuCallback(char* menuName, int menuId, int type)
{
  extern SystemState EmuState;

  DynamicMenuCallback(&EmuState, menuName, menuId, type);
}

void RefreshDynamicMenu(SystemState* systemState)
{
  MENUITEMINFO Mii;
  char MenuTitle[32] = "Cartridge";
  unsigned char tempIndex = 0, Index = 0;
  static HWND hOld = 0;
  int SubMenuIndex = 0;

  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if ((pakInterfaceState->hMenu == NULL) || (systemState->WindowHandle != hOld)) {
    pakInterfaceState->hMenu = GetMenu(systemState->WindowHandle);
  }
  else {
    DeleteMenu(pakInterfaceState->hMenu, 3, MF_BYPOSITION);
  }

  hOld = systemState->WindowHandle;
  pakInterfaceState->hSubMenu[SubMenuIndex] = CreatePopupMenu();

  memset(&Mii, 0, sizeof(MENUITEMINFO));

  Mii.cbSize = sizeof(MENUITEMINFO);
  Mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
  Mii.fType = MFT_STRING;
  Mii.wID = 4999;
  Mii.hSubMenu = pakInterfaceState->hSubMenu[SubMenuIndex];
  Mii.dwTypeData = MenuTitle;
  Mii.cch = (UINT)strlen(MenuTitle);

  InsertMenuItem(pakInterfaceState->hMenu, 3, TRUE, &Mii);

  SubMenuIndex++;

  for (tempIndex = 0; tempIndex < pakInterfaceState->MenuIndex; tempIndex++)
  {
    if (strlen(pakInterfaceState->MenuItem[tempIndex].MenuName) == 0) {
      pakInterfaceState->MenuItem[tempIndex].Type = STANDALONE;
    }

    //Create Menu item in title bar if no exist already
    switch (pakInterfaceState->MenuItem[tempIndex].Type)
    {
    case HEAD:
      SubMenuIndex++;

      pakInterfaceState->hSubMenu[SubMenuIndex] = CreatePopupMenu();

      memset(&Mii, 0, sizeof(MENUITEMINFO));

      Mii.cbSize = sizeof(MENUITEMINFO);
      Mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
      Mii.fType = MFT_STRING;
      Mii.wID = pakInterfaceState->MenuItem[tempIndex].MenuId;
      Mii.hSubMenu = pakInterfaceState->hSubMenu[SubMenuIndex];
      Mii.dwTypeData = pakInterfaceState->MenuItem[tempIndex].MenuName;
      Mii.cch = (UINT)strlen(pakInterfaceState->MenuItem[tempIndex].MenuName);

      InsertMenuItem(pakInterfaceState->hSubMenu[0], 0, FALSE, &Mii);

      break;

    case SLAVE:
      memset(&Mii, 0, sizeof(MENUITEMINFO));

      Mii.cbSize = sizeof(MENUITEMINFO);
      Mii.fMask = MIIM_TYPE | MIIM_ID;
      Mii.fType = MFT_STRING;
      Mii.wID = pakInterfaceState->MenuItem[tempIndex].MenuId;
      Mii.hSubMenu = pakInterfaceState->hSubMenu[SubMenuIndex];
      Mii.dwTypeData = pakInterfaceState->MenuItem[tempIndex].MenuName;
      Mii.cch = (UINT)strlen(pakInterfaceState->MenuItem[tempIndex].MenuName);

      InsertMenuItem(pakInterfaceState->hSubMenu[SubMenuIndex], 0, FALSE, &Mii);

      break;

    case STANDALONE:
      if (strlen(pakInterfaceState->MenuItem[tempIndex].MenuName) == 0)
      {
        memset(&Mii, 0, sizeof(MENUITEMINFO));

        Mii.cbSize = sizeof(MENUITEMINFO);
        Mii.fMask = MIIM_TYPE | MIIM_ID;
        Mii.fType = MF_SEPARATOR;
        Mii.wID = pakInterfaceState->MenuItem[tempIndex].MenuId;
        Mii.hSubMenu = pakInterfaceState->hMenu;
        Mii.dwTypeData = pakInterfaceState->MenuItem[tempIndex].MenuName;
        Mii.cch = (UINT)strlen(pakInterfaceState->MenuItem[tempIndex].MenuName);

        InsertMenuItem(pakInterfaceState->hSubMenu[0], 0, FALSE, &Mii);
      }
      else
      {
        memset(&Mii, 0, sizeof(MENUITEMINFO));

        Mii.cbSize = sizeof(MENUITEMINFO);
        Mii.fMask = MIIM_TYPE | MIIM_ID;
        Mii.fType = MFT_STRING;
        Mii.wID = pakInterfaceState->MenuItem[tempIndex].MenuId;
        Mii.hSubMenu = pakInterfaceState->hMenu;
        Mii.dwTypeData = pakInterfaceState->MenuItem[tempIndex].MenuName;
        Mii.cch = (UINT)strlen(pakInterfaceState->MenuItem[tempIndex].MenuName);

        InsertMenuItem(pakInterfaceState->hSubMenu[0], 0, FALSE, &Mii);
      }

      break;
    }
  }

  DrawMenuBar(systemState->WindowHandle);
}

int InsertModule(SystemState* systemState, char* modulePath)
{
  char catNumber[MAX_LOADSTRING] = "";
  char temp[MAX_LOADSTRING] = "";
  char text[1024] = "";
  char ini[MAX_PATH] = "";
  unsigned char fileType = 0;

  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  fileType = FileID(modulePath);

  switch (fileType)
  {
  case 0:		//File doesn't exist
    return(NOMODULE);
    break;

  case 2:		//File is a ROM image
    UnloadDll(systemState);
    
    load_ext_rom(systemState, modulePath);

    strncpy(pakInterfaceState->Modname, modulePath, MAX_PATH);

    FilePathStripPath(pakInterfaceState->Modname);

    DynamicMenuCallback(systemState, "", 0, 0); //Refresh Menus
    DynamicMenuCallback(systemState, "", 1, 0);

    systemState->ResetPending = 2;

    SetCart(1);

    return(NOMODULE);

  case 1:		//File is a DLL
    
    UnloadDll(systemState);
    pakInterfaceState->hInstLib = LoadLibrary(modulePath);

    if (pakInterfaceState->hInstLib == NULL) {
      return(NOMODULE);
    }

    SetCart(0);

    pakInterfaceState->GetModuleName = (GETNAME)GetProcAddress(pakInterfaceState->hInstLib, "ModuleName");
    pakInterfaceState->ConfigModule = (CONFIGIT)GetProcAddress(pakInterfaceState->hInstLib, "ModuleConfig");
    pakInterfaceState->PakPortWrite = (PACKPORTWRITE)GetProcAddress(pakInterfaceState->hInstLib, "PackPortWrite");
    pakInterfaceState->PakPortRead = (PACKPORTREAD)GetProcAddress(pakInterfaceState->hInstLib, "PackPortRead");
    pakInterfaceState->SetInterruptCallPointer = (SETINTERRUPTCALLPOINTER)GetProcAddress(pakInterfaceState->hInstLib, "AssertInterrupt");
    pakInterfaceState->DmaMemPointer = (DMAMEMPOINTERS)GetProcAddress(pakInterfaceState->hInstLib, "MemPointers");
    pakInterfaceState->HeartBeat = (HEARTBEAT)GetProcAddress(pakInterfaceState->hInstLib, "HeartBeat");
    pakInterfaceState->PakMemWrite8 = (MEMWRITE8)GetProcAddress(pakInterfaceState->hInstLib, "PakMemWrite8");
    pakInterfaceState->PakMemRead8 = (MEMREAD8)GetProcAddress(pakInterfaceState->hInstLib, "PakMemRead8");
    pakInterfaceState->ModuleStatus = (MODULESTATUS)GetProcAddress(pakInterfaceState->hInstLib, "ModuleStatus");
    pakInterfaceState->ModuleAudioSample = (MODULEAUDIOSAMPLE)GetProcAddress(pakInterfaceState->hInstLib, "ModuleAudioSample");
    pakInterfaceState->ModuleReset = (MODULERESET)GetProcAddress(pakInterfaceState->hInstLib, "ModuleReset");
    pakInterfaceState->SetIniPath = (SETINIPATH)GetProcAddress(pakInterfaceState->hInstLib, "SetIniPath");
    pakInterfaceState->PakSetCart = (SETCARTPOINTER)GetProcAddress(pakInterfaceState->hInstLib, "SetCart");

    if (pakInterfaceState->GetModuleName == NULL)
    {
      FreeLibrary(pakInterfaceState->hInstLib);

      pakInterfaceState->hInstLib = NULL;

      return(NOTVCC);
    }

    pakInterfaceState->BankedCartOffset = 0;

    if (pakInterfaceState->DmaMemPointer != NULL) {
      pakInterfaceState->DmaMemPointer(MemRead8, MemWrite8);
    }

    if (pakInterfaceState->SetInterruptCallPointer != NULL) {
      pakInterfaceState->SetInterruptCallPointer(GetCPU()->CPUAssertInterrupt);
    }

    pakInterfaceState->GetModuleName(pakInterfaceState->Modname, catNumber, DynamicMenuCallback);  //Instantiate the menus from HERE!

    sprintf(temp, "Configure %s", pakInterfaceState->Modname);

    strcat(text, "Module Name: ");
    strcat(text, pakInterfaceState->Modname);
    strcat(text, "\n");

    if (pakInterfaceState->ConfigModule != NULL)
    {
      pakInterfaceState->ModualParms |= 1;

      strcat(text, "Has Configurable options\n");
    }

    if (pakInterfaceState->PakPortWrite != NULL)
    {
      pakInterfaceState->ModualParms |= 2;

      strcat(text, "Is IO writable\n");
    }

    if (pakInterfaceState->PakPortRead != NULL)
    {
      pakInterfaceState->ModualParms |= 4;

      strcat(text, "Is IO readable\n");
    }

    if (pakInterfaceState->SetInterruptCallPointer != NULL)
    {
      pakInterfaceState->ModualParms |= 8;

      strcat(text, "Generates Interrupts\n");
    }

    if (pakInterfaceState->DmaMemPointer != NULL)
    {
      pakInterfaceState->ModualParms |= 16;

      strcat(text, "Generates DMA Requests\n");
    }

    if (pakInterfaceState->HeartBeat != NULL)
    {
      pakInterfaceState->ModualParms |= 32;

      strcat(text, "Needs Heartbeat\n");
    }

    if (pakInterfaceState->ModuleAudioSample != NULL)
    {
      pakInterfaceState->ModualParms |= 64;
      
      strcat(text, "Analog Audio Outputs\n");
    }

    if (pakInterfaceState->PakMemWrite8 != NULL)
    {
      pakInterfaceState->ModualParms |= 128;

      strcat(text, "Needs ChipSelect Write\n");
    }

    if (pakInterfaceState->PakMemRead8 != NULL)
    {
      pakInterfaceState->ModualParms |= 256;

      strcat(text, "Needs ChipSelect Read\n");
    }

    if (pakInterfaceState->ModuleStatus != NULL)
    {
      pakInterfaceState->ModualParms |= 512;
      
      strcat(text, "Returns Status\n");
    }

    if (pakInterfaceState->ModuleReset != NULL)
    {
      pakInterfaceState->ModualParms |= 1024;
      
      strcat(text, "Needs Reset Notification\n");
    }

    if (pakInterfaceState->SetIniPath != NULL)
    {
      pakInterfaceState->ModualParms |= 2048;
      
      GetIniFilePath(ini);
      
      pakInterfaceState->SetIniPath(ini);
    }

    if (pakInterfaceState->PakSetCart != NULL)
    {
      pakInterfaceState->ModualParms |= 4096;
      
      strcat(text, "Can Assert CART\n");
      
      pakInterfaceState->PakSetCart(SetCart);
    }

    strcpy(pakInterfaceState->DllPath, modulePath);

    systemState->ResetPending = 2;

    return(0);
  }

  return(NOMODULE);
}
