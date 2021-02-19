#include <iostream>

#include "pakinterfacestate.h"
#include "systemstate.h"

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
