#pragma once

void CreateMenu(HWND hWnd);
void EmuExit();
void EmuReset(unsigned char state);
void EmuRun();
void HelpAbout(HWND hWnd);
void KeyUp(WPARAM wParam, LPARAM lParam);
void MouseMove(LPARAM lParam);
void ShowConfiguration();
