#include <string>

#include "library/pakinterfacestate.h"
#include "library/VCC.h"

#include "RefreshDynamicMenu.h"

#include "library/systemstate.h"

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

/*
* TODO: This exists because this is what the different plugins expect, but it requires the EmuState
*/
void DynamicMenuCallback(char* menuName, int menuId, int type)
{
  DynamicMenuCallback(&(GetVccState()->EmuState), menuName, menuId, type);
}
