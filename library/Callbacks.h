#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) LRESULT CALLBACK __cdecl CreateAudioConfigDialogCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) LRESULT CALLBACK __cdecl CreateBitBangerConfigDialogCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) LRESULT CALLBACK __cdecl CreateCpuConfigDialogCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) LRESULT CALLBACK __cdecl CreateDisplayConfigDialogCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) LRESULT CALLBACK __cdecl CreateInputConfigDialogCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) LRESULT CALLBACK __cdecl CreateJoyStickConfigDialogCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) LRESULT CALLBACK __cdecl CreateMiscConfigDialogCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) LRESULT CALLBACK __cdecl CreateTapeConfigDialogCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
