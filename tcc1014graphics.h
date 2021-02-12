#ifndef __TCC1014GRAPHICS_H__
#define __TCC1014GRAPHICS_H__
/*
Copyright 2015 by Joseph Forgione
This file is part of VCC (Virtual Color Computer).

    VCC (Virtual Color Computer) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VCC (Virtual Color Computer) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VCC (Virtual Color Computer).  If not, see <http://www.gnu.org/licenses/>.
*/

#include "library/systemstate.h"

void SetGimeVdgOffset(unsigned char);
void SetGimeVdgMode(unsigned char);
void SetGimeVdgMode2(unsigned char);

void SetVerticalOffsetRegister(unsigned short);
void SetCompatMode(unsigned char);
void SetGimeVmode(unsigned char);
void SetGimeVres(unsigned char);
void SetGimeHorzOffset(unsigned char);
void SetGimeBoarderColor(unsigned char);
void InvalidateBoarder();

void GimeInit(void);
void GimeReset(void);
void SetVideoBank(unsigned char);
unsigned char SetMonitorType(unsigned char);
void SetBoarderChange(unsigned char);

//unsigned char SetScanLines(unsigned char);
unsigned char SetScanLines(SystemState* systemState, unsigned char lines);

#define MRGB	1
#define MCMP	0

#endif
