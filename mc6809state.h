#pragma once

typedef struct {
} MC6809State;

extern "C" __declspec(dllexport) MC6809State* __cdecl GetMC6809State();
