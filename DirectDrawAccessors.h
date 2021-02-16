#pragma once

#include "windows.h"

#include "library/systemstate.h"

POINT GetCurrentWindowSize();
void CheckSurfaces();
void SetStatusBarText(char*, SystemState*);
void Cls(unsigned int, SystemState*);
unsigned char SetInfoBand(unsigned char);
unsigned char SetResize(unsigned char);
unsigned char SetAspect(unsigned char);
