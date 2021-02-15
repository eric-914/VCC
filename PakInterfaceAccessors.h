#pragma once

#include "library/systemstate.h"

void GetCurrentModule(char* defaultModule);
int FileID(char* filename);
void PakTimer(void);
void ResetBus(void);
void GetModuleStatus(SystemState* systemState);
unsigned char PackPortRead(unsigned char port);
void PackPortWrite(unsigned char port, unsigned char data);
unsigned char PackMem8Read(unsigned short address);
void PackMem8Write(unsigned char port, unsigned char data);
unsigned short PackAudioSample(void);
