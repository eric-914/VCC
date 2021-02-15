#pragma once

extern "C" void SetJoystick(unsigned short x, unsigned short y);
extern "C" void SetStickNumbers(unsigned char leftStickNumber, unsigned char rightStickNumber);
extern "C" unsigned short get_pot_value(unsigned char pot);
extern "C" void SetButtonStatus(unsigned char side, unsigned char state);

char SetMouseStatus(char scanCode, unsigned char phase);
