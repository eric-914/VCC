#include <windows.h>
#include <process.h>

#include "VCC.h"

extern "C" {
  __declspec(dllexport) INT __cdecl AppRun(HINSTANCE hInstance, PSTR lpCmdLine, INT nCmdShow) {
    VccStartup(hInstance, lpCmdLine, nCmdShow);

    VccRun();

    return VccShutdown();
  }
}