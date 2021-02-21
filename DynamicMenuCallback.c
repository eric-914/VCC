#include "library/PAKInterface.h"
#include "library/VCC.h"

/*
* TODO: This exists because this is what the different plugins expect, but it requires the EmuState
*/
void DynamicMenuCallback(char* menuName, int menuId, int type)
{
  DynamicMenuCallback(&(GetVccState()->EmuState), menuName, menuId, type);
}
