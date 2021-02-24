#pragma once

void CreateMenu(HWND hWnd);
void EmuExit();
void EmuReset(unsigned char state);
void EmuRun();
void HelpAbout(HWND hWnd);
void KeyDown(WPARAM wParam, LPARAM lParam);
void KeyUp(WPARAM wParam, LPARAM lParam);
void MouseMove(LPARAM lParam);
void ShowConfiguration();

void ToggleMonitorType();
void ToggleThrottle();
void ToggleFullScreen();
void ToggleOnOff();
void ToggleInfoBand();

void SlowDown();
void SpeedUp();
