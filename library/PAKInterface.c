#include <iostream>

#include "PAKInterface.h"
#include "VCC.h"
#include "systemstate.h"

PakInterfaceState* InitializeInstance(PakInterfaceState*);

static PakInterfaceState* instance = InitializeInstance(new PakInterfaceState());

extern "C" {
  __declspec(dllexport) PakInterfaceState* __cdecl GetPakInterfaceState() {
    return instance;
  }
}

PakInterfaceState* InitializeInstance(PakInterfaceState* p) {
  p->BankedCartOffset = 0;
  p->ModualParms = 0;
  p->DialogOpen = false;
  p->RomPackLoaded = false;
  p->MenuIndex = 0;

  strcpy(p->DllPath, "");
  strcpy(p->Modname, "Blank");

  p->ExternalRomBuffer = nullptr;
  p->hMenu = NULL;

  p->GetModuleName = NULL;
  p->ConfigModule = NULL;
  p->SetInterruptCallPointer = NULL;
  p->DmaMemPointer = NULL;
  p->HeartBeat = NULL;
  p->PakPortWrite = NULL;
  p->PakPortRead = NULL;
  p->PakMemWrite8 = NULL;
  p->PakMemRead8 = NULL;
  p->ModuleStatus = NULL;
  p->ModuleAudioSample = NULL;
  p->ModuleReset = NULL;
  p->SetIniPath = NULL;
  p->PakSetCart = NULL;

  return p;
}

extern "C" {
  __declspec(dllexport) void __cdecl GetCurrentModule(char* defaultModule)
  {
    strcpy(defaultModule, GetPakInterfaceState()->DllPath);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl FileID(char* filename)
  {
    FILE* handle = NULL;
    char temp[3] = "";

    handle = fopen(filename, "rb");

    if (handle == NULL) {
      return(0);	//File Doesn't exist
    }

    temp[0] = fgetc(handle);
    temp[1] = fgetc(handle);
    temp[2] = 0;
    fclose(handle);

    if (strcmp(temp, "MZ") == 0) {
      return(1);	//DLL File
    }

    return(2);		//Rom Image 
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl PakTimer(void)
  {
    PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

    if (pakInterfaceState->HeartBeat != NULL) {
      pakInterfaceState->HeartBeat();
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ResetBus(void)
  {
    PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

    pakInterfaceState->BankedCartOffset = 0;

    if (pakInterfaceState->ModuleReset != NULL) {
      pakInterfaceState->ModuleReset();
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GetModuleStatus(SystemState* systemState)
  {
    PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

    if (pakInterfaceState->ModuleStatus != NULL) {
      pakInterfaceState->ModuleStatus(systemState->StatusLine);
    }
    else {
      sprintf(systemState->StatusLine, "");
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl PakPortRead(unsigned char port)
  {
    PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

    if (pakInterfaceState->PakPortRead != NULL) {
      return(pakInterfaceState->PakPortRead(port));
    }
    else {
      return(NULL);
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl PakPortWrite(unsigned char port, unsigned char data)
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
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl PakMem8Read(unsigned short address)
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
}

extern "C" {
  __declspec(dllexport) void __cdecl PakMem8Write(unsigned char port, unsigned char data)
  {
    //TODO: This really is empty
  }
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl PakAudioSample(void)
  {
    PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

    if (pakInterfaceState->ModuleAudioSample != NULL) {
      return(pakInterfaceState->ModuleAudioSample());
    }

    return(NULL);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl RefreshDynamicMenu(SystemState* systemState)
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
}

extern "C" {
  __declspec(dllexport) void __cdecl DynamicMenuCallback(SystemState* systemState, char* menuName, int menuId, int type)
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
}
