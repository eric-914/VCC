#pragma once

unsigned char GimeRead(unsigned char port);
void GimeAssertKeyboardInterrupt(void);
void GimeAssertVertInterrupt(void);
void GimeAssertTimerInterrupt(void);
void GimeAssertHorzInterrupt(void);
