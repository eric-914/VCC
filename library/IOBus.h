#pragma once

extern "C" __declspec(dllexport) unsigned char __cdecl port_read(unsigned short addr);
extern "C" __declspec(dllexport) void __cdecl port_write(unsigned char data, unsigned short addr);
