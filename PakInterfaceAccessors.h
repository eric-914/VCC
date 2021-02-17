#pragma once

#include "library/systemstate.h"

void GetCurrentModule(char* defaultModule);
int FileID(char* filename);
void PakTimer(void);
void ResetBus(void);
void GetModuleStatus(SystemState* systemState);
unsigned char PakPortRead(unsigned char port);
void PakPortWrite(unsigned char port, unsigned char data);
unsigned char PakMem8Read(unsigned short address);
void PakMem8Write(unsigned char port, unsigned char data);
unsigned short PakAudioSample(void);
