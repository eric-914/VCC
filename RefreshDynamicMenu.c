#include <windows.h>

#include "pakinterfacestate.h"

#include "library/systemstate.h"

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
