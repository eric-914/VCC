#pragma once

#include <windows.h>

BOOL CALLBACK DirectSoundEnumerateCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
