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

    Additional 6309 modifications by Walter ZAMBOTTI 2019
*/

#include <stdio.h>
#include <stdlib.h>

#include "hd6309.h"
#include "hd6309defs.h"
#include "hd6309state.h"
#include "hd6309vector.h"

#include "MmuAccessors.h"
#include "MemRead8.h"
#include "MemWrite8.h"
#include "MemRead16.h"
#include "MemWrite16.h"
#include "MemRead32.h"
#include "MemWrite32.h"

CpuRegister pc, x, y, u, s, dp, v, z;
WideRegister q;

unsigned char cc[8];
unsigned int md[8];
unsigned char ccbits;
unsigned char mdbits;

unsigned char* ureg8[8];
unsigned short* xfreg16[8];

char InInterrupt = 0;
int CycleCounter = 0;
unsigned int SyncWaiting = 0;
unsigned char Source = 0;
unsigned char Dest = 0;

unsigned char postbyte = 0;
short unsigned postword = 0;
signed char* spostbyte = (signed char*)&postbyte;
signed short* spostword = (signed short*)&postword;

unsigned char temp8;
unsigned short temp16;
unsigned int temp32;

signed short stemp16;
int stemp32;

int gCycleFor;

unsigned char NatEmuCycles65 = 6;
unsigned char NatEmuCycles64 = 6;
unsigned char NatEmuCycles32 = 3;
unsigned char NatEmuCycles21 = 2;
unsigned char NatEmuCycles54 = 5;
unsigned char NatEmuCycles97 = 9;
unsigned char NatEmuCycles85 = 8;
unsigned char NatEmuCycles51 = 5;
unsigned char NatEmuCycles31 = 3;
unsigned char NatEmuCycles1110 = 11;
unsigned char NatEmuCycles76 = 7;
unsigned char NatEmuCycles75 = 7;
unsigned char NatEmuCycles43 = 4;
unsigned char NatEmuCycles87 = 8;
unsigned char NatEmuCycles86 = 8;
unsigned char NatEmuCycles98 = 9;
unsigned char NatEmuCycles2726 = 27;
unsigned char NatEmuCycles3635 = 36;
unsigned char NatEmuCycles3029 = 30;
unsigned char NatEmuCycles2827 = 28;
unsigned char NatEmuCycles3726 = 37;
unsigned char NatEmuCycles3130 = 31;
unsigned char NatEmuCycles42 = 4;
unsigned char NatEmuCycles53 = 5;

static unsigned char IRQWaiter = 0;
static unsigned char PendingInterrupts = 0;
static unsigned char InsCycles[2][25];

static unsigned char* NatEmuCycles[] =
{
  &NatEmuCycles65,
  &NatEmuCycles64,
  &NatEmuCycles32,
  &NatEmuCycles21,
  &NatEmuCycles54,
  &NatEmuCycles97,
  &NatEmuCycles85,
  &NatEmuCycles51,
  &NatEmuCycles31,
  &NatEmuCycles1110,
  &NatEmuCycles76,
  &NatEmuCycles75,
  &NatEmuCycles43,
  &NatEmuCycles87,
  &NatEmuCycles86,
  &NatEmuCycles98,
  &NatEmuCycles2726,
  &NatEmuCycles3635,
  &NatEmuCycles3029,
  &NatEmuCycles2827,
  &NatEmuCycles3726,
  &NatEmuCycles3130,
  &NatEmuCycles42,
  &NatEmuCycles53
};

HD6309State* hd63096State = GetHD6309State();

unsigned short hd6309_CalculateEA(unsigned char);
void InvalidInsHandler(void);
void DivbyZero(void);
void ErrorVector(void);

void setcc(unsigned char);
unsigned char getcc(void);
void setmd(unsigned char);
unsigned char getmd(void);

static void cpu_firq(void);
static void cpu_irq(void);
static void cpu_nmi(void);

unsigned char GetSorceReg(unsigned char);

void Page_2(void);
void Page_3(void);

