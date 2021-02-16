#pragma once

unsigned short GetMem(long);
unsigned char* GetInternalRomPointer(void);
void SetMmuEnabled(unsigned char);
void SetMmuPrefix(unsigned char);
void SetMmuRegister(unsigned char, unsigned char);
void SetMmuTask(unsigned char task);
void UpdateMmuArray(void);
void SetMapType(unsigned char type);
void SetRomMap(unsigned char data);
void SetVectors(unsigned char data);
