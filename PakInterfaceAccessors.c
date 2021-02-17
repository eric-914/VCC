#include <iostream>

#include "pakinterfacestate.h"

#include "library/systemstate.h"

void GetCurrentModule(char* defaultModule)
{
  strcpy(defaultModule, GetPakInterfaceState()->DllPath);
}

int FileID(char* filename)
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

unsigned char PakPortRead(unsigned char port)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->PakPortRead != NULL) {
    return(pakInterfaceState->PakPortRead(port));
  }
  else {
    return(NULL);
  }
}

void PakPortWrite(unsigned char port, unsigned char data)
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

unsigned char PakMem8Read(unsigned short address)
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

void PakMem8Write(unsigned char port, unsigned char data)
{
  //TODO: This really is empty
}

unsigned short PakAudioSample(void)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->ModuleAudioSample != NULL) {
    return(pakInterfaceState->ModuleAudioSample());
  }

  return(NULL);
}
