#ifndef __DEFINES_H__
#define __DEFINES_H__

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

extern void (*CPUInit)(void);
extern int  (*CPUExec)(int);
extern void (*CPUReset)(void);
extern void (*CPUAssertInterupt)(unsigned char, unsigned char);
extern void (*CPUDeAssertInterupt)(unsigned char);
extern void (*CPUForcePC)(unsigned short);

static char RateList[4][7] = { "Mute", "11025", "22050", "44100" };
static unsigned short iRateList[4] = { 0, 11025, 22050, 44100 };

#endif
