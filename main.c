#include <windows.h>

#include "library/App.h"

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ INT nCmdShow) {
  return AppExec(hInstance, lpCmdLine, nCmdShow);
}
