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

#include "hd6309.h"
#include "hd6309defs.h"
#include "hd6309state.h"
#include "hd6309intstate.h"
#include "hd6309vector.h"

#include "hd6309_cc.h"
#include "hd6309_md.h"
#include "hd6309_i.h"

#include "MmuAccessors.h"
#include "MemRead8.h"
#include "MemWrite8.h"
#include "MemRead16.h"
#include "MemWrite16.h"
#include "MemRead32.h"
#include "MemWrite32.h"

void InvalidInsHandler(void);
void DivbyZero(void);
void ErrorVector(void);

void Page_2(void);
void Page_3(void);

int HD6309Exec(int cycleFor)
{
  HD6309State* hd63096State = GetHD6309State();
  HD6309IntState* hd63096IntState = GetHD6309IntState();

  hd63096State->CycleCounter = 0;
  hd63096State->gCycleFor = cycleFor;

  while (hd63096State->CycleCounter < cycleFor) {

    if (hd63096IntState->PendingInterrupts)
    {
      if (hd63096IntState->PendingInterrupts & 4) {
        HD6309_cpu_nmi();
      }

      if (hd63096IntState->PendingInterrupts & 2) {
        HD6309_cpu_firq();
      }

      if (hd63096IntState->PendingInterrupts & 1)
      {
        if (hd63096IntState->IRQWaiter == 0) { // This is needed to fix a subtle timming problem
          HD6309_cpu_irq();		// It allows the CPU to see $FF03 bit 7 high before
        }
        else {				// The IRQ is asserted.
          hd63096IntState->IRQWaiter -= 1;
        }
      }
    }

    if (hd63096State->SyncWaiting == 1) { //Abort the run nothing happens asyncronously from the CPU
      return(0); // WDZ - Experimental SyncWaiting should still return used cycles (and not zero) by breaking from loop
    }

    JmpVec1[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
  }

  return(cycleFor - hd63096State->CycleCounter);
}

void Page_2(void) //10
{
  HD6309State* hd63096State = GetHD6309State();

  JmpVec2[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
}

void Page_3(void) //11
{
  HD6309State* hd63096State = GetHD6309State();

  JmpVec3[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
}

void InvalidInsHandler(void)
{
  HD6309State* hd63096State = GetHD6309State();

  MD_ILLEGAL = 1;
  hd63096State->mdbits = getmd();

  ErrorVector();
}

void DivbyZero(void)
{
  HD6309State* hd63096State = GetHD6309State();

  MD_ZERODIV = 1;
  hd63096State->mdbits = getmd();

  ErrorVector();
}

void ErrorVector(void)
{
  HD6309State* hd63096State = GetHD6309State();

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

    hd63096State->CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(getcc(), --S_REG);

  PC_REG = MemRead16(VTRAP);

  hd63096State->CycleCounter += (12 + hd63096State->NatEmuCycles54);	//One for each byte +overhead? Guessing from PSHS
}
