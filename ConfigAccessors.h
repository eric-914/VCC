#pragma once

#include <windows.h>

char* AppDirectory();
char* BasicRomName(void);

void GetIniFilePath(char* path);
void SetIniFilePath(char* path);

int GetKeyboardLayout();
int GetPaletteType();
int GetRememberSize();

POINT GetIniWindowSize();
