#include <string>

#include "pakinterfacestate.h"

#include "ConfigAccessors.h"
#include "PakInterfaceAccessors.h"
#include "load_ext_rom.h"

#include "library/cpudef.h"
#include "library/defines.h"
#include "library/fileoperations.h"
#include "library/systemstate.h"

extern void UnloadDll(SystemState* systemState);
extern void DynamicMenuCallback(SystemState* systemState, char* menuName, int menuId, int type);
extern void DynamicMenuCallback(char* menuName, int menuId, int type);
extern void SetCart(unsigned char);
extern unsigned char MemRead8(unsigned short);
extern void MemWrite8(unsigned char, unsigned short);

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
