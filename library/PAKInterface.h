#pragma once

#include <windows.h>
#include <stdint.h>

#include "systemstate.h"

#define ID_SDYNAMENU 5000	//Defines the start and end IDs for the dynamic menus
#define ID_EDYNAMENU 5100
#define NOMODULE	1
#define NOTVCC	2

#define	HEAD 0
#define SLAVE 1
#define STANDALONE 2

#define HASCONFIG		1
#define HASIOWRITE		2
#define HASIOREAD		4
#define NEEDSCPUIRQ		8
#define DOESDMA			16
#define NEEDHEARTBEAT	32
#define ANALOGAUDIO		64
#define CSWRITE			128
#define CSREAD			256
#define RETURNSSTATUS	512
#define CARTRESET		1024
#define SAVESINI		2048
#define ASSERTCART		4096

typedef void (*DYNAMICMENUCALLBACK)(char*, int, int);
typedef void (*GETNAME)(char*, char*, DYNAMICMENUCALLBACK);
typedef void (*CONFIGIT)(unsigned char);
typedef void (*HEARTBEAT) (void);
typedef unsigned char (*PACKPORTREAD)(unsigned char);
typedef void (*PACKPORTWRITE)(unsigned char, unsigned char);
typedef void (*ASSERTINTERRUPT) (unsigned char, unsigned char);
typedef unsigned char (*MEMREAD8)(unsigned short);
typedef void (*SETCART)(unsigned char);
typedef void (*MEMWRITE8)(unsigned char, unsigned short);
typedef void (*MODULESTATUS)(char*);
typedef void (*DMAMEMPOINTERS) (MEMREAD8, MEMWRITE8);
typedef void (*SETCARTPOINTER)(SETCART);
typedef void (*SETINTERRUPTCALLPOINTER) (ASSERTINTERRUPT);
typedef unsigned short (*MODULEAUDIOSAMPLE)(void);
typedef void (*MODULERESET)(void);
typedef void (*SETINIPATH)(char*);

typedef struct {
  char MenuName[512];
  int MenuId;
  int Type;
} Dmenu;

typedef struct {
  HINSTANCE hInstLib;

  // Storage for Pak ROMs
  uint8_t* ExternalRomBuffer;
  bool RomPackLoaded;

  unsigned int BankedCartOffset;
  char DllPath[256];
  unsigned short ModualParms;
  bool DialogOpen;

  Dmenu MenuItem[100];

  HMENU hMenu;
  HMENU hSubMenu[64];

  char PakPath[MAX_PATH];
  unsigned char MenuIndex;
  char Modname[MAX_PATH];

  void (*GetModuleName)(char*, char*, DYNAMICMENUCALLBACK);
  void (*ConfigModule)(unsigned char);
  void (*SetInterruptCallPointer) (ASSERTINTERRUPT);
  void (*DmaMemPointer) (MEMREAD8, MEMWRITE8);
  void (*HeartBeat)(void);
  void (*PakPortWrite)(unsigned char, unsigned char);
  unsigned char (*PakPortRead)(unsigned char);
  void (*PakMemWrite8)(unsigned char, unsigned short);
  unsigned char (*PakMemRead8)(unsigned short);
  void (*ModuleStatus)(char*);
  unsigned short (*ModuleAudioSample)(void);
  void (*ModuleReset) (void);
  void (*SetIniPath) (char*);
  void (*PakSetCart)(SETCART);
} PakInterfaceState;

extern "C" __declspec(dllexport) PakInterfaceState * __cdecl GetPakInterfaceState();

extern "C" __declspec(dllexport) void __cdecl GetCurrentModule(char* defaultModule);
extern "C" __declspec(dllexport) int __cdecl FileID(char* filename);
extern "C" __declspec(dllexport) void __cdecl PakTimer(void);
extern "C" __declspec(dllexport) void __cdecl ResetBus(void);
extern "C" __declspec(dllexport) void __cdecl GetModuleStatus(SystemState* systemState);
extern "C" __declspec(dllexport) unsigned char __cdecl PakPortRead(unsigned char port);
extern "C" __declspec(dllexport) void __cdecl PakPortWrite(unsigned char port, unsigned char data);
extern "C" __declspec(dllexport) unsigned char __cdecl PakMem8Read(unsigned short address);
extern "C" __declspec(dllexport) void __cdecl PakMem8Write(unsigned char port, unsigned char data);
extern "C" __declspec(dllexport) unsigned short __cdecl PakAudioSample(void);

extern "C" __declspec(dllexport) void __cdecl RefreshDynamicMenu(SystemState* systemState);
extern "C" __declspec(dllexport) void __cdecl DynamicMenuCallback(SystemState* systemState, char* menuName, int menuId, int type);
extern "C" __declspec(dllexport) void __cdecl UnloadDll(SystemState* systemState);
extern "C" __declspec(dllexport) int __cdecl LoadROMPack(SystemState* systemState, char filename[MAX_PATH]);
extern "C" __declspec(dllexport) void __cdecl UnloadPack(SystemState* systemState);
extern "C" __declspec(dllexport) void __cdecl UpdateBusPointer(void);

extern "C" __declspec(dllexport) int __cdecl InsertModule(SystemState* systemState, char* modulePath);
extern "C" __declspec(dllexport) int __cdecl LoadCart(SystemState* systemState);
