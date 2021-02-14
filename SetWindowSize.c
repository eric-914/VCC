#include <windows.h>

void SetWindowSize(POINT p) {
  int width = p.x + 16;
  int height = p.y + 81;

  HWND handle = GetActiveWindow();

  SetWindowPos(handle, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}