void HD6309Reset(void)
{
  for (char index = 0; index <= 6; index++) {		//Set all register to 0 except V
    PXF(index) = 0;
  }

  for (char index = 0; index <= 7; index++) {
    PUR(index) = 0;
  }

  CC_E = 0;
  CC_F = 1;
  CC_H = 0;
  CC_I = 1;
  CC_N = 0;
  CC_Z = 0;
  CC_V = 0;
  CC_C = 0;

  MD_NATIVE6309 = 0;
  MD_FIRQMODE = 0;
  MD_UNDEFINED2 = 0;  //UNDEFINED
  MD_UNDEFINED3 = 0;  //UNDEFINED
  MD_UNDEFINED4 = 0;  //UNDEFINED
  MD_UNDEFINED5 = 0;  //UNDEFINED
  MD_ILLEGAL = 0;
  MD_ZERODIV = 0;

  mdbits = getmd();

  SyncWaiting = 0;

  DP_REG = 0;
  PC_REG = MemRead16(VRESET);	//PC gets its reset vector

  SetMapType(0);	//shouldn't be here
}

void HD6309Init(void)
{	//Call this first or RESET will core!
  // reg pointers for TFR and EXG and LEA ops
  xfreg16[0] = &D_REG;
  xfreg16[1] = &X_REG;
  xfreg16[2] = &Y_REG;
  xfreg16[3] = &U_REG;
  xfreg16[4] = &S_REG;
  xfreg16[5] = &PC_REG;
  xfreg16[6] = &W_REG;
  xfreg16[7] = &V_REG;

  ureg8[0] = (unsigned char*)&A_REG;
  ureg8[1] = (unsigned char*)&B_REG;
  ureg8[2] = (unsigned char*)&ccbits;
  ureg8[3] = (unsigned char*)&DPA;
  ureg8[4] = (unsigned char*)&Z_H;
  ureg8[5] = (unsigned char*)&Z_L;
  ureg8[6] = (unsigned char*)&E_REG;
  ureg8[7] = (unsigned char*)&F_REG;

  //This handles the disparity between 6309 and 6809 Instruction timing
  InsCycles[0][M65] = 6;	//6-5
  InsCycles[1][M65] = 5;
  InsCycles[0][M64] = 6;	//6-4
  InsCycles[1][M64] = 4;
  InsCycles[0][M32] = 3;	//3-2
  InsCycles[1][M32] = 2;
  InsCycles[0][M21] = 2;	//2-1
  InsCycles[1][M21] = 1;
  InsCycles[0][M54] = 5;	//5-4
  InsCycles[1][M54] = 4;
  InsCycles[0][M97] = 9;	//9-7
  InsCycles[1][M97] = 7;
  InsCycles[0][M85] = 8;	//8-5
  InsCycles[1][M85] = 5;
  InsCycles[0][M51] = 5;	//5-1
  InsCycles[1][M51] = 1;
  InsCycles[0][M31] = 3;	//3-1
  InsCycles[1][M31] = 1;
  InsCycles[0][M1110] = 11;	//11-10
  InsCycles[1][M1110] = 10;
  InsCycles[0][M76] = 7;	//7-6
  InsCycles[1][M76] = 6;
  InsCycles[0][M75] = 7;	//7-5
  InsCycles[1][M75] = 5;
  InsCycles[0][M43] = 4;	//4-3
  InsCycles[1][M43] = 3;
  InsCycles[0][M87] = 8;	//8-7
  InsCycles[1][M87] = 7;
  InsCycles[0][M86] = 8;	//8-6
  InsCycles[1][M86] = 6;
  InsCycles[0][M98] = 9;	//9-8
  InsCycles[1][M98] = 8;
  InsCycles[0][M2726] = 27;	//27-26
  InsCycles[1][M2726] = 26;
  InsCycles[0][M3635] = 36;	//36-25
  InsCycles[1][M3635] = 35;
  InsCycles[0][M3029] = 30;	//30-29
  InsCycles[1][M3029] = 29;
  InsCycles[0][M2827] = 28;	//28-27
  InsCycles[1][M2827] = 27;
  InsCycles[0][M3726] = 37;	//37-26
  InsCycles[1][M3726] = 26;
  InsCycles[0][M3130] = 31;	//31-30
  InsCycles[1][M3130] = 30;
  InsCycles[0][M42] = 4;	//4-2
  InsCycles[1][M42] = 2;
  InsCycles[0][M53] = 5;	//5-3
  InsCycles[1][M53] = 3;
}

