#pragma once

#include "library/systemstate.h"

void DynamicMenuCallback(char* menuName, int menuId, int type);
void DynamicMenuCallback(SystemState* systemState, char* menuName, int menuId, int type);