int HD6309Exec(int cycleFor)
{
  CycleCounter = 0;
  gCycleFor = cycleFor;

  while (CycleCounter < cycleFor) {

    if (PendingInterrupts)
    {
      if (PendingInterrupts & 4) {
        cpu_nmi();
      }

      if (PendingInterrupts & 2) {
        cpu_firq();
      }

      if (PendingInterrupts & 1)
      {
        if (IRQWaiter == 0) { // This is needed to fix a subtle timming problem
          cpu_irq();		// It allows the CPU to see $FF03 bit 7 high before
        }
        else {				// The IRQ is asserted.
          IRQWaiter -= 1;
        }
      }
    }

    if (SyncWaiting == 1) { //Abort the run nothing happens asyncronously from the CPU
      return(0); // WDZ - Experimental SyncWaiting should still return used cycles (and not zero) by breaking from loop
    }

    JmpVec1[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
  }

  return(cycleFor - CycleCounter);
}

void Page_2(void) //10
{
  JmpVec2[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
}

void Page_3(void) //11
{
  JmpVec3[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
}

void cpu_firq(void)
{
  if (!CC_F)
  {
    InInterrupt = 1; //Flag to indicate FIRQ has been asserted

    switch (MD_FIRQMODE)
    {
    case 0:
      CC_E = 0; // Turn E flag off

      MemWrite8(PC_L, --S_REG);
      MemWrite8(PC_H, --S_REG);
      MemWrite8(getcc(), --S_REG);

      CC_I = 1;
      CC_F = 1;
      PC_REG = MemRead16(VFIRQ);

      break;

    case 1:		//6309
      CC_E = 1;

      MemWrite8(PC_L, --S_REG);
      MemWrite8(PC_H, --S_REG);
      MemWrite8(U_L, --S_REG);
      MemWrite8(U_H, --S_REG);
      MemWrite8(Y_L, --S_REG);
      MemWrite8(Y_H, --S_REG);
      MemWrite8(X_L, --S_REG);
      MemWrite8(X_H, --S_REG);
      MemWrite8(DPA, --S_REG);

      if (MD_NATIVE6309)
      {
        MemWrite8((F_REG), --S_REG);
        MemWrite8((E_REG), --S_REG);
      }

      MemWrite8(B_REG, --S_REG);
      MemWrite8(A_REG, --S_REG);
      MemWrite8(getcc(), --S_REG);

      CC_I = 1;
      CC_F = 1;
      PC_REG = MemRead16(VFIRQ);

      break;
    }
  }

  PendingInterrupts &= 253;
}

void cpu_irq(void)
{
  if (InInterrupt == 1) { //If FIRQ is running postpone the IRQ
    return;
  }

  if (!CC_I) {
    CC_E = 1;

    MemWrite8(PC_L, --S_REG);
    MemWrite8(PC_H, --S_REG);
    MemWrite8(U_L, --S_REG);
    MemWrite8(U_H, --S_REG);
    MemWrite8(Y_L, --S_REG);
    MemWrite8(Y_H, --S_REG);
    MemWrite8(X_L, --S_REG);
    MemWrite8(X_H, --S_REG);
    MemWrite8(DPA, --S_REG);

    if (MD_NATIVE6309)
    {
      MemWrite8((F_REG), --S_REG);
      MemWrite8((E_REG), --S_REG);
    }

    MemWrite8(B_REG, --S_REG);
    MemWrite8(A_REG, --S_REG);
    MemWrite8(getcc(), --S_REG);

    PC_REG = MemRead16(VIRQ);
    CC_I = 1;
  }

  PendingInterrupts &= 254;
}

void cpu_nmi(void)
{
  CC_E = 1;

  MemWrite8(PC_L, --S_REG);
  MemWrite8(PC_H, --S_REG);
  MemWrite8(U_L, --S_REG);
  MemWrite8(U_H, --S_REG);
  MemWrite8(Y_L, --S_REG);
  MemWrite8(Y_H, --S_REG);
  MemWrite8(X_L, --S_REG);
  MemWrite8(X_H, --S_REG);
  MemWrite8(DPA, --S_REG);

  if (MD_NATIVE6309)
  {
    MemWrite8((F_REG), --S_REG);
    MemWrite8((E_REG), --S_REG);
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(getcc(), --S_REG);

  CC_I = 1;
  CC_F = 1;
  PC_REG = MemRead16(VNMI);

  PendingInterrupts &= 251;
}

unsigned short hd6309_CalculateEA(unsigned char postbyte)
{
  static unsigned short int ea = 0;
  static signed char byte = 0;
  static unsigned char reg;

  reg = ((postbyte >> 5) & 3) + 1;

  if (postbyte & 0x80)
  {
    switch (postbyte & 0x1F)
    {
    case 0: // Post inc by 1
      ea = PXF(reg);

      PXF(reg)++;

      CycleCounter += NatEmuCycles21;

      break;

    case 1: // post in by 2
      ea = PXF(reg);

      PXF(reg) += 2;

      CycleCounter += NatEmuCycles32;

      break;

    case 2: // pre dec by 1
      PXF(reg) -= 1;

      ea = PXF(reg);

      CycleCounter += NatEmuCycles21;

      break;

    case 3: // pre dec by 2
      PXF(reg) -= 2;

      ea = PXF(reg);

      CycleCounter += NatEmuCycles32;

      break;

    case 4: // no offset
      ea = PXF(reg);

      break;

    case 5: // B reg offset
      ea = PXF(reg) + ((signed char)B_REG);

      CycleCounter += 1;

      break;

    case 6: // A reg offset
      ea = PXF(reg) + ((signed char)A_REG);

      CycleCounter += 1;

      break;

    case 7: // E reg offset 
      ea = PXF(reg) + ((signed char)E_REG);

      CycleCounter += 1;

      break;

    case 8: // 8 bit offset
      ea = PXF(reg) + (signed char)MemRead8(PC_REG++);

      CycleCounter += 1;

      break;

    case 9: // 16 bit offset
      ea = PXF(reg) + IMMADDRESS(PC_REG);

      CycleCounter += NatEmuCycles43;

      PC_REG += 2;

      break;

    case 10: // F reg offset
      ea = PXF(reg) + ((signed char)F_REG);

      CycleCounter += 1;

      break;

    case 11: // D reg offset 
      ea = PXF(reg) + D_REG; //Changed to unsigned 03/14/2005 NG Was signed

      CycleCounter += NatEmuCycles42;

      break;

    case 12: // 8 bit PC relative
      ea = (signed short)PC_REG + (signed char)MemRead8(PC_REG) + 1;

      CycleCounter += 1;

      PC_REG++;

      break;

    case 13: // 16 bit PC relative
      ea = PC_REG + IMMADDRESS(PC_REG) + 2;

      CycleCounter += NatEmuCycles53;

      PC_REG += 2;

      break;

    case 14: // W reg offset
      ea = PXF(reg) + W_REG;

      CycleCounter += 4;

      break;

    case 15: // W reg
      byte = (postbyte >> 5) & 3;

      switch (byte)
      {
      case 0: // No offset from W reg
        ea = W_REG;

        break;

      case 1: // 16 bit offset from W reg
        ea = W_REG + IMMADDRESS(PC_REG);

        PC_REG += 2;

        CycleCounter += 2;

        break;

      case 2: // Post inc by 2 from W reg
        ea = W_REG;

        W_REG += 2;

        CycleCounter += 1;

        break;

      case 3: // Pre dec by 2 from W reg
        W_REG -= 2;

        ea = W_REG;

        CycleCounter += 1;

        break;
      }

      break;

    case 16: // W reg
      byte = (postbyte >> 5) & 3;

      switch (byte)
      {
      case 0: // Indirect no offset from W reg
        ea = MemRead16(W_REG);

        CycleCounter += 3;

        break;

      case 1: // Indirect 16 bit offset from W reg
        ea = MemRead16(W_REG + IMMADDRESS(PC_REG));

        PC_REG += 2;

        CycleCounter += 5;

        break;

      case 2: // Indirect post inc by 2 from W reg
        ea = MemRead16(W_REG);

        W_REG += 2;

        CycleCounter += 4;

        break;

      case 3: // Indirect pre dec by 2 from W reg
        W_REG -= 2;

        ea = MemRead16(W_REG);

        CycleCounter += 4;

        break;
      }
      break;

    case 17: // Indirect post inc by 2 
      ea = PXF(reg);

      PXF(reg) += 2;

      ea = MemRead16(ea);

      CycleCounter += 6;

      break;

    case 18: // possibly illegal instruction
      CycleCounter += 6;

      break;

    case 19: // Indirect pre dec by 2
      PXF(reg) -= 2;

      ea = MemRead16(PXF(reg));

      CycleCounter += 6;

      break;

    case 20: // Indirect no offset 
      ea = MemRead16(PXF(reg));

      CycleCounter += 3;

      break;

    case 21: // Indirect B reg offset
      ea = MemRead16(PXF(reg) + ((signed char)B_REG));

      CycleCounter += 4;

      break;

    case 22: // indirect A reg offset
      ea = MemRead16(PXF(reg) + ((signed char)A_REG));

      CycleCounter += 4;

      break;

    case 23: // indirect E reg offset
      ea = MemRead16(PXF(reg) + ((signed char)E_REG));

      CycleCounter += 4;

      break;

    case 24: // indirect 8 bit offset
      ea = MemRead16(PXF(reg) + (signed char)MemRead8(PC_REG++));

      CycleCounter += 4;

      break;

    case 25: // indirect 16 bit offset
      ea = MemRead16(PXF(reg) + IMMADDRESS(PC_REG));

      CycleCounter += 7;

      PC_REG += 2;

      break;

    case 26: // indirect F reg offset
      ea = MemRead16(PXF(reg) + ((signed char)F_REG));

      CycleCounter += 4;

      break;

    case 27: // indirect D reg offset
      ea = MemRead16(PXF(reg) + D_REG);

      CycleCounter += 7;

      break;

    case 28: // indirect 8 bit PC relative
      ea = MemRead16((signed short)PC_REG + (signed char)MemRead8(PC_REG) + 1);

      CycleCounter += 4;

      PC_REG++;

      break;

    case 29: //indirect 16 bit PC relative
      ea = MemRead16(PC_REG + IMMADDRESS(PC_REG) + 2);

      CycleCounter += 8;

      PC_REG += 2;

      break;

    case 30: // indirect W reg offset
      ea = MemRead16(PXF(reg) + W_REG);

      CycleCounter += 7;

      break;

    case 31: // extended indirect
      ea = MemRead16(IMMADDRESS(PC_REG));

      CycleCounter += 8;

      PC_REG += 2;

      break;
    }
  }
  else // 5 bit offset
  {
    byte = (postbyte & 31);
    byte = (byte << 3);
    byte = byte / 8;

    ea = PXF(reg) + byte; //Was signed

    CycleCounter += 1;
  }

  return(ea);
}

void setcc(unsigned char bincc)
{
  ccbits = bincc;

  CC_E = !!(bincc & (1 << E));
  CC_F = !!(bincc & (1 << F));
  CC_H = !!(bincc & (1 << H));
  CC_I = !!(bincc & (1 << I));
  CC_N = !!(bincc & (1 << N));
  CC_Z = !!(bincc & (1 << Z));
  CC_V = !!(bincc & (1 << V));
  CC_C = !!(bincc & (1 << C));
}

unsigned char getcc(void)
{
  unsigned char bincc = 0;

#define TST(_CC, _F) if (_CC) { bincc |= (1 << _F); }

  TST(CC_E, E);
  TST(CC_F, F);
  TST(CC_H, H);
  TST(CC_I, I);
  TST(CC_N, N);
  TST(CC_Z, Z);
  TST(CC_V, V);
  TST(CC_C, C);

  return(bincc);
}

void setmd(unsigned char binmd)
{
  MD_NATIVE6309 = !!(binmd & (1 << NATIVE6309));
  MD_FIRQMODE = !!(binmd & (1 << FIRQMODE));
  MD_UNDEFINED2 = !!(binmd & (1 << MD_UNDEF2));
  MD_UNDEFINED3 = !!(binmd & (1 << MD_UNDEF3));
  MD_UNDEFINED4 = !!(binmd & (1 << MD_UNDEF4));
  MD_UNDEFINED5 = !!(binmd & (1 << MD_UNDEF5));
  MD_ILLEGAL = !!(binmd & (1 << ILLEGAL));
  MD_ZERODIV = !!(binmd & (1 << ZERODIV));

  for (short i = 0; i < 24; i++)
  {
    *NatEmuCycles[i] = InsCycles[MD_NATIVE6309][i];
  }
}

unsigned char getmd(void)
{
  unsigned char binmd = 0;

#define TSM(_MD, _F) if (_MD) { binmd |= (1 << _F); } //--Can't use the same macro name

  TSM(MD_NATIVE6309, NATIVE6309);
  TSM(MD_FIRQMODE, FIRQMODE);
  TSM(MD_UNDEFINED2, MD_UNDEF2);
  TSM(MD_UNDEFINED3, MD_UNDEF3);
  TSM(MD_UNDEFINED4, MD_UNDEF4);
  TSM(MD_UNDEFINED5, MD_UNDEF5);
  TSM(MD_ILLEGAL, ILLEGAL);
  TSM(MD_ZERODIV, ZERODIV);

  return(binmd);
}

void HD6309AssertInterrupt(unsigned char interrupt, unsigned char waiter)// 4 nmi 2 firq 1 irq
{
  SyncWaiting = 0;
  PendingInterrupts |= (1 << (interrupt - 1));
  IRQWaiter = waiter;
}

void HD6309DeAssertInterrupt(unsigned char interrupt)// 4 nmi 2 firq 1 irq
{
  PendingInterrupts &= ~(1 << (interrupt - 1));
  InInterrupt = 0;
}

void InvalidInsHandler(void)
{
  MD_ILLEGAL = 1;
  mdbits = getmd();

  ErrorVector();
}

void DivbyZero(void)
{
  MD_ZERODIV = 1;
  mdbits = getmd();

  ErrorVector();
}

void ErrorVector(void)
{
  CC_E = 1;

  MemWrite8(PC_L, --S_REG);
  MemWrite8(PC_H, --S_REG);
  MemWrite8(U_L, --S_REG);
  MemWrite8(U_H, --S_REG);
  MemWrite8(Y_L, --S_REG);
  MemWrite8(Y_H, --S_REG);
  MemWrite8(X_L, --S_REG);
  MemWrite8(X_H, --S_REG);
  MemWrite8(DPA, --S_REG);

  if (MD_NATIVE6309)
  {
    MemWrite8((F_REG), --S_REG);
    MemWrite8((E_REG), --S_REG);

    CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(getcc(), --S_REG);

  PC_REG = MemRead16(VTRAP);

  CycleCounter += (12 + NatEmuCycles54);	//One for each byte +overhead? Guessing from PSHS
}

unsigned char GetSorceReg(unsigned char tmp)
{
  unsigned char source = (tmp >> 4);
  unsigned char dest = tmp & 15;
  unsigned char translate[] = { 0,0 };

  if ((source & 8) == (dest & 8)) { //like size registers
    return(source);
  }

  return(0);
}

void HD6309ForcePC(unsigned short address)
{
  PC_REG = address;

  PendingInterrupts = 0;
  SyncWaiting = 0;
}
