#include "hd6309defs.h"
#include "hd6309.h"
#include "hd6309state.h"

#include "MemRead8.h"
#include "MemWrite8.h"
#include "MemRead16.h"
#include "MemWrite16.h"

extern CpuRegister pc, x, y, u, s, dp, v, z;
extern WideRegister q;

extern unsigned char cc[8];
extern unsigned char* ureg8[8];
extern unsigned short* xfreg16[8];
extern unsigned int md[8];
extern unsigned char ccbits;
extern unsigned char mdbits;

extern int CycleCounter;
extern unsigned int SyncWaiting;
extern char InInterrupt;

extern unsigned char Source;
extern unsigned char Dest;

extern unsigned char temp8;
extern unsigned short temp16;
extern unsigned int temp32;

extern signed short stemp16;
extern int stemp32;

extern unsigned char postbyte;
extern short unsigned postword;
extern signed char* spostbyte;
extern signed short* spostword;

extern int gCycleFor;

extern unsigned char NatEmuCycles65;
extern unsigned char NatEmuCycles64;
extern unsigned char NatEmuCycles32;
extern unsigned char NatEmuCycles21;
extern unsigned char NatEmuCycles54;
extern unsigned char NatEmuCycles97;
extern unsigned char NatEmuCycles85;
extern unsigned char NatEmuCycles51;
extern unsigned char NatEmuCycles31;
extern unsigned char NatEmuCycles1110;
extern unsigned char NatEmuCycles76;
extern unsigned char NatEmuCycles75;
extern unsigned char NatEmuCycles43;
extern unsigned char NatEmuCycles87;
extern unsigned char NatEmuCycles86;
extern unsigned char NatEmuCycles98;
extern unsigned char NatEmuCycles2726;
extern unsigned char NatEmuCycles3635;
extern unsigned char NatEmuCycles3029;
extern unsigned char NatEmuCycles2827;
extern unsigned char NatEmuCycles3726;
extern unsigned char NatEmuCycles3130;
extern unsigned char NatEmuCycles42;
extern unsigned char NatEmuCycles53;

extern void setcc(unsigned char);
extern unsigned char getcc(void);
extern void setmd(unsigned char);
extern unsigned char getmd(void);

extern void InvalidInsHandler(void);
extern unsigned short hd6309_CalculateEA(unsigned char);
extern void DivbyZero(void);

extern void MemWrite32(unsigned int, unsigned short);
extern unsigned int MemRead32(unsigned short);

void Neg_D(void)
{ //0
  temp16 = DPADDRESS(PC_REG++);
  postbyte = MemRead8(temp16);
  temp8 = 0 - postbyte;
  cc[C] = temp8 > 0;
  cc[V] = (postbyte == 0x80);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles65;
}

void Oim_D(void)
{//1 6309
  postbyte = MemRead8(PC_REG++);
  temp16 = DPADDRESS(PC_REG++);
  postbyte |= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  CycleCounter += 6;
}

void Aim_D(void)
{//2 Phase 2 6309
  postbyte = MemRead8(PC_REG++);
  temp16 = DPADDRESS(PC_REG++);
  postbyte &= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  CycleCounter += 6;
}

void Com_D(void)
{ //03
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  temp8 = 0xFF - temp8;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  cc[C] = 1;
  cc[V] = 0;
  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles65;
}

void Lsr_D(void)
{ //04 S2
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  cc[C] = temp8 & 1;
  temp8 = temp8 >> 1;
  cc[Z] = ZTEST(temp8);
  cc[N] = 0;
  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles65;
}

void Eim_D(void)
{ //05 6309 Untested
  postbyte = MemRead8(PC_REG++);
  temp16 = DPADDRESS(PC_REG++);
  postbyte ^= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  CycleCounter += 6;
}

void Ror_D(void)
{ //06 S2
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  postbyte = cc[C] << 7;
  cc[C] = temp8 & 1;
  temp8 = (temp8 >> 1) | postbyte;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles65;
}

void Asr_D(void)
{ //7
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  cc[C] = temp8 & 1;
  temp8 = (temp8 & 0x80) | (temp8 >> 1);
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles65;
}

void Asl_D(void)
{ //8 
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  cc[C] = (temp8 & 0x80) >> 7;
  cc[V] = cc[C] ^ ((temp8 & 0x40) >> 6);
  temp8 = temp8 << 1;
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles65;
}

void Rol_D(void)
{	//9
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  postbyte = cc[C];
  cc[C] = (temp8 & 0x80) >> 7;
  cc[V] = cc[C] ^ ((temp8 & 0x40) >> 6);
  temp8 = (temp8 << 1) | postbyte;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles65;
}

void Dec_D(void)
{ //A
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16) - 1;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  cc[V] = temp8 == 0x7F;
  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles65;
}

void Tim_D(void)
{	//B 6309 Untested wcreate
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  postbyte &= temp8;
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  CycleCounter += 6;
}

void Inc_D(void)
{ //C
  temp16 = (DPADDRESS(PC_REG++));
  temp8 = MemRead8(temp16) + 1;
  cc[Z] = ZTEST(temp8);
  cc[V] = temp8 == 0x80;
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles65;
}

void Tst_D(void)
{ //D
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  cc[V] = 0;
  CycleCounter += NatEmuCycles64;
}

void Jmp_D(void)
{	//E
  PC_REG = ((dp.Reg | MemRead8(PC_REG)));
  CycleCounter += NatEmuCycles32;
}

void Clr_D(void)
{	//F
  MemWrite8(0, DPADDRESS(PC_REG++));
  cc[Z] = 1;
  cc[N] = 0;
  cc[V] = 0;
  cc[C] = 0;
  CycleCounter += NatEmuCycles65;
}

void LBeq_R(void)
{ //1027
  if (cc[Z]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }
  PC_REG += 2;
  CycleCounter += 5;
}

void LBrn_R(void)
{ //1021
  PC_REG += 2;
  CycleCounter += 5;
}

void LBhi_R(void)
{ //1022
  if (!(cc[C] | cc[Z])) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBls_R(void)
{ //1023
  if (cc[C] | cc[Z]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBhs_R(void)
{ //1024
  if (!cc[C]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 6;
}

void LBcs_R(void)
{ //1025
  if (cc[C]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBne_R(void)
{ //1026
  if (!cc[Z]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBvc_R(void)
{ //1028
  if (!cc[V]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBvs_R(void)
{ //1029
  if (cc[V]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBpl_R(void)
{ //102A
  if (!cc[N]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBmi_R(void)
{ //102B
  if (cc[N]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBge_R(void)
{ //102C
  if (!(cc[N] ^ cc[V])) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBlt_R(void)
{ //102D
  if (cc[V] ^ cc[N]) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBgt_R(void)
{ //102E
  if (!(cc[Z] | (cc[N] ^ cc[V]))) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void LBle_R(void)
{	//102F
  if (cc[Z] | (cc[N] ^ cc[V])) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    CycleCounter += 1;
  }

  PC_REG += 2;
  CycleCounter += 5;
}

void Addr(void)
{ //1030 6309 - WallyZ 2019
  unsigned char dest8 = 0, source8 = 0;
  unsigned short dest16 = 0, source16 = 0;
  temp8 = MemRead8(PC_REG++);
  Source = temp8 >> 4;
  Dest = temp8 & 15;

  if (Dest > 7) { // 8 bit dest
    Dest &= 7;

    if (Dest == 2) {
      dest8 = getcc();
    }
    else {
      dest8 = *ureg8[Dest];
    }

    if (Source > 7) { // 8 bit source
      Source &= 7;

      if (Source == 2) {
        source8 = getcc();
      }
      else {
        source8 = *ureg8[Source];
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)*xfreg16[Source];
    }

    temp16 = source8 + dest8;

    switch (Dest)
    {
    case 2: 				setcc((unsigned char)temp16); break;
    case 4: case 5: break; // never assign to zero reg
    default: 				*ureg8[Dest] = (unsigned char)temp16; break;
    }

    cc[C] = (temp16 & 0x100) >> 8;
    cc[V] = OVERFLOW8(cc[C], source8, dest8, temp16);
    cc[N] = NTEST8(*ureg8[Dest]);
    cc[Z] = ZTEST(*ureg8[Dest]);
  }
  else // 16 bit dest
  {
    dest16 = *xfreg16[Dest];

    if (Source < 8) // 16 bit source
    {
      source16 = *xfreg16[Source];
    }
    else // 8 bit source - promote to 16 bit
    {
      Source &= 7;

      switch (Source)
      {
      case 0: case 1: source16 = D_REG; break; // A & B Reg
      case 2:	        source16 = (unsigned short)getcc(); break; // CC
      case 3:	        source16 = (unsigned short)dp.Reg; break; // DP
      case 4: case 5: source16 = 0; break; // Zero Reg
      case 6: case 7: source16 = W_REG; break; // E & F Reg
      }
    }

    temp32 = source16 + dest16;
    *xfreg16[Dest] = (unsigned short)temp32;
    cc[C] = (temp32 & 0x10000) >> 16;
    cc[V] = OVERFLOW16(cc[C], source16, dest16, temp32);
    cc[N] = NTEST16(*xfreg16[Dest]);
    cc[Z] = ZTEST(*xfreg16[Dest]);
  }

  CycleCounter += 4;
}

void Adcr(void)
{ //1031 6309 - WallyZ 2019
  unsigned char dest8 = 0, source8 = 0;
  unsigned short dest16 = 0, source16 = 0;
  temp8 = MemRead8(PC_REG++);
  Source = temp8 >> 4;
  Dest = temp8 & 15;

  if (Dest > 7) // 8 bit dest
  {
    Dest &= 7;

    if (Dest == 2) {
      dest8 = getcc();
    }
    else {
      dest8 = *ureg8[Dest];
    }

    if (Source > 7) { // 8 bit source
      Source &= 7;

      if (Source == 2) {
        source8 = getcc();
      }
      else {
        source8 = *ureg8[Source];
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)*xfreg16[Source];
    }

    temp16 = source8 + dest8 + cc[C];

    switch (Dest)
    {
    case 2:
      setcc((unsigned char)temp16);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      *ureg8[Dest] = (unsigned char)temp16;
      break;
    }

    cc[C] = (temp16 & 0x100) >> 8;
    cc[V] = OVERFLOW8(cc[C], source8, dest8, temp16);
    cc[N] = NTEST8(*ureg8[Dest]);
    cc[Z] = ZTEST(*ureg8[Dest]);
  }
  else // 16 bit dest
  {
    dest16 = *xfreg16[Dest];

    if (Source < 8) // 16 bit source
    {
      source16 = *xfreg16[Source];
    }
    else // 8 bit source - promote to 16 bit
    {
      Source &= 7;

      switch (Source)
      {
      case 0:
      case 1:
        source16 = D_REG;
        break; // A & B Reg

      case 2:
        source16 = (unsigned short)getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)dp.Reg;
        break; // DP

      case 4:
      case 5:
        source16 = 0;
        break; // Zero Reg

      case 6:
      case 7:
        source16 = W_REG;
        break; // E & F Reg
      }
    }

    temp32 = source16 + dest16 + cc[C];
    *xfreg16[Dest] = (unsigned short)temp32;
    cc[C] = (temp32 & 0x10000) >> 16;
    cc[V] = OVERFLOW16(cc[C], source16, dest16, temp32);
    cc[N] = NTEST16(*xfreg16[Dest]);		cc[Z] = ZTEST(*xfreg16[Dest]);
  }

  CycleCounter += 4;
}

void Subr(void)
{ //1032 6309 - WallyZ 2019
  unsigned char dest8 = 0, source8 = 0;
  unsigned short dest16 = 0, source16 = 0;
  temp8 = MemRead8(PC_REG++);
  Source = temp8 >> 4;
  Dest = temp8 & 15;

  if (Dest > 7) // 8 bit dest
  {
    Dest &= 7;

    if (Dest == 2) {
      dest8 = getcc();
    }
    else {
      dest8 = *ureg8[Dest];
    }

    if (Source > 7) { // 8 bit source
      Source &= 7;

      if (Source == 2) {
        source8 = getcc();
      }
      else {
        source8 = *ureg8[Source];
      }
    }
    else { // 16 bit source - demote to 8 bit
      Source &= 7;
      source8 = (unsigned char)*xfreg16[Source];
    }

    temp16 = dest8 - source8;

    switch (Dest)
    {
    case 2:
      setcc((unsigned char)temp16);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      *ureg8[Dest] = (unsigned char)temp16;
      break;
    }

    cc[C] = (temp16 & 0x100) >> 8;
    cc[V] = cc[C] ^ ((dest8 ^ *ureg8[Dest] ^ source8) >> 7);
    cc[N] = *ureg8[Dest] >> 7;
    cc[Z] = ZTEST(*ureg8[Dest]);
  }
  else // 16 bit dest
  {
    dest16 = *xfreg16[Dest];

    if (Source < 8) // 16 bit source
    {
      source16 = *xfreg16[Source];
    }
    else // 8 bit source - promote to 16 bit
    {
      Source &= 7;

      switch (Source)
      {
      case 0:
      case 1:
        source16 = D_REG;
        break; // A & B Reg

      case 2:
        source16 = (unsigned short)getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)dp.Reg;
        break; // DP

      case 4:
      case 5:
        source16 = 0;
        break; // Zero Reg

      case 6:
      case 7:
        source16 = W_REG;
        break; // E & F Reg
      }
    }

    temp32 = dest16 - source16;
    cc[C] = (temp32 & 0x10000) >> 16;
    cc[V] = !!((dest16 ^ source16 ^ temp32 ^ (temp32 >> 1)) & 0x8000);
    *xfreg16[Dest] = (unsigned short)temp32;
    cc[N] = (temp32 & 0x8000) >> 15;
    cc[Z] = ZTEST(temp32);
  }

  CycleCounter += 4;
}

void Sbcr(void)
{ //1033 6309 - WallyZ 2019
  unsigned char dest8 = 0, source8 = 0;
  unsigned short dest16 = 0, source16 = 0;
  temp8 = MemRead8(PC_REG++);
  Source = temp8 >> 4;
  Dest = temp8 & 15;

  if (Dest > 7) // 8 bit dest
  {
    Dest &= 7;

    if (Dest == 2) {
      dest8 = getcc();
    }
    else {
      dest8 = *ureg8[Dest];
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = getcc();
      }
      else {
        source8 = *ureg8[Source];
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)*xfreg16[Source];
    }

    temp16 = dest8 - source8 - cc[C];

    switch (Dest)
    {
    case 2:
      setcc((unsigned char)temp16);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      *ureg8[Dest] = (unsigned char)temp16;
      break;
    }

    cc[C] = (temp16 & 0x100) >> 8;
    cc[V] = cc[C] ^ ((dest8 ^ *ureg8[Dest] ^ source8) >> 7);
    cc[N] = *ureg8[Dest] >> 7;
    cc[Z] = ZTEST(*ureg8[Dest]);
  }
  else // 16 bit dest
  {
    dest16 = *xfreg16[Dest];

    if (Source < 8) // 16 bit source
    {
      source16 = *xfreg16[Source];
    }
    else // 8 bit source - promote to 16 bit
    {
      Source &= 7;

      switch (Source)
      {
      case 0:
      case 1:
        source16 = D_REG;
        break; // A & B Reg

      case 2:
        source16 = (unsigned short)getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)dp.Reg;
        break; // DP

      case 4:
      case 5:
        source16 = 0;
        break; // Zero Reg

      case 6:
      case 7:
        source16 = W_REG;
        break; // E & F Reg
      }
    }

    temp32 = dest16 - source16 - cc[C];
    cc[C] = (temp32 & 0x10000) >> 16;
    cc[V] = !!((dest16 ^ source16 ^ temp32 ^ (temp32 >> 1)) & 0x8000);
    *xfreg16[Dest] = (unsigned short)temp32;
    cc[N] = (temp32 & 0x8000) >> 15;
    cc[Z] = ZTEST(temp32);
  }

  CycleCounter += 4;
}

void Andr(void)
{ //1034 6309 - WallyZ 2019
  unsigned char dest8 = 0, source8 = 0;
  unsigned short dest16 = 0, source16 = 0;
  temp8 = MemRead8(PC_REG++);
  Source = temp8 >> 4;
  Dest = temp8 & 15;

  if (Dest > 7) // 8 bit dest
  {
    Dest &= 7;

    if (Dest == 2) {
      dest8 = getcc();
    }
    else {
      dest8 = *ureg8[Dest];
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = getcc();
      }
      else {
        source8 = *ureg8[Source];
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)*xfreg16[Source];
    }

    temp8 = dest8 & source8;

    switch (Dest)
    {
    case 2:
      setcc((unsigned char)temp8);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      *ureg8[Dest] = (unsigned char)temp8;
      break;
    }

    cc[N] = temp8 >> 7;
    cc[Z] = ZTEST(temp8);
  }
  else // 16 bit dest
  {
    dest16 = *xfreg16[Dest];

    if (Source < 8) // 16 bit source
    {
      source16 = *xfreg16[Source];
    }
    else // 8 bit source - promote to 16 bit
    {
      Source &= 7;
      switch (Source)
      {
      case 0:
      case 1:
        source16 = D_REG;
        break; // A & B Reg

      case 2:
        source16 = (unsigned short)getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)dp.Reg;
        break; // DP

      case 4:
      case 5:
        source16 = 0;
        break; // Zero Reg

      case 6:
      case 7:
        source16 = W_REG;
        break; // E & F Reg
      }
    }

    temp16 = dest16 & source16;
    *xfreg16[Dest] = temp16;
    cc[N] = temp16 >> 15;
    cc[Z] = ZTEST(temp16);
  }
  cc[V] = 0;
  CycleCounter += 4;
}

void Orr(void)
{ //1035 6309 - WallyZ 2019
  unsigned char dest8 = 0, source8 = 0;
  unsigned short dest16 = 0, source16 = 0;
  temp8 = MemRead8(PC_REG++);
  Source = temp8 >> 4;
  Dest = temp8 & 15;

  if (Dest > 7) // 8 bit dest
  {
    Dest &= 7;

    if (Dest == 2) {
      dest8 = getcc();
    }
    else {
      dest8 = *ureg8[Dest];
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = getcc();
      }
      else {
        source8 = *ureg8[Source];
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)*xfreg16[Source];
    }

    temp8 = dest8 | source8;

    switch (Dest)
    {
    case 2:
      setcc((unsigned char)temp8);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      *ureg8[Dest] = (unsigned char)temp8;
      break;
    }

    cc[N] = temp8 >> 7;
    cc[Z] = ZTEST(temp8);
  }
  else // 16 bit dest
  {
    dest16 = *xfreg16[Dest];

    if (Source < 8) // 16 bit source
    {
      source16 = *xfreg16[Source];
    }
    else // 8 bit source - promote to 16 bit
    {
      Source &= 7;

      switch (Source)
      {
      case 0:
      case 1:
        source16 = D_REG;
        break; // A & B Reg

      case 2:
        source16 = (unsigned short)getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)dp.Reg;
        break; // DP

      case 4:
      case 5:
        source16 = 0;
        break; // Zero Reg

      case 6:
      case 7:
        source16 = W_REG;
        break; // E & F Reg
      }
    }

    temp16 = dest16 | source16;
    *xfreg16[Dest] = temp16;
    cc[N] = temp16 >> 15;
    cc[Z] = ZTEST(temp16);
  }

  cc[V] = 0;
  CycleCounter += 4;
}

void Eorr(void)
{ //1036 6309 - WallyZ 2019
  unsigned char dest8 = 0, source8 = 0;
  unsigned short dest16 = 0, source16 = 0;
  temp8 = MemRead8(PC_REG++);
  Source = temp8 >> 4;
  Dest = temp8 & 15;

  if (Dest > 7) // 8 bit dest
  {
    Dest &= 7;

    if (Dest == 2) {
      dest8 = getcc();
    }
    else {
      dest8 = *ureg8[Dest];
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = getcc();
      }
      else {
        source8 = *ureg8[Source];
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)*xfreg16[Source];
    }

    temp8 = dest8 ^ source8;

    switch (Dest)
    {
    case 2:
      setcc((unsigned char)temp8);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      *ureg8[Dest] = (unsigned char)temp8;
      break;
    }

    cc[N] = temp8 >> 7;
    cc[Z] = ZTEST(temp8);
  }
  else // 16 bit dest
  {
    dest16 = *xfreg16[Dest];

    if (Source < 8) // 16 bit source
    {
      source16 = *xfreg16[Source];
    }
    else // 8 bit source - promote to 16 bit
    {
      Source &= 7;

      switch (Source)
      {
      case 0:
      case 1:
        source16 = D_REG;
        break; // A & B Reg

      case 2:
        source16 = (unsigned short)getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)dp.Reg;
        break; // DP

      case 4:
      case 5:
        source16 = 0;
        break; // Zero Reg

      case 6:
      case 7:
        source16 = W_REG;
        break; // E & F Reg
      }
    }

    temp16 = dest16 ^ source16;
    *xfreg16[Dest] = temp16;
    cc[N] = temp16 >> 15;
    cc[Z] = ZTEST(temp16);
  }

  cc[V] = 0;
  CycleCounter += 4;
}

void Cmpr(void)
{ //1037 6309 - WallyZ 2019
  unsigned char dest8 = 0, source8 = 0;
  unsigned short dest16 = 0, source16 = 0;
  temp8 = MemRead8(PC_REG++);
  Source = temp8 >> 4;
  Dest = temp8 & 15;

  if (Dest > 7) // 8 bit dest
  {
    Dest &= 7;

    if (Dest == 2) {
      dest8 = getcc();
    }
    else {
      dest8 = *ureg8[Dest];
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = getcc();
      }
      else {
        source8 = *ureg8[Source];
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)*xfreg16[Source];
    }

    temp16 = dest8 - source8;
    temp8 = (unsigned char)temp16;
    cc[C] = (temp16 & 0x100) >> 8;
    cc[V] = cc[C] ^ ((dest8 ^ temp8 ^ source8) >> 7);
    cc[N] = temp8 >> 7;
    cc[Z] = ZTEST(temp8);
  }
  else // 16 bit dest
  {
    dest16 = *xfreg16[Dest];

    if (Source < 8) // 16 bit source
    {
      source16 = *xfreg16[Source];
    }
    else // 8 bit source - promote to 16 bit
    {
      Source &= 7;

      switch (Source)
      {
      case 0:
      case 1:
        source16 = D_REG;
        break; // A & B Reg

      case 2:
        source16 = (unsigned short)getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)dp.Reg;
        break; // DP

      case 4:
      case 5:
        source16 = 0;
        break; // Zero Reg

      case 6:
      case 7:
        source16 = W_REG;
        break; // E & F Reg
      }
    }

    temp32 = dest16 - source16;
    cc[C] = (temp32 & 0x10000) >> 16;
    cc[V] = !!((dest16 ^ source16 ^ temp32 ^ (temp32 >> 1)) & 0x8000);
    cc[N] = (temp32 & 0x8000) >> 15;
    cc[Z] = ZTEST(temp32);
  }

  CycleCounter += 4;
}

void Pshsw(void)
{ //1038 DONE 6309
  MemWrite8((F_REG), --S_REG);
  MemWrite8((E_REG), --S_REG);
  CycleCounter += 6;
}

void Pulsw(void)
{	//1039 6309 Untested wcreate
  E_REG = MemRead8(S_REG++);
  F_REG = MemRead8(S_REG++);
  CycleCounter += 6;
}

void Pshuw(void)
{ //103A 6309 Untested
  MemWrite8((F_REG), --U_REG);
  MemWrite8((E_REG), --U_REG);
  CycleCounter += 6;
}

void Puluw(void)
{ //103B 6309 Untested
  E_REG = MemRead8(U_REG++);
  F_REG = MemRead8(U_REG++);
  CycleCounter += 6;
}

void Swi2_I(void)
{ //103F
  cc[E] = 1;
  MemWrite8(pc.B.lsb, --S_REG);
  MemWrite8(pc.B.msb, --S_REG);
  MemWrite8(u.B.lsb, --S_REG);
  MemWrite8(u.B.msb, --S_REG);
  MemWrite8(y.B.lsb, --S_REG);
  MemWrite8(y.B.msb, --S_REG);
  MemWrite8(x.B.lsb, --S_REG);
  MemWrite8(x.B.msb, --S_REG);
  MemWrite8(dp.B.msb, --S_REG);

  if (md[NATIVE6309])
  {
    MemWrite8((F_REG), --S_REG);
    MemWrite8((E_REG), --S_REG);
    CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(getcc(), --S_REG);
  PC_REG = MemRead16(VSWI2);
  CycleCounter += 20;
}

void Negd_I(void)
{ //1040 Phase 5 6309
  D_REG = 0 - D_REG;
  cc[C] = temp16 > 0;
  cc[V] = D_REG == 0x8000;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  CycleCounter += NatEmuCycles32;
}

void Comd_I(void)
{ //1043 6309
  D_REG = 0xFFFF - D_REG;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  cc[C] = 1;
  cc[V] = 0;
  CycleCounter += NatEmuCycles32;
}

void Lsrd_I(void)
{ //1044 6309
  cc[C] = D_REG & 1;
  D_REG = D_REG >> 1;
  cc[Z] = ZTEST(D_REG);
  cc[N] = 0;
  CycleCounter += NatEmuCycles32;
}

void Rord_I(void)
{ //1046 6309 Untested
  postword = cc[C] << 15;
  cc[C] = D_REG & 1;
  D_REG = (D_REG >> 1) | postword;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  CycleCounter += NatEmuCycles32;
}

void Asrd_I(void)
{ //1047 6309 Untested TESTED NITRO MULTIVUE
  cc[C] = D_REG & 1;
  D_REG = (D_REG & 0x8000) | (D_REG >> 1);
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  CycleCounter += NatEmuCycles32;
}

void Asld_I(void)
{ //1048 6309
  cc[C] = D_REG >> 15;
  cc[V] = cc[C] ^ ((D_REG & 0x4000) >> 14);
  D_REG = D_REG << 1;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  CycleCounter += NatEmuCycles32;
}

void Rold_I(void)
{ //1049 6309 Untested
  postword = cc[C];
  cc[C] = D_REG >> 15;
  cc[V] = cc[C] ^ ((D_REG & 0x4000) >> 14);
  D_REG = (D_REG << 1) | postword;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  CycleCounter += NatEmuCycles32;
}

void Decd_I(void)
{ //104A 6309
  D_REG--;
  cc[Z] = ZTEST(D_REG);
  cc[V] = D_REG == 0x7FFF;
  cc[N] = NTEST16(D_REG);
  CycleCounter += NatEmuCycles32;
}

void Incd_I(void)
{ //104C 6309
  D_REG++;
  cc[Z] = ZTEST(D_REG);
  cc[V] = D_REG == 0x8000;
  cc[N] = NTEST16(D_REG);
  CycleCounter += NatEmuCycles32;
}

void Tstd_I(void)
{ //104D 6309
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles32;
}

void Clrd_I(void)
{ //104F 6309
  D_REG = 0;
  cc[C] = 0;
  cc[V] = 0;
  cc[N] = 0;
  cc[Z] = 1;
  CycleCounter += NatEmuCycles32;
}

void Comw_I(void)
{ //1053 6309 Untested
  W_REG = 0xFFFF - W_REG;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  cc[C] = 1;
  cc[V] = 0;
  CycleCounter += NatEmuCycles32;
}

void Lsrw_I(void)
{ //1054 6309 Untested
  cc[C] = W_REG & 1;
  W_REG = W_REG >> 1;
  cc[Z] = ZTEST(W_REG);
  cc[N] = 0;
  CycleCounter += NatEmuCycles32;
}

void Rorw_I(void)
{ //1056 6309 Untested
  postword = cc[C] << 15;
  cc[C] = W_REG & 1;
  W_REG = (W_REG >> 1) | postword;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  CycleCounter += NatEmuCycles32;
}

void Rolw_I(void)
{ //1059 6309
  postword = cc[C];
  cc[C] = W_REG >> 15;
  cc[V] = cc[C] ^ ((W_REG & 0x4000) >> 14);
  W_REG = (W_REG << 1) | postword;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  CycleCounter += NatEmuCycles32;
}

void Decw_I(void)
{ //105A 6309
  W_REG--;
  cc[Z] = ZTEST(W_REG);
  cc[V] = W_REG == 0x7FFF;
  cc[N] = NTEST16(W_REG);
  CycleCounter += NatEmuCycles32;
}

void Incw_I(void)
{ //105C 6309
  W_REG++;
  cc[Z] = ZTEST(W_REG);
  cc[V] = W_REG == 0x8000;
  cc[N] = NTEST16(W_REG);
  CycleCounter += NatEmuCycles32;
}

void Tstw_I(void)
{ //105D Untested 6309 wcreate
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles32;
}

void Clrw_I(void)
{ //105F 6309
  W_REG = 0;
  cc[C] = 0;
  cc[V] = 0;
  cc[N] = 0;
  cc[Z] = 1;
  CycleCounter += NatEmuCycles32;
}

void Subw_M(void)
{ //1080 6309 CHECK
  postword = IMMADDRESS(PC_REG);
  temp16 = W_REG - postword;
  cc[C] = temp16 > W_REG;
  cc[V] = OVERFLOW16(cc[C], temp16, W_REG, postword);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  W_REG = temp16;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Cmpw_M(void)
{ //1081 6309 CHECK
  postword = IMMADDRESS(PC_REG);
  temp16 = W_REG - postword;
  cc[C] = temp16 > W_REG;
  cc[V] = OVERFLOW16(cc[C], temp16, W_REG, postword);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Sbcd_M(void)
{ //1082 6309
  postword = IMMADDRESS(PC_REG);
  temp32 = D_REG - postword - cc[C];
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, D_REG, postword);
  D_REG = temp32;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Cmpd_M(void)
{ //1083
  postword = IMMADDRESS(PC_REG);
  temp16 = D_REG - postword;
  cc[C] = temp16 > D_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, D_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Andd_M(void)
{ //1084 6309
  D_REG &= IMMADDRESS(PC_REG);
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Bitd_M(void)
{ //1085 6309 Untested
  temp16 = D_REG & IMMADDRESS(PC_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Ldw_M(void)
{ //1086 6309
  W_REG = IMMADDRESS(PC_REG);
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Eord_M(void)
{ //1088 6309 Untested
  D_REG ^= IMMADDRESS(PC_REG);
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Adcd_M(void)
{ //1089 6309
  postword = IMMADDRESS(PC_REG);
  temp32 = D_REG + postword + cc[C];
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], postword, temp32, D_REG);
  cc[H] = ((D_REG ^ temp32 ^ postword) & 0x100) >> 8;
  D_REG = temp32;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Ord_M(void)
{ //108A 6309 Untested
  D_REG |= IMMADDRESS(PC_REG);
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Addw_M(void)
{ //108B Phase 5 6309
  temp16 = IMMADDRESS(PC_REG);
  temp32 = W_REG + temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, W_REG);
  W_REG = temp32;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Cmpy_M(void)
{ //108C
  postword = IMMADDRESS(PC_REG);
  temp16 = Y_REG - postword;
  cc[C] = temp16 > Y_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, Y_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Ldy_M(void)
{ //108E
  Y_REG = IMMADDRESS(PC_REG);
  cc[Z] = ZTEST(Y_REG);
  cc[N] = NTEST16(Y_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Subw_D(void)
{ //1090 Untested 6309
  temp16 = MemRead16(DPADDRESS(PC_REG++));
  temp32 = W_REG - temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, W_REG);
  W_REG = temp32;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  CycleCounter += NatEmuCycles75;
}

void Cmpw_D(void)
{ //1091 6309 Untested
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = W_REG - postword;
  cc[C] = temp16 > W_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, W_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles75;
}

void Sbcd_D(void)
{ //1092 6309
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp32 = D_REG - postword - cc[C];
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, D_REG, postword);
  D_REG = temp32;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  CycleCounter += NatEmuCycles75;
}

void Cmpd_D(void)
{ //1093
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = D_REG - postword;
  cc[C] = temp16 > D_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, D_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles75;
}

void Andd_D(void)
{ //1094 6309 Untested
  postword = MemRead16(DPADDRESS(PC_REG++));
  D_REG &= postword;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles75;
}

void Bitd_D(void)
{ //1095 6309 Untested
  temp16 = D_REG & MemRead16(DPADDRESS(PC_REG++));
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  cc[V] = 0;
  CycleCounter += NatEmuCycles75;
}

void Ldw_D(void)
{ //1096 6309
  W_REG = MemRead16(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles65;
}

void Stw_D(void)
{ //1097 6309
  MemWrite16(W_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles65;
}

void Eord_D(void)
{ //1098 6309 Untested
  D_REG ^= MemRead16(DPADDRESS(PC_REG++));
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles75;
}

void Adcd_D(void)
{ //1099 6309
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp32 = D_REG + postword + cc[C];
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], postword, temp32, D_REG);
  cc[H] = ((D_REG ^ temp32 ^ postword) & 0x100) >> 8;
  D_REG = temp32;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  CycleCounter += NatEmuCycles75;
}

void Ord_D(void)
{ //109A 6309 Untested
  D_REG |= MemRead16(DPADDRESS(PC_REG++));
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles75;
}

void Addw_D(void)
{ //109B 6309
  temp16 = MemRead16(DPADDRESS(PC_REG++));
  temp32 = W_REG + temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, W_REG);
  W_REG = temp32;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  CycleCounter += NatEmuCycles75;
}

void Cmpy_D(void)
{	//109C
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = Y_REG - postword;
  cc[C] = temp16 > Y_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, Y_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles75;
}

void Ldy_D(void)
{ //109E
  Y_REG = MemRead16(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(Y_REG);
  cc[N] = NTEST16(Y_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles65;
}

void Sty_D(void)
{ //109F
  MemWrite16(Y_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(Y_REG);
  cc[N] = NTEST16(Y_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles65;
}

void Subw_X(void)
{ //10A0 6309 MODDED
  temp16 = MemRead16(INDADDRESS(PC_REG++));
  temp32 = W_REG - temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, W_REG);
  W_REG = temp32;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  CycleCounter += NatEmuCycles76;
}

void Cmpw_X(void)
{ //10A1 6309
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = W_REG - postword;
  cc[C] = temp16 > W_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, W_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles76;
}

void Sbcd_X(void)
{ //10A2 6309
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp32 = D_REG - postword - cc[C];
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], postword, temp32, D_REG);
  D_REG = temp32;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  CycleCounter += NatEmuCycles76;
}

void Cmpd_X(void)
{ //10A3
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = D_REG - postword;
  cc[C] = temp16 > D_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, D_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles76;
}

void Andd_X(void)
{ //10A4 6309
  D_REG &= MemRead16(INDADDRESS(PC_REG++));
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles76;
}

void Bitd_X(void)
{ //10A5 6309 Untested
  temp16 = D_REG & MemRead16(INDADDRESS(PC_REG++));
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  cc[V] = 0;
  CycleCounter += NatEmuCycles76;
}

void Ldw_X(void)
{ //10A6 6309
  W_REG = MemRead16(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  cc[V] = 0;
  CycleCounter += 6;
}

void Stw_X(void)
{ //10A7 6309
  MemWrite16(W_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  cc[V] = 0;
  CycleCounter += 6;
}

void Eord_X(void)
{ //10A8 6309 Untested TESTED NITRO 
  D_REG ^= MemRead16(INDADDRESS(PC_REG++));
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles76;
}

void Adcd_X(void)
{ //10A9 6309 untested
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp32 = D_REG + postword + cc[C];
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], postword, temp32, D_REG);
  cc[H] = (((D_REG ^ temp32 ^ postword) & 0x100) >> 8);
  D_REG = temp32;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  CycleCounter += NatEmuCycles76;
}

void Ord_X(void)
{ //10AA 6309 Untested wcreate
  D_REG |= MemRead16(INDADDRESS(PC_REG++));
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles76;
}

void Addw_X(void)
{ //10AB 6309 Untested TESTED NITRO CHECK no Half carry?
  temp16 = MemRead16(INDADDRESS(PC_REG++));
  temp32 = W_REG + temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, W_REG);
  W_REG = temp32;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  CycleCounter += NatEmuCycles76;
}

void Cmpy_X(void)
{ //10AC
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = Y_REG - postword;
  cc[C] = temp16 > Y_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, Y_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles76;
}

void Ldy_X(void)
{ //10AE
  Y_REG = MemRead16(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(Y_REG);
  cc[N] = NTEST16(Y_REG);
  cc[V] = 0;
  CycleCounter += 6;
}

void Sty_X(void)
{ //10AF
  MemWrite16(Y_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(Y_REG);
  cc[N] = NTEST16(Y_REG);
  cc[V] = 0;
  CycleCounter += 6;
}

void Subw_E(void)
{ //10B0 6309 Untested
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = W_REG - temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, W_REG);
  W_REG = temp32;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Cmpw_E(void)
{ //10B1 6309 Untested
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = W_REG - postword;
  cc[C] = temp16 > W_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, W_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Sbcd_E(void)
{ //10B2 6309 Untested
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = D_REG - temp16 - cc[C];
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, D_REG);
  D_REG = temp32;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Cmpd_E(void)
{ //10B3
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = D_REG - postword;
  cc[C] = temp16 > D_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, D_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Andd_E(void)
{ //10B4 6309 Untested
  D_REG &= MemRead16(IMMADDRESS(PC_REG));
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Bitd_E(void)
{ //10B5 6309 Untested CHECK NITRO
  temp16 = D_REG & MemRead16(IMMADDRESS(PC_REG));
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Ldw_E(void)
{ //10B6 6309
  W_REG = MemRead16(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Stw_E(void)
{ //10B7 6309
  MemWrite16(W_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Eord_E(void)
{ //10B8 6309 Untested
  D_REG ^= MemRead16(IMMADDRESS(PC_REG));
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Adcd_E(void)
{ //10B9 6309 untested
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp32 = D_REG + postword + cc[C];
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], postword, temp32, D_REG);
  cc[H] = (((D_REG ^ temp32 ^ postword) & 0x100) >> 8);
  D_REG = temp32;
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Ord_E(void)
{ //10BA 6309 Untested
  D_REG |= MemRead16(IMMADDRESS(PC_REG));
  cc[N] = NTEST16(D_REG);
  cc[Z] = ZTEST(D_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Addw_E(void)
{ //10BB 6309 Untested
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = W_REG + temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, W_REG);
  W_REG = temp32;
  cc[Z] = ZTEST(W_REG);
  cc[N] = NTEST16(W_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Cmpy_E(void)
{ //10BC
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = Y_REG - postword;
  cc[C] = temp16 > Y_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, Y_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Ldy_E(void)
{ //10BE
  Y_REG = MemRead16(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(Y_REG);
  cc[N] = NTEST16(Y_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Sty_E(void)
{ //10BF
  MemWrite16(Y_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(Y_REG);
  cc[N] = NTEST16(Y_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Lds_I(void)
{  //10CE
  S_REG = IMMADDRESS(PC_REG);
  cc[Z] = ZTEST(S_REG);
  cc[N] = NTEST16(S_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += 4;
}

void Ldq_D(void)
{ //10DC 6309
  Q_REG = MemRead32(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(Q_REG);
  cc[N] = NTEST32(Q_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles87;
}

void Stq_D(void)
{ //10DD 6309
  MemWrite32(Q_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(Q_REG);
  cc[N] = NTEST32(Q_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles87;
}

void Lds_D(void)
{ //10DE
  S_REG = MemRead16(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(S_REG);
  cc[N] = NTEST16(S_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles65;
}

void Sts_D(void)
{ //10DF 6309
  MemWrite16(S_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(S_REG);
  cc[N] = NTEST16(S_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles65;
}

void Ldq_X(void)
{ //10EC 6309
  Q_REG = MemRead32(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(Q_REG);
  cc[N] = NTEST32(Q_REG);
  cc[V] = 0;
  CycleCounter += 8;
}

void Stq_X(void)
{ //10ED 6309 DONE
  MemWrite32(Q_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(Q_REG);
  cc[N] = NTEST32(Q_REG);
  cc[V] = 0;
  CycleCounter += 8;
}

void Lds_X(void)
{ //10EE
  S_REG = MemRead16(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(S_REG);
  cc[N] = NTEST16(S_REG);
  cc[V] = 0;
  CycleCounter += 6;
}

void Sts_X(void)
{ //10EF 6309
  MemWrite16(S_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(S_REG);
  cc[N] = NTEST16(S_REG);
  cc[V] = 0;
  CycleCounter += 6;
}

void Ldq_E(void)
{ //10FC 6309
  Q_REG = MemRead32(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(Q_REG);
  cc[N] = NTEST32(Q_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles98;
}

void Stq_E(void)
{ //10FD 6309
  MemWrite32(Q_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(Q_REG);
  cc[N] = NTEST32(Q_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles98;
}

void Lds_E(void)
{ //10FE
  S_REG = MemRead16(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(S_REG);
  cc[N] = NTEST16(S_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Sts_E(void)
{ //10FF 6309
  MemWrite16(S_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(S_REG);
  cc[N] = NTEST16(S_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Band(void)
{ //1130 6309 untested
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  Source = (postbyte >> 3) & 7;
  Dest = (postbyte) & 7;
  postbyte >>= 6;

  if (postbyte == 3)
  {
    InvalidInsHandler();
    return;
  }

  if ((temp8 & (1 << Source)) == 0)
  {
    switch (postbyte)
    {
    case 0: // A Reg
    case 1: // B Reg
      *ureg8[postbyte] &= ~(1 << Dest);
      break;

    case 2: // CC Reg
      setcc(getcc() & ~(1 << Dest));
      break;
    }
  }

  // Else nothing changes
  CycleCounter += NatEmuCycles76;
}

void Biand(void)
{ //1131 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  Source = (postbyte >> 3) & 7;
  Dest = (postbyte) & 7;
  postbyte >>= 6;

  if (postbyte == 3)
  {
    InvalidInsHandler();
    return;
  }

  if ((temp8 & (1 << Source)) != 0)
  {
    switch (postbyte)
    {
    case 0: // A Reg
    case 1: // B Reg
      *ureg8[postbyte] &= ~(1 << Dest);
      break;

    case 2: // CC Reg
      setcc(getcc() & ~(1 << Dest));
      break;
    }
  }

  // Else do nothing
  CycleCounter += NatEmuCycles76;
}

void Bor(void)
{ //1132 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  Source = (postbyte >> 3) & 7;
  Dest = (postbyte) & 7;
  postbyte >>= 6;

  if (postbyte == 3)
  {
    InvalidInsHandler();
    return;
  }

  if ((temp8 & (1 << Source)) != 0)
  {
    switch (postbyte)
    {
    case 0: // A Reg
    case 1: // B Reg
      *ureg8[postbyte] |= (1 << Dest);
      break;

    case 2: // CC Reg
      setcc(getcc() | (1 << Dest));
      break;
    }
  }

  // Else do nothing
  CycleCounter += NatEmuCycles76;
}

void Bior(void)
{ //1133 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  Source = (postbyte >> 3) & 7;
  Dest = (postbyte) & 7;
  postbyte >>= 6;

  if (postbyte == 3)
  {
    InvalidInsHandler();
    return;
  }

  if ((temp8 & (1 << Source)) == 0)
  {
    switch (postbyte)
    {
    case 0: // A Reg
    case 1: // B Reg
      *ureg8[postbyte] |= (1 << Dest);
      break;

    case 2: // CC Reg
      setcc(getcc() | (1 << Dest));
      break;
    }
  }

  // Else do nothing
  CycleCounter += NatEmuCycles76;
}

void Beor(void)
{ //1134 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  Source = (postbyte >> 3) & 7;
  Dest = (postbyte) & 7;
  postbyte >>= 6;

  if (postbyte == 3)
  {
    InvalidInsHandler();
    return;
  }

  if ((temp8 & (1 << Source)) != 0)
  {
    switch (postbyte)
    {
    case 0: // A Reg
    case 1: // B Reg
      *ureg8[postbyte] ^= (1 << Dest);
      break;

    case 2: // CC Reg
      setcc(getcc() ^ (1 << Dest));
      break;
    }
  }

  CycleCounter += NatEmuCycles76;
}

void Bieor(void)
{ //1135 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  Source = (postbyte >> 3) & 7;
  Dest = (postbyte) & 7;
  postbyte >>= 6;

  if (postbyte == 3)
  {
    InvalidInsHandler();
    return;
  }

  if ((temp8 & (1 << Source)) == 0)
  {
    switch (postbyte)
    {
    case 0: // A Reg
    case 1: // B Reg
      *ureg8[postbyte] ^= (1 << Dest);
      break;

    case 2: // CC Reg
      setcc(getcc() ^ (1 << Dest));
      break;
    }
  }

  CycleCounter += NatEmuCycles76;
}

void Ldbt(void)
{ //1136 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  Source = (postbyte >> 3) & 7;
  Dest = (postbyte) & 7;
  postbyte >>= 6;

  if (postbyte == 3)
  {
    InvalidInsHandler();
    return;
  }

  if ((temp8 & (1 << Source)) != 0)
  {
    switch (postbyte)
    {
    case 0: // A Reg
    case 1: // B Reg
      *ureg8[postbyte] |= (1 << Dest);
      break;

    case 2: // CC Reg
      setcc(getcc() | (1 << Dest));
      break;
    }
  }
  else
  {
    switch (postbyte)
    {
    case 0: // A Reg
    case 1: // B Reg
      *ureg8[postbyte] &= ~(1 << Dest);
      break;

    case 2: // CC Reg
      setcc(getcc() & ~(1 << Dest));
      break;
    }
  }

  CycleCounter += NatEmuCycles76;
}

void Stbt(void)
{ //1137 6309
  postbyte = MemRead8(PC_REG++);
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  Source = (postbyte >> 3) & 7;
  Dest = (postbyte) & 7;
  postbyte >>= 6;

  if (postbyte == 3)
  {
    InvalidInsHandler();
    return;
  }

  switch (postbyte)
  {
  case 0: // A Reg
  case 1: // B Reg
    postbyte = *ureg8[postbyte];
    break;

  case 2: // CC Reg
    postbyte = getcc();
    break;
  }

  if ((postbyte & (1 << Source)) != 0)
  {
    temp8 |= (1 << Dest);
  }
  else
  {
    temp8 &= ~(1 << Dest);
  }

  MemWrite8(temp8, temp16);
  CycleCounter += NatEmuCycles87;
}

void Tfm1(void)
{ //1138 TFM R+,R+ 6309
  if (W_REG == 0)
  {
    CycleCounter += 6;
    PC_REG++;
    return;
  }

  postbyte = MemRead8(PC_REG);
  Source = postbyte >> 4;
  Dest = postbyte & 15;

  if (Source > 4 || Dest > 4)
  {
    InvalidInsHandler();
    return;
  }

  temp8 = MemRead8(*xfreg16[Source]);
  MemWrite8(temp8, *xfreg16[Dest]);
  (*xfreg16[Dest])++;
  (*xfreg16[Source])++;
  W_REG--;
  CycleCounter += 3;
  PC_REG -= 2;
}

void Tfm2(void)
{ //1139 TFM R-,R- Phase 3 6309
  if (W_REG == 0)
  {
    CycleCounter += 6;
    PC_REG++;
    return;
  }

  postbyte = MemRead8(PC_REG);
  Source = postbyte >> 4;
  Dest = postbyte & 15;

  if (Source > 4 || Dest > 4)
  {
    InvalidInsHandler();
    return;
  }

  temp8 = MemRead8(*xfreg16[Source]);
  MemWrite8(temp8, *xfreg16[Dest]);
  (*xfreg16[Dest])--;
  (*xfreg16[Source])--;
  W_REG--;
  CycleCounter += 3;
  PC_REG -= 2;
}

void Tfm3(void)
{ //113A 6309 TFM R+,R 6309
  if (W_REG == 0)
  {
    CycleCounter += 6;
    PC_REG++;
    return;
  }

  postbyte = MemRead8(PC_REG);
  Source = postbyte >> 4;
  Dest = postbyte & 15;

  if (Source > 4 || Dest > 4)
  {
    InvalidInsHandler();
    return;
  }

  temp8 = MemRead8(*xfreg16[Source]);
  MemWrite8(temp8, *xfreg16[Dest]);
  (*xfreg16[Source])++;
  W_REG--;
  PC_REG -= 2; //Hit the same instruction on the next loop if not done copying
  CycleCounter += 3;
}

void Tfm4(void)
{ //113B TFM R,R+ 6309 
  if (W_REG == 0)
  {
    CycleCounter += 6;
    PC_REG++;
    return;
  }

  postbyte = MemRead8(PC_REG);
  Source = postbyte >> 4;
  Dest = postbyte & 15;

  if (Source > 4 || Dest > 4)
  {
    InvalidInsHandler();
    return;
  }

  temp8 = MemRead8(*xfreg16[Source]);
  MemWrite8(temp8, *xfreg16[Dest]);
  (*xfreg16[Dest])++;
  W_REG--;
  PC_REG -= 2; //Hit the same instruction on the next loop if not done copying
  CycleCounter += 3;
}

void Bitmd_M(void)
{ //113C  6309
  postbyte = MemRead8(PC_REG++) & 0xC0;
  temp8 = getmd() & postbyte;
  cc[Z] = ZTEST(temp8);

  if (temp8 & 0x80) md[7] = 0;
  if (temp8 & 0x40) md[6] = 0;

  CycleCounter += 4;
}

void Ldmd_M(void)
{ //113D DONE 6309
  mdbits = MemRead8(PC_REG++) & 0x03;
  setmd(mdbits);
  CycleCounter += 5;
}

void Swi3_I(void)
{ //113F
  cc[E] = 1;
  MemWrite8(pc.B.lsb, --S_REG);
  MemWrite8(pc.B.msb, --S_REG);
  MemWrite8(u.B.lsb, --S_REG);
  MemWrite8(u.B.msb, --S_REG);
  MemWrite8(y.B.lsb, --S_REG);
  MemWrite8(y.B.msb, --S_REG);
  MemWrite8(x.B.lsb, --S_REG);
  MemWrite8(x.B.msb, --S_REG);
  MemWrite8(dp.B.msb, --S_REG);

  if (md[NATIVE6309])
  {
    MemWrite8((F_REG), --S_REG);
    MemWrite8((E_REG), --S_REG);
    CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(getcc(), --S_REG);
  PC_REG = MemRead16(VSWI3);
  CycleCounter += 20;
}

void Come_I(void)
{ //1143 6309 Untested
  E_REG = 0xFF - E_REG;
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[C] = 1;
  cc[V] = 0;
  CycleCounter += NatEmuCycles32;
}

void Dece_I(void)
{ //114A 6309
  E_REG--;
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = E_REG == 0x7F;
  CycleCounter += NatEmuCycles32;
}

void Ince_I(void)
{ //114C 6309
  E_REG++;
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = E_REG == 0x80;
  CycleCounter += NatEmuCycles32;
}

void Tste_I(void)
{ //114D 6309 Untested TESTED NITRO
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles32;
}

void Clre_I(void)
{ //114F 6309
  E_REG = 0;
  cc[C] = 0;
  cc[V] = 0;
  cc[N] = 0;
  cc[Z] = 1;
  CycleCounter += NatEmuCycles32;
}

void Comf_I(void)
{ //1153 6309 Untested
  F_REG = 0xFF - F_REG;
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[C] = 1;
  cc[V] = 0;
  CycleCounter += NatEmuCycles32;
}

void Decf_I(void)
{ //115A 6309
  F_REG--;
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = F_REG == 0x7F;
  CycleCounter += NatEmuCycles21;
}

void Incf_I(void)
{ //115C 6309 Untested
  F_REG++;
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = F_REG == 0x80;
  CycleCounter += NatEmuCycles32;
}

void Tstf_I(void)
{ //115D 6309
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles32;
}

void Clrf_I(void)
{ //115F 6309 Untested wcreate
  F_REG = 0;
  cc[C] = 0;
  cc[V] = 0;
  cc[N] = 0;
  cc[Z] = 1;
  CycleCounter += NatEmuCycles32;
}

void Sube_M(void)
{ //1180 6309 Untested
  postbyte = MemRead8(PC_REG++);
  temp16 = E_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  CycleCounter += 3;
}

void Cmpe_M(void)
{ //1181 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = E_REG - postbyte;
  cc[C] = temp8 > E_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, E_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += 3;
}

void Cmpu_M(void)
{ //1183
  postword = IMMADDRESS(PC_REG);
  temp16 = U_REG - postword;
  cc[C] = temp16 > U_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, U_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Lde_M(void)
{ //1186 6309
  E_REG = MemRead8(PC_REG++);
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = 0;
  CycleCounter += 3;
}

void Adde_M(void)
{ //118B 6309
  postbyte = MemRead8(PC_REG++);
  temp16 = E_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((E_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  cc[N] = NTEST8(E_REG);
  cc[Z] = ZTEST(E_REG);
  CycleCounter += 3;
}

void Cmps_M(void)
{ //118C
  postword = IMMADDRESS(PC_REG);
  temp16 = S_REG - postword;
  cc[C] = temp16 > S_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, S_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Divd_M(void)
{ //118D 6309
  postbyte = MemRead8(PC_REG++);

  if (postbyte == 0)
  {
    CycleCounter += 3;
    DivbyZero();
    return;
  }

  postword = D_REG;
  stemp16 = (signed short)postword / (signed char)postbyte;

  if ((stemp16 > 255) || (stemp16 < -256)) //Abort
  {
    cc[V] = 1;
    cc[N] = 0;
    cc[Z] = 0;
    cc[C] = 0;
    CycleCounter += 17;
    return;
  }

  A_REG = (unsigned char)((signed short)postword % (signed char)postbyte);
  B_REG = (unsigned char)stemp16;

  if ((stemp16 > 127) || (stemp16 < -128))
  {
    cc[V] = 1;
    cc[N] = 1;
  }
  else
  {
    cc[Z] = ZTEST(B_REG);
    cc[N] = NTEST8(B_REG);
    cc[V] = 0;
  }

  cc[C] = B_REG & 1;
  CycleCounter += 25;
}

void Divq_M(void)
{ //118E 6309
  postword = MemRead16(PC_REG);
  PC_REG += 2;

  if (postword == 0)
  {
    CycleCounter += 4;
    DivbyZero();
    return;
  }

  temp32 = Q_REG;
  stemp32 = (signed int)temp32 / (signed short int)postword;

  if ((stemp32 > 65535) || (stemp32 < -65536)) //Abort
  {
    cc[V] = 1;
    cc[N] = 0;
    cc[Z] = 0;
    cc[C] = 0;
    CycleCounter += 34 - 21;
    return;
  }

  D_REG = (unsigned short)((signed int)temp32 % (signed short int)postword);
  W_REG = stemp32;

  if ((stemp16 > 32767) || (stemp16 < -32768))
  {
    cc[V] = 1;
    cc[N] = 1;
  }
  else
  {
    cc[Z] = ZTEST(W_REG);
    cc[N] = NTEST16(W_REG);
    cc[V] = 0;
  }

  cc[C] = B_REG & 1;
  CycleCounter += 34;
}

void Muld_M(void)
{ //118F Phase 5 6309
  Q_REG = (signed short)D_REG * (signed short)IMMADDRESS(PC_REG);
  cc[C] = 0;
  cc[Z] = ZTEST(Q_REG);
  cc[V] = 0;
  cc[N] = NTEST32(Q_REG);
  PC_REG += 2;
  CycleCounter += 28;
}

void Sube_D(void)
{ //1190 6309 Untested HERE
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = E_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  CycleCounter += NatEmuCycles54;
}

void Cmpe_D(void)
{ //1191 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp8 = E_REG - postbyte;
  cc[C] = temp8 > E_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, E_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += NatEmuCycles54;
}

void Cmpu_D(void)
{ //1193
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = U_REG - postword;
  cc[C] = temp16 > U_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, U_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles75;
}

void Lde_D(void)
{ //1196 6309
  E_REG = MemRead8(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Ste_D(void)
{ //1197 Phase 5 6309
  MemWrite8(E_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Adde_D(void)
{ //119B 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = E_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((E_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  cc[N] = NTEST8(E_REG);
  cc[Z] = ZTEST(E_REG);
  CycleCounter += NatEmuCycles54;
}

void Cmps_D(void)
{ //119C
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = S_REG - postword;
  cc[C] = temp16 > S_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, S_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles75;
}

void Divd_D(void)
{ //119D 6309 02292008
  postbyte = MemRead8(DPADDRESS(PC_REG++));

  if (postbyte == 0)
  {
    CycleCounter += 3;
    DivbyZero();
    return;
  }

  postword = D_REG;
  stemp16 = (signed short)postword / (signed char)postbyte;

  if ((stemp16 > 255) || (stemp16 < -256)) //Abort
  {
    cc[V] = 1;
    cc[N] = 0;
    cc[Z] = 0;
    cc[C] = 0;
    CycleCounter += 19;
    return;
  }

  A_REG = (unsigned char)((signed short)postword % (signed char)postbyte);
  B_REG = (unsigned char)stemp16;

  if ((stemp16 > 127) || (stemp16 < -128))
  {
    cc[V] = 1;
    cc[N] = 1;
  }
  else
  {
    cc[Z] = ZTEST(B_REG);
    cc[N] = NTEST8(B_REG);
    cc[V] = 0;
  }

  cc[C] = B_REG & 1;
  CycleCounter += 27;
}

void Divq_D(void)
{ //119E 6309
  postword = MemRead16(DPADDRESS(PC_REG++));

  if (postword == 0)
  {
    CycleCounter += 4;
    DivbyZero();
    return;
  }

  temp32 = Q_REG;
  stemp32 = (signed int)temp32 / (signed short int)postword;

  if ((stemp32 > 65535) || (stemp32 < -65536)) //Abort
  {
    cc[V] = 1;
    cc[N] = 0;
    cc[Z] = 0;
    cc[C] = 0;
    CycleCounter += NatEmuCycles3635 - 21;
    return;
  }

  D_REG = (unsigned short)((signed int)temp32 % (signed short int)postword);
  W_REG = stemp32;

  if ((stemp16 > 32767) || (stemp32 < -32768))
  {
    cc[V] = 1;
    cc[N] = 1;
  }
  else
  {
    cc[Z] = ZTEST(W_REG);
    cc[N] = NTEST16(W_REG);
    cc[V] = 0;
  }

  cc[C] = B_REG & 1;
  CycleCounter += NatEmuCycles3635;
}

void Muld_D(void)
{ //119F 6309 02292008
  Q_REG = (signed short)D_REG * (signed short)MemRead16(DPADDRESS(PC_REG++));
  cc[C] = 0;
  cc[Z] = ZTEST(Q_REG);
  cc[V] = 0;
  cc[N] = NTEST32(Q_REG);
  CycleCounter += NatEmuCycles3029;
}

void Sube_X(void)
{ //11A0 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = E_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  CycleCounter += 5;
}

void Cmpe_X(void)
{ //11A1 6309
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp8 = E_REG - postbyte;
  cc[C] = temp8 > E_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, E_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += 5;
}

void Cmpu_X(void)
{ //11A3
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = U_REG - postword;
  cc[C] = temp16 > U_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, U_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles76;
}

void Lde_X(void)
{ //11A6 6309
  E_REG = MemRead8(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Ste_X(void)
{ //11A7 6309
  MemWrite8(E_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Adde_X(void)
{ //11AB 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = E_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((E_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  cc[N] = NTEST8(E_REG);
  cc[Z] = ZTEST(E_REG);
  CycleCounter += 5;
}

void Cmps_X(void)
{  //11AC
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = S_REG - postword;
  cc[C] = temp16 > S_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, S_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles76;
}

void Divd_X(void)
{ //11AD wcreate  6309
  postbyte = MemRead8(INDADDRESS(PC_REG++));

  if (postbyte == 0)
  {
    CycleCounter += 3;
    DivbyZero();
    return;
  }

  postword = D_REG;
  stemp16 = (signed short)postword / (signed char)postbyte;

  if ((stemp16 > 255) || (stemp16 < -256)) //Abort
  {
    cc[V] = 1;
    cc[N] = 0;
    cc[Z] = 0;
    cc[C] = 0;
    CycleCounter += 19;
    return;
  }

  A_REG = (unsigned char)((signed short)postword % (signed char)postbyte);
  B_REG = (unsigned char)stemp16;

  if ((stemp16 > 127) || (stemp16 < -128))
  {
    cc[V] = 1;
    cc[N] = 1;
  }
  else
  {
    cc[Z] = ZTEST(B_REG);
    cc[N] = NTEST8(B_REG);
    cc[V] = 0;
  }
  cc[C] = B_REG & 1;
  CycleCounter += 27;
}

void Divq_X(void)
{ //11AE Phase 5 6309 CHECK
  postword = MemRead16(INDADDRESS(PC_REG++));

  if (postword == 0)
  {
    CycleCounter += 4;
    DivbyZero();
    return;
  }

  temp32 = Q_REG;
  stemp32 = (signed int)temp32 / (signed short int)postword;

  if ((stemp32 > 65535) || (stemp32 < -65536)) //Abort
  {
    cc[V] = 1;
    cc[N] = 0;
    cc[Z] = 0;
    cc[C] = 0;
    CycleCounter += NatEmuCycles3635 - 21;
    return;
  }

  D_REG = (unsigned short)((signed int)temp32 % (signed short int)postword);
  W_REG = stemp32;

  if ((stemp16 > 32767) || (stemp16 < -32768))
  {
    cc[V] = 1;
    cc[N] = 1;
  }
  else
  {
    cc[Z] = ZTEST(W_REG);
    cc[N] = NTEST16(W_REG);
    cc[V] = 0;
  }

  cc[C] = B_REG & 1;
  CycleCounter += NatEmuCycles3635;
}

void Muld_X(void)
{ //11AF 6309 CHECK
  Q_REG = (signed short)D_REG * (signed short)MemRead16(INDADDRESS(PC_REG++));
  cc[C] = 0;
  cc[Z] = ZTEST(Q_REG);
  cc[V] = 0;
  cc[N] = NTEST32(Q_REG);
  CycleCounter += 30;
}

void Sube_E(void)
{ //11B0 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = E_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Cmpe_E(void)
{ //11B1 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp8 = E_REG - postbyte;
  cc[C] = temp8 > E_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, E_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Cmpu_E(void)
{ //11B3
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = U_REG - postword;
  cc[C] = temp16 > U_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, U_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Lde_E(void)
{ //11B6 6309
  E_REG = MemRead8(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Ste_E(void)
{ //11B7 6309
  MemWrite8(E_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(E_REG);
  cc[N] = NTEST8(E_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Adde_E(void)
{ //11BB 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = E_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((E_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  cc[N] = NTEST8(E_REG);
  cc[Z] = ZTEST(E_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Cmps_E(void)
{ //11BC
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = S_REG - postword;
  cc[C] = temp16 > S_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, S_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles86;
}

void Divd_E(void)
{ //11BD 6309 02292008 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  PC_REG += 2;

  if (postbyte == 0)
  {
    CycleCounter += 3;
    DivbyZero();
    return;
  }

  postword = D_REG;
  stemp16 = (signed short)postword / (signed char)postbyte;

  if ((stemp16 > 255) || (stemp16 < -256)) //Abort
  {
    cc[V] = 1;
    cc[N] = 0;
    cc[Z] = 0;
    cc[C] = 0;
    CycleCounter += 17;
    return;
  }

  A_REG = (unsigned char)((signed short)postword % (signed char)postbyte);
  B_REG = (unsigned char)stemp16;

  if ((stemp16 > 127) || (stemp16 < -128))
  {
    cc[V] = 1;
    cc[N] = 1;
  }
  else
  {
    cc[Z] = ZTEST(B_REG);
    cc[N] = NTEST8(B_REG);
    cc[V] = 0;
  }
  cc[C] = B_REG & 1;
  CycleCounter += 25;
}

void Divq_E(void)
{ //11BE Phase 5 6309 CHECK
  postword = MemRead16(IMMADDRESS(PC_REG));
  PC_REG += 2;

  if (postword == 0)
  {
    CycleCounter += 4;
    DivbyZero();
    return;
  }

  temp32 = Q_REG;
  stemp32 = (signed int)temp32 / (signed short int)postword;

  if ((stemp32 > 65535) || (stemp32 < -65536)) //Abort
  {
    cc[V] = 1;
    cc[N] = 0;
    cc[Z] = 0;
    cc[C] = 0;
    CycleCounter += NatEmuCycles3635 - 21;
    return;
  }

  D_REG = (unsigned short)((signed int)temp32 % (signed short int)postword);
  W_REG = stemp32;

  if ((stemp16 > 32767) || (stemp16 < -32768))
  {
    cc[V] = 1;
    cc[N] = 1;
  }
  else
  {
    cc[Z] = ZTEST(W_REG);
    cc[N] = NTEST16(W_REG);
    cc[V] = 0;
  }
  cc[C] = B_REG & 1;
  CycleCounter += NatEmuCycles3635;
}

void Muld_E(void)
{ //11BF 6309
  Q_REG = (signed short)D_REG * (signed short)MemRead16(IMMADDRESS(PC_REG));
  PC_REG += 2;
  cc[C] = 0;
  cc[Z] = ZTEST(Q_REG);
  cc[V] = 0;
  cc[N] = NTEST32(Q_REG);
  CycleCounter += NatEmuCycles3130;
}

void Subf_M(void)
{ //11C0 6309 Untested
  postbyte = MemRead8(PC_REG++);
  temp16 = F_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  CycleCounter += 3;
}

void Cmpf_M(void)
{ //11C1 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = F_REG - postbyte;
  cc[C] = temp8 > F_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, F_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += 3;
}

void Ldf_M(void)
{ //11C6 6309
  F_REG = MemRead8(PC_REG++);
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = 0;
  CycleCounter += 3;
}

void Addf_M(void)
{ //11CB 6309 Untested
  postbyte = MemRead8(PC_REG++);
  temp16 = F_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((F_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  cc[N] = NTEST8(F_REG);
  cc[Z] = ZTEST(F_REG);
  CycleCounter += 3;
}

void Subf_D(void)
{ //11D0 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = F_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  CycleCounter += NatEmuCycles54;
}

void Cmpf_D(void)
{ //11D1 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp8 = F_REG - postbyte;
  cc[C] = temp8 > F_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, F_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += NatEmuCycles54;
}

void Ldf_D(void)
{ //11D6 6309 Untested wcreate
  F_REG = MemRead8(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Stf_D(void)
{ //11D7 Phase 5 6309
  MemWrite8(F_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Addf_D(void)
{ //11DB 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = F_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((F_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  cc[N] = NTEST8(F_REG);
  cc[Z] = ZTEST(F_REG);
  CycleCounter += NatEmuCycles54;
}

void Subf_X(void)
{ //11E0 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = F_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  CycleCounter += 5;
}

void Cmpf_X(void)
{ //11E1 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp8 = F_REG - postbyte;
  cc[C] = temp8 > F_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, F_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += 5;
}

void Ldf_X(void)
{ //11E6 6309
  F_REG = MemRead8(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Stf_X(void)
{ //11E7 Phase 5 6309
  MemWrite8(F_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Addf_X(void)
{ //11EB 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = F_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((F_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  cc[N] = NTEST8(F_REG);
  cc[Z] = ZTEST(F_REG);
  CycleCounter += 5;
}

void Subf_E(void)
{ //11F0 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = F_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Cmpf_E(void)
{ //11F1 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp8 = F_REG - postbyte;
  cc[C] = temp8 > F_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, F_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Ldf_E(void)
{ //11F6 6309
  F_REG = MemRead8(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Stf_E(void)
{ //11F7 Phase 5 6309
  MemWrite8(F_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(F_REG);
  cc[N] = NTEST8(F_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Addf_E(void)
{ //11FB 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = F_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((F_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  cc[N] = NTEST8(F_REG);
  cc[Z] = ZTEST(F_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Nop_I(void)
{	//12
  CycleCounter += NatEmuCycles21;
}

void Sync_I(void)
{ //13
  CycleCounter = gCycleFor;
  SyncWaiting = 1;
}

void Sexw_I(void)
{ //14 6309 CHECK
  if (W_REG & 32768)
    D_REG = 0xFFFF;
  else
    D_REG = 0;

  cc[Z] = ZTEST(Q_REG);
  cc[N] = NTEST16(D_REG);
  CycleCounter += 4;
}

void Lbra_R(void)
{ //16
  *spostword = IMMADDRESS(PC_REG);
  PC_REG += 2;
  PC_REG += *spostword;
  CycleCounter += NatEmuCycles54;
}

void Lbsr_R(void)
{ //17
  *spostword = IMMADDRESS(PC_REG);
  PC_REG += 2;
  S_REG--;
  MemWrite8(pc.B.lsb, S_REG--);
  MemWrite8(pc.B.msb, S_REG);
  PC_REG += *spostword;
  CycleCounter += NatEmuCycles97;
}

void Daa_I(void)
{ //19
  static unsigned char msn, lsn;

  msn = (A_REG & 0xF0);
  lsn = (A_REG & 0xF);
  temp8 = 0;

  if (cc[H] || (lsn > 9))
    temp8 |= 0x06;

  if ((msn > 0x80) && (lsn > 9))
    temp8 |= 0x60;

  if ((msn > 0x90) || cc[C])
    temp8 |= 0x60;

  temp16 = A_REG + temp8;
  cc[C] |= ((temp16 & 0x100) >> 8);
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += NatEmuCycles21;
}

void Orcc_M(void)
{ //1A
  postbyte = MemRead8(PC_REG++);
  temp8 = getcc();
  temp8 = (temp8 | postbyte);
  setcc(temp8);
  CycleCounter += NatEmuCycles32;
}

void Andcc_M(void)
{ //1C
  postbyte = MemRead8(PC_REG++);
  temp8 = getcc();
  temp8 = (temp8 & postbyte);
  setcc(temp8);
  CycleCounter += 3;
}

void Sex_I(void)
{ //1D
  A_REG = 0 - (B_REG >> 7);
  cc[Z] = ZTEST(D_REG);
  cc[N] = D_REG >> 15;
  CycleCounter += NatEmuCycles21;
}

void Exg_M(void)
{ //1E
  postbyte = MemRead8(PC_REG++);
  Source = postbyte >> 4;
  Dest = postbyte & 15;

  ccbits = getcc();

  if ((Source & 0x08) == (Dest & 0x08)) //Verify like size registers
  {
    if (Dest & 0x08) //8 bit EXG
    {
      Source &= 0x07;
      Dest &= 0x07;
      temp8 = (*ureg8[Source]);
      *ureg8[Source] = (*ureg8[Dest]);
      *ureg8[Dest] = temp8;
      O_REG = 0;
    }
    else // 16 bit EXG
    {
      Source &= 0x07;
      Dest &= 0x07;
      temp16 = (*xfreg16[Source]);
      (*xfreg16[Source]) = (*xfreg16[Dest]);
      (*xfreg16[Dest]) = temp16;
    }
  }
  else
  {
    if (Dest & 0x08) // Swap 16 to 8 bit exchange to be 8 to 16 bit exchange (for convenience)
    {
      temp8 = Dest; Dest = Source; Source = temp8;
    }

    Source &= 0x07;
    Dest &= 0x07;

    switch (Source)
    {
    case 0x04: // Z
    case 0x05: // Z
      (*xfreg16[Dest]) = 0; // Source is Zero reg. Just zero the Destination.
      break;

    case 0x00: // A
    case 0x03: // DP
    case 0x06: // E
      temp8 = *ureg8[Source];
      temp16 = (temp8 << 8) | temp8;
      (*ureg8[Source]) = (*xfreg16[Dest]) >> 8; // A, DP, E get high byte of 16 bit Dest
      (*xfreg16[Dest]) = temp16; // Place 8 bit source in both halves of 16 bit Dest
      break;

    case 0x01: // B
    case 0x02: // CC
    case 0x07: // F
      temp8 = *ureg8[Source];
      temp16 = (temp8 << 8) | temp8;
      (*ureg8[Source]) = (*xfreg16[Dest]) & 0xFF; // B, CC, F get low byte of 16 bit Dest
      (*xfreg16[Dest]) = temp16; // Place 8 bit source in both halves of 16 bit Dest
      break;
    }
  }

  setcc(ccbits);
  CycleCounter += NatEmuCycles85;
}

void Tfr_M(void)
{ //1F
  postbyte = MemRead8(PC_REG++);
  Source = postbyte >> 4;
  Dest = postbyte & 15;

  if (Dest < 8)
  {
    if (Source < 8)
      *xfreg16[Dest] = *xfreg16[Source];
    else
      *xfreg16[Dest] = (*ureg8[Source & 7] << 8) | *ureg8[Source & 7];
  }
  else
  {
    ccbits = getcc();
    Dest &= 7;

    if (Source < 8)
      switch (Dest)
      {
      case 0:  // A
      case 3: // DP
      case 6: // E
        *ureg8[Dest] = *xfreg16[Source] >> 8;
        break;
      case 1:  // B
      case 2: // CC
      case 7: // F
        *ureg8[Dest] = *xfreg16[Source] & 0xFF;
        break;
      }
    else
      *ureg8[Dest] = *ureg8[Source & 7];

    O_REG = 0;
    setcc(ccbits);
  }

  CycleCounter += NatEmuCycles64;
}

void Bra_R(void)
{ //20
  *spostbyte = MemRead8(PC_REG++);
  PC_REG += *spostbyte;
  CycleCounter += 3;
}

void Brn_R(void)
{ //21
  CycleCounter += 3;
  PC_REG++;
}

void Bhi_R(void)
{ //22
  if (!(cc[C] | cc[Z]))
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Bls_R(void)
{ //23
  if (cc[C] | cc[Z])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Bhs_R(void)
{ //24
  if (!cc[C])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Blo_R(void)
{ //25
  if (cc[C])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Bne_R(void)
{ //26
  if (!cc[Z])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Beq_R(void)
{ //27
  if (cc[Z])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Bvc_R(void)
{ //28
  if (!cc[V])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Bvs_R(void)
{ //29
  if (cc[V])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Bpl_R(void)
{ //2A
  if (!cc[N])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Bmi_R(void)
{ //2B
  if (cc[N])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Bge_R(void)
{ //2C
  if (!(cc[N] ^ cc[V]))
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Blt_R(void)
{ //2D
  if (cc[V] ^ cc[N])
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Bgt_R(void)
{ //2E
  if (!(cc[Z] | (cc[N] ^ cc[V])))
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Ble_R(void)
{ //2F
  if (cc[Z] | (cc[N] ^ cc[V]))
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  CycleCounter += 3;
}

void Leax_X(void)
{ //30
  X_REG = INDADDRESS(PC_REG++);
  cc[Z] = ZTEST(X_REG);
  CycleCounter += 4;
}

void Leay_X(void)
{ //31
  Y_REG = INDADDRESS(PC_REG++);
  cc[Z] = ZTEST(Y_REG);
  CycleCounter += 4;
}

void Leas_X(void)
{ //32
  S_REG = INDADDRESS(PC_REG++);
  CycleCounter += 4;
}

void Leau_X(void)
{ //33
  U_REG = INDADDRESS(PC_REG++);
  CycleCounter += 4;
}

void Pshs_M(void)
{ //34
  postbyte = MemRead8(PC_REG++);

  if (postbyte & 0x80)
  {
    MemWrite8(pc.B.lsb, --S_REG);
    MemWrite8(pc.B.msb, --S_REG);
    CycleCounter += 2;
  }

  if (postbyte & 0x40)
  {
    MemWrite8(u.B.lsb, --S_REG);
    MemWrite8(u.B.msb, --S_REG);
    CycleCounter += 2;
  }

  if (postbyte & 0x20)
  {
    MemWrite8(y.B.lsb, --S_REG);
    MemWrite8(y.B.msb, --S_REG);
    CycleCounter += 2;
  }

  if (postbyte & 0x10)
  {
    MemWrite8(x.B.lsb, --S_REG);
    MemWrite8(x.B.msb, --S_REG);
    CycleCounter += 2;
  }

  if (postbyte & 0x08)
  {
    MemWrite8(dp.B.msb, --S_REG);
    CycleCounter += 1;
  }

  if (postbyte & 0x04)
  {
    MemWrite8(B_REG, --S_REG);
    CycleCounter += 1;
  }

  if (postbyte & 0x02)
  {
    MemWrite8(A_REG, --S_REG);
    CycleCounter += 1;
  }

  if (postbyte & 0x01)
  {
    MemWrite8(getcc(), --S_REG);
    CycleCounter += 1;
  }

  CycleCounter += NatEmuCycles54;
}

void Puls_M(void)
{ //35
  postbyte = MemRead8(PC_REG++);

  if (postbyte & 0x01)
  {
    setcc(MemRead8(S_REG++));
    CycleCounter += 1;
  }

  if (postbyte & 0x02)
  {
    A_REG = MemRead8(S_REG++);
    CycleCounter += 1;
  }

  if (postbyte & 0x04)
  {
    B_REG = MemRead8(S_REG++);
    CycleCounter += 1;
  }

  if (postbyte & 0x08)
  {
    dp.B.msb = MemRead8(S_REG++);
    CycleCounter += 1;
  }

  if (postbyte & 0x10)
  {
    x.B.msb = MemRead8(S_REG++);
    x.B.lsb = MemRead8(S_REG++);
    CycleCounter += 2;
  }

  if (postbyte & 0x20)
  {
    y.B.msb = MemRead8(S_REG++);
    y.B.lsb = MemRead8(S_REG++);
    CycleCounter += 2;
  }

  if (postbyte & 0x40)
  {
    u.B.msb = MemRead8(S_REG++);
    u.B.lsb = MemRead8(S_REG++);
    CycleCounter += 2;
  }

  if (postbyte & 0x80)
  {
    pc.B.msb = MemRead8(S_REG++);
    pc.B.lsb = MemRead8(S_REG++);
    CycleCounter += 2;
  }

  CycleCounter += NatEmuCycles54;
}

void Pshu_M(void)
{ //36
  postbyte = MemRead8(PC_REG++);

  if (postbyte & 0x80)
  {
    MemWrite8(pc.B.lsb, --U_REG);
    MemWrite8(pc.B.msb, --U_REG);
    CycleCounter += 2;
  }

  if (postbyte & 0x40)
  {
    MemWrite8(s.B.lsb, --U_REG);
    MemWrite8(s.B.msb, --U_REG);
    CycleCounter += 2;
  }

  if (postbyte & 0x20)
  {
    MemWrite8(y.B.lsb, --U_REG);
    MemWrite8(y.B.msb, --U_REG);
    CycleCounter += 2;
  }

  if (postbyte & 0x10)
  {
    MemWrite8(x.B.lsb, --U_REG);
    MemWrite8(x.B.msb, --U_REG);
    CycleCounter += 2;
  }

  if (postbyte & 0x08)
  {
    MemWrite8(dp.B.msb, --U_REG);
    CycleCounter += 1;
  }

  if (postbyte & 0x04)
  {
    MemWrite8(B_REG, --U_REG);
    CycleCounter += 1;
  }

  if (postbyte & 0x02)
  {
    MemWrite8(A_REG, --U_REG);
    CycleCounter += 1;
  }

  if (postbyte & 0x01)
  {
    MemWrite8(getcc(), --U_REG);
    CycleCounter += 1;
  }

  CycleCounter += NatEmuCycles54;
}

void Pulu_M(void)
{ //37
  postbyte = MemRead8(PC_REG++);

  if (postbyte & 0x01)
  {
    setcc(MemRead8(U_REG++));
    CycleCounter += 1;
  }

  if (postbyte & 0x02)
  {
    A_REG = MemRead8(U_REG++);
    CycleCounter += 1;
  }

  if (postbyte & 0x04)
  {
    B_REG = MemRead8(U_REG++);
    CycleCounter += 1;
  }

  if (postbyte & 0x08)
  {
    dp.B.msb = MemRead8(U_REG++);
    CycleCounter += 1;
  }

  if (postbyte & 0x10)
  {
    x.B.msb = MemRead8(U_REG++);
    x.B.lsb = MemRead8(U_REG++);
    CycleCounter += 2;
  }

  if (postbyte & 0x20)
  {
    y.B.msb = MemRead8(U_REG++);
    y.B.lsb = MemRead8(U_REG++);
    CycleCounter += 2;
  }

  if (postbyte & 0x40)
  {
    s.B.msb = MemRead8(U_REG++);
    s.B.lsb = MemRead8(U_REG++);
    CycleCounter += 2;
  }

  if (postbyte & 0x80)
  {
    pc.B.msb = MemRead8(U_REG++);
    pc.B.lsb = MemRead8(U_REG++);
    CycleCounter += 2;
  }

  CycleCounter += NatEmuCycles54;
}

void Rts_I(void)
{ //39
  pc.B.msb = MemRead8(S_REG++);
  pc.B.lsb = MemRead8(S_REG++);
  CycleCounter += NatEmuCycles51;
}

void Abx_I(void)
{ //3A
  X_REG = X_REG + B_REG;
  CycleCounter += NatEmuCycles31;
}

void Rti_I(void)
{ //3B
  setcc(MemRead8(S_REG++));
  CycleCounter += 6;
  InInterrupt = 0;

  if (cc[E])
  {
    A_REG = MemRead8(S_REG++);
    B_REG = MemRead8(S_REG++);

    if (md[NATIVE6309])
    {
      (E_REG) = MemRead8(S_REG++);
      (F_REG) = MemRead8(S_REG++);
      CycleCounter += 2;
    }

    dp.B.msb = MemRead8(S_REG++);
    x.B.msb = MemRead8(S_REG++);
    x.B.lsb = MemRead8(S_REG++);
    y.B.msb = MemRead8(S_REG++);
    y.B.lsb = MemRead8(S_REG++);
    u.B.msb = MemRead8(S_REG++);
    u.B.lsb = MemRead8(S_REG++);
    CycleCounter += 9;
  }

  pc.B.msb = MemRead8(S_REG++);
  pc.B.lsb = MemRead8(S_REG++);
}

void Cwai_I(void)
{ //3C
  postbyte = MemRead8(PC_REG++);
  ccbits = getcc();
  ccbits = ccbits & postbyte;
  setcc(ccbits);
  CycleCounter = gCycleFor;
  SyncWaiting = 1;
}

void Mul_I(void)
{ //3D
  D_REG = A_REG * B_REG;
  cc[C] = B_REG > 0x7F;
  cc[Z] = ZTEST(D_REG);
  CycleCounter += NatEmuCycles1110;
}

void Reset(void) // 3E
{	//Undocumented
  HD6309Reset();
}

void Swi1_I(void)
{ //3F
  cc[E] = 1;
  MemWrite8(pc.B.lsb, --S_REG);
  MemWrite8(pc.B.msb, --S_REG);
  MemWrite8(u.B.lsb, --S_REG);
  MemWrite8(u.B.msb, --S_REG);
  MemWrite8(y.B.lsb, --S_REG);
  MemWrite8(y.B.msb, --S_REG);
  MemWrite8(x.B.lsb, --S_REG);
  MemWrite8(x.B.msb, --S_REG);
  MemWrite8(dp.B.msb, --S_REG);

  if (md[NATIVE6309])
  {
    MemWrite8((F_REG), --S_REG);
    MemWrite8((E_REG), --S_REG);
    CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(getcc(), --S_REG);
  PC_REG = MemRead16(VSWI);
  CycleCounter += 19;
  cc[I] = 1;
  cc[F] = 1;
}

void Nega_I(void)
{ //40
  temp8 = 0 - A_REG;
  cc[C] = temp8 > 0;
  cc[V] = A_REG == 0x80; //cc[C] ^ ((A_REG^temp8)>>7);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  A_REG = temp8;
  CycleCounter += NatEmuCycles21;
}

void Coma_I(void)
{ //43
  A_REG = 0xFF - A_REG;
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  cc[C] = 1;
  cc[V] = 0;
  CycleCounter += NatEmuCycles21;
}

void Lsra_I(void)
{ //44
  cc[C] = A_REG & 1;
  A_REG = A_REG >> 1;
  cc[Z] = ZTEST(A_REG);
  cc[N] = 0;
  CycleCounter += NatEmuCycles21;
}

void Rora_I(void)
{ //46
  postbyte = cc[C] << 7;
  cc[C] = A_REG & 1;
  A_REG = (A_REG >> 1) | postbyte;
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  CycleCounter += NatEmuCycles21;
}

void Asra_I(void)
{ //47
  cc[C] = A_REG & 1;
  A_REG = (A_REG & 0x80) | (A_REG >> 1);
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  CycleCounter += NatEmuCycles21;
}

void Asla_I(void)
{ //48
  cc[C] = A_REG > 0x7F;
  cc[V] = cc[C] ^ ((A_REG & 0x40) >> 6);
  A_REG = A_REG << 1;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += NatEmuCycles21;
}

void Rola_I(void)
{ //49
  postbyte = cc[C];
  cc[C] = A_REG > 0x7F;
  cc[V] = cc[C] ^ ((A_REG & 0x40) >> 6);
  A_REG = (A_REG << 1) | postbyte;
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  CycleCounter += NatEmuCycles21;
}

void Deca_I(void)
{ //4A
  A_REG--;
  cc[Z] = ZTEST(A_REG);
  cc[V] = A_REG == 0x7F;
  cc[N] = NTEST8(A_REG);
  CycleCounter += NatEmuCycles21;
}

void Inca_I(void)
{ //4C
  A_REG++;
  cc[Z] = ZTEST(A_REG);
  cc[V] = A_REG == 0x80;
  cc[N] = NTEST8(A_REG);
  CycleCounter += NatEmuCycles21;
}

void Tsta_I(void)
{ //4D
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles21;
}

void Clra_I(void)
{ //4F
  A_REG = 0;
  cc[C] = 0;
  cc[V] = 0;
  cc[N] = 0;
  cc[Z] = 1;
  CycleCounter += NatEmuCycles21;
}

void Negb_I(void)
{ //50
  temp8 = 0 - B_REG;
  cc[C] = temp8 > 0;
  cc[V] = B_REG == 0x80;
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  B_REG = temp8;
  CycleCounter += NatEmuCycles21;
}

void Comb_I(void)
{ //53
  B_REG = 0xFF - B_REG;
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  cc[C] = 1;
  cc[V] = 0;
  CycleCounter += NatEmuCycles21;
}

void Lsrb_I(void)
{ //54
  cc[C] = B_REG & 1;
  B_REG = B_REG >> 1;
  cc[Z] = ZTEST(B_REG);
  cc[N] = 0;
  CycleCounter += NatEmuCycles21;
}

void Rorb_I(void)
{ //56
  postbyte = cc[C] << 7;
  cc[C] = B_REG & 1;
  B_REG = (B_REG >> 1) | postbyte;
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  CycleCounter += NatEmuCycles21;
}

void Asrb_I(void)
{ //57
  cc[C] = B_REG & 1;
  B_REG = (B_REG & 0x80) | (B_REG >> 1);
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  CycleCounter += NatEmuCycles21;
}

void Aslb_I(void)
{ //58
  cc[C] = B_REG > 0x7F;
  cc[V] = cc[C] ^ ((B_REG & 0x40) >> 6);
  B_REG = B_REG << 1;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += NatEmuCycles21;
}

void Rolb_I(void)
{ //59
  postbyte = cc[C];
  cc[C] = B_REG > 0x7F;
  cc[V] = cc[C] ^ ((B_REG & 0x40) >> 6);
  B_REG = (B_REG << 1) | postbyte;
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  CycleCounter += NatEmuCycles21;
}

void Decb_I(void)
{ //5A
  B_REG--;
  cc[Z] = ZTEST(B_REG);
  cc[V] = B_REG == 0x7F;
  cc[N] = NTEST8(B_REG);
  CycleCounter += NatEmuCycles21;
}

void Incb_I(void)
{ //5C
  B_REG++;
  cc[Z] = ZTEST(B_REG);
  cc[V] = B_REG == 0x80;
  cc[N] = NTEST8(B_REG);
  CycleCounter += NatEmuCycles21;
}

void Tstb_I(void)
{ //5D
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles21;
}

void Clrb_I(void)
{ //5F
  B_REG = 0;
  cc[C] = 0;
  cc[N] = 0;
  cc[V] = 0;
  cc[Z] = 1;
  CycleCounter += NatEmuCycles21;
}

void Neg_X(void)
{ //60
  temp16 = INDADDRESS(PC_REG++);
  postbyte = MemRead8(temp16);
  temp8 = 0 - postbyte;
  cc[C] = temp8 > 0;
  cc[V] = postbyte == 0x80;
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += 6;
}

void Oim_X(void)
{ //61 6309 DONE
  postbyte = MemRead8(PC_REG++);
  temp16 = INDADDRESS(PC_REG++);
  postbyte |= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  CycleCounter += 6;
}

void Aim_X(void)
{ //62 6309 Phase 2
  postbyte = MemRead8(PC_REG++);
  temp16 = INDADDRESS(PC_REG++);
  postbyte &= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  CycleCounter += 6;
}

void Com_X(void)
{ //63
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  temp8 = 0xFF - temp8;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  cc[V] = 0;
  cc[C] = 1;
  MemWrite8(temp8, temp16);
  CycleCounter += 6;
}

void Lsr_X(void)
{ //64
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  cc[C] = temp8 & 1;
  temp8 = temp8 >> 1;
  cc[Z] = ZTEST(temp8);
  cc[N] = 0;
  MemWrite8(temp8, temp16);
  CycleCounter += 6;
}

void Eim_X(void)
{ //65 6309 Untested TESTED NITRO
  postbyte = MemRead8(PC_REG++);
  temp16 = INDADDRESS(PC_REG++);
  postbyte ^= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  CycleCounter += 7;
}

void Ror_X(void)
{ //66
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  postbyte = cc[C] << 7;
  cc[C] = (temp8 & 1);
  temp8 = (temp8 >> 1) | postbyte;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += 6;
}

void Asr_X(void)
{ //67
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  cc[C] = temp8 & 1;
  temp8 = (temp8 & 0x80) | (temp8 >> 1);
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += 6;
}

void Asl_X(void)
{ //68 
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  cc[C] = temp8 > 0x7F;
  cc[V] = cc[C] ^ ((temp8 & 0x40) >> 6);
  temp8 = temp8 << 1;
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += 6;
}

void Rol_X(void)
{ //69
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  postbyte = cc[C];
  cc[C] = temp8 > 0x7F;
  cc[V] = (cc[C] ^ ((temp8 & 0x40) >> 6));
  temp8 = ((temp8 << 1) | postbyte);
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += 6;
}

void Dec_X(void)
{ //6A
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  temp8--;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  cc[V] = (temp8 == 0x7F);
  MemWrite8(temp8, temp16);
  CycleCounter += 6;
}

void Tim_X(void)
{ //6B 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(INDADDRESS(PC_REG++));
  postbyte &= temp8;
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  CycleCounter += 7;
}

void Inc_X(void)
{ //6C
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  temp8++;
  cc[V] = (temp8 == 0x80);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  CycleCounter += 6;
}

void Tst_X(void)
{ //6D
  temp8 = MemRead8(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  cc[V] = 0;
  CycleCounter += NatEmuCycles65;
}

void Jmp_X(void)
{ //6E
  PC_REG = INDADDRESS(PC_REG++);
  CycleCounter += 3;
}

void Clr_X(void)
{ //6F
  MemWrite8(0, INDADDRESS(PC_REG++));
  cc[C] = 0;
  cc[N] = 0;
  cc[V] = 0;
  cc[Z] = 1;
  CycleCounter += 6;
}

void Neg_E(void)
{ //70
  temp16 = IMMADDRESS(PC_REG);
  postbyte = MemRead8(temp16);
  temp8 = 0 - postbyte;
  cc[C] = temp8 > 0;
  cc[V] = postbyte == 0x80;
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Oim_E(void)
{ //71 6309 Phase 2
  postbyte = MemRead8(PC_REG++);
  temp16 = IMMADDRESS(PC_REG);
  postbyte |= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += 7;
}

void Aim_E(void)
{ //72 6309 Untested CHECK NITRO
  postbyte = MemRead8(PC_REG++);
  temp16 = IMMADDRESS(PC_REG);
  postbyte &= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += 7;
}

void Com_E(void)
{ //73
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  temp8 = 0xFF - temp8;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  cc[C] = 1;
  cc[V] = 0;
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Lsr_E(void)
{  //74
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  cc[C] = temp8 & 1;
  temp8 = temp8 >> 1;
  cc[Z] = ZTEST(temp8);
  cc[N] = 0;
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Eim_E(void)
{ //75 6309 Untested CHECK NITRO
  postbyte = MemRead8(PC_REG++);
  temp16 = IMMADDRESS(PC_REG);
  postbyte ^= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += 7;
}

void Ror_E(void)
{ //76
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  postbyte = cc[C] << 7;
  cc[C] = temp8 & 1;
  temp8 = (temp8 >> 1) | postbyte;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Asr_E(void)
{ //77
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  cc[C] = temp8 & 1;
  temp8 = (temp8 & 0x80) | (temp8 >> 1);
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Asl_E(void)
{ //78 6309
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  cc[C] = temp8 > 0x7F;
  cc[V] = cc[C] ^ ((temp8 & 0x40) >> 6);
  temp8 = temp8 << 1;
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Rol_E(void)
{ //79
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  postbyte = cc[C];
  cc[C] = temp8 > 0x7F;
  cc[V] = cc[C] ^ ((temp8 & 0x40) >> 6);
  temp8 = ((temp8 << 1) | postbyte);
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Dec_E(void)
{ //7A
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  temp8--;
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  cc[V] = temp8 == 0x7F;
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Tim_E(void)
{ //7B 6309 NITRO 
  postbyte = MemRead8(PC_REG++);
  temp16 = IMMADDRESS(PC_REG);
  postbyte &= MemRead8(temp16);
  cc[N] = NTEST8(postbyte);
  cc[Z] = ZTEST(postbyte);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += 7;
}

void Inc_E(void)
{ //7C
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  temp8++;
  cc[Z] = ZTEST(temp8);
  cc[V] = temp8 == 0x80;
  cc[N] = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Tst_E(void)
{ //7D
  temp8 = MemRead8(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(temp8);
  cc[N] = NTEST8(temp8);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles75;
}

void Jmp_E(void)
{ //7E
  PC_REG = IMMADDRESS(PC_REG);
  CycleCounter += NatEmuCycles43;
}

void Clr_E(void)
{ //7F
  MemWrite8(0, IMMADDRESS(PC_REG));
  cc[C] = 0;
  cc[N] = 0;
  cc[V] = 0;
  cc[Z] = 1;
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Suba_M(void)
{ //80
  postbyte = MemRead8(PC_REG++);
  temp16 = A_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  CycleCounter += 2;
}

void Cmpa_M(void)
{ //81
  postbyte = MemRead8(PC_REG++);
  temp8 = A_REG - postbyte;
  cc[C] = temp8 > A_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, A_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += 2;
}

void Sbca_M(void)
{  //82
  postbyte = MemRead8(PC_REG++);
  temp16 = A_REG - postbyte - cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += 2;
}

void Subd_M(void)
{ //83
  temp16 = IMMADDRESS(PC_REG);
  temp32 = D_REG - temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, D_REG);
  D_REG = temp32;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles43;
}

void Anda_M(void)
{ //84
  A_REG = A_REG & MemRead8(PC_REG++);
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  CycleCounter += 2;
}

void Bita_M(void)
{ //85
  temp8 = A_REG & MemRead8(PC_REG++);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  cc[V] = 0;
  CycleCounter += 2;
}

void Lda_M(void)
{ //86
  A_REG = MemRead8(PC_REG++);
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  cc[V] = 0;
  CycleCounter += 2;
}

void Eora_M(void)
{ //88
  A_REG = A_REG ^ MemRead8(PC_REG++);
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  CycleCounter += 2;
}

void Adca_M(void)
{ //89
  postbyte = MemRead8(PC_REG++);
  temp16 = A_REG + postbyte + cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  cc[H] = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += 2;
}

void Ora_M(void)
{ //8A
  A_REG = A_REG | MemRead8(PC_REG++);
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  CycleCounter += 2;
}

void Adda_M(void)
{ //8B
  postbyte = MemRead8(PC_REG++);
  temp16 = A_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += 2;
}

void Cmpx_M(void)
{ //8C
  postword = IMMADDRESS(PC_REG);
  temp16 = X_REG - postword;
  cc[C] = temp16 > X_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, X_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles43;
}

void Bsr_R(void)
{ //8D
  *spostbyte = MemRead8(PC_REG++);
  S_REG--;
  MemWrite8(pc.B.lsb, S_REG--);
  MemWrite8(pc.B.msb, S_REG);
  PC_REG += *spostbyte;
  CycleCounter += NatEmuCycles76;
}

void Ldx_M(void)
{ //8E
  X_REG = IMMADDRESS(PC_REG);
  cc[Z] = ZTEST(X_REG);
  cc[N] = NTEST16(X_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += 3;
}

void Suba_D(void)
{ //90
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = A_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  CycleCounter += NatEmuCycles43;
}

void Cmpa_D(void)
{ //91
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp8 = A_REG - postbyte;
  cc[C] = temp8 > A_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, A_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += NatEmuCycles43;
}

void Scba_D(void)
{ //92
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = A_REG - postbyte - cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += NatEmuCycles43;
}

void Subd_D(void)
{ //93
  temp16 = MemRead16(DPADDRESS(PC_REG++));
  temp32 = D_REG - temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, D_REG);
  D_REG = temp32;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  CycleCounter += NatEmuCycles64;
}

void Anda_D(void)
{ //94
  A_REG = A_REG & MemRead8(DPADDRESS(PC_REG++));
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Bita_D(void)
{ //95
  temp8 = A_REG & MemRead8(DPADDRESS(PC_REG++));
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Lda_D(void)
{ //96
  A_REG = MemRead8(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Sta_D(void)
{ //97
  MemWrite8(A_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Eora_D(void)
{ //98
  A_REG = A_REG ^ MemRead8(DPADDRESS(PC_REG++));
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Adca_D(void)
{ //99
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = A_REG + postbyte + cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  cc[H] = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += NatEmuCycles43;
}

void Ora_D(void)
{ //9A
  A_REG = A_REG | MemRead8(DPADDRESS(PC_REG++));
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Adda_D(void)
{ //9B
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = A_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += NatEmuCycles43;
}

void Cmpx_D(void)
{ //9C
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = X_REG - postword;
  cc[C] = temp16 > X_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, X_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles64;
}

void Jsr_D(void)
{ //9D
  temp16 = DPADDRESS(PC_REG++);
  S_REG--;
  MemWrite8(pc.B.lsb, S_REG--);
  MemWrite8(pc.B.msb, S_REG);
  PC_REG = temp16;
  CycleCounter += NatEmuCycles76;
}

void Ldx_D(void)
{ //9E
  X_REG = MemRead16(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(X_REG);
  cc[N] = NTEST16(X_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Stx_D(void)
{ //9F
  MemWrite16(X_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(X_REG);
  cc[N] = NTEST16(X_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Suba_X(void)
{ //A0
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = A_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  CycleCounter += 4;
}

void Cmpa_X(void)
{ //A1
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp8 = A_REG - postbyte;
  cc[C] = temp8 > A_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, A_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += 4;
}

void Sbca_X(void)
{ //A2
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = A_REG - postbyte - cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += 4;
}

void Subd_X(void)
{ //A3
  temp16 = MemRead16(INDADDRESS(PC_REG++));
  temp32 = D_REG - temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, D_REG);
  D_REG = temp32;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  CycleCounter += NatEmuCycles65;
}

void Anda_X(void)
{ //A4
  A_REG = A_REG & MemRead8(INDADDRESS(PC_REG++));
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Bita_X(void)
{  //A5
  temp8 = A_REG & MemRead8(INDADDRESS(PC_REG++));
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  cc[V] = 0;
  CycleCounter += 4;
}

void Lda_X(void)
{ //A6
  A_REG = MemRead8(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Sta_X(void)
{ //A7
  MemWrite8(A_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Eora_X(void)
{ //A8
  A_REG = A_REG ^ MemRead8(INDADDRESS(PC_REG++));
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Adca_X(void)
{ //A9	
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = A_REG + postbyte + cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  cc[H] = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += 4;
}

void Ora_X(void)
{ //AA
  A_REG = A_REG | MemRead8(INDADDRESS(PC_REG++));
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Adda_X(void)
{ //AB
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = A_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  CycleCounter += 4;
}

void Cmpx_X(void)
{ //AC
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = X_REG - postword;
  cc[C] = temp16 > X_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, X_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  CycleCounter += NatEmuCycles65;
}

void Jsr_X(void)
{ //AD
  temp16 = INDADDRESS(PC_REG++);
  S_REG--;
  MemWrite8(pc.B.lsb, S_REG--);
  MemWrite8(pc.B.msb, S_REG);
  PC_REG = temp16;
  CycleCounter += NatEmuCycles76;
}

void Ldx_X(void)
{ //AE
  X_REG = MemRead16(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(X_REG);
  cc[N] = NTEST16(X_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Stx_X(void)
{ //AF
  MemWrite16(X_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(X_REG);
  cc[N] = NTEST16(X_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Suba_E(void)
{ //B0
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = A_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Cmpa_E(void)
{ //B1
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp8 = A_REG - postbyte;
  cc[C] = temp8 > A_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, A_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Sbca_E(void)
{ //B2
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = A_REG - postbyte - cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Subd_E(void)
{ //B3
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = D_REG - temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, D_REG);
  D_REG = temp32;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles75;
}

void Anda_E(void)
{ //B4
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  A_REG = A_REG & postbyte;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Bita_E(void)
{ //B5
  temp8 = A_REG & MemRead8(IMMADDRESS(PC_REG));
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Lda_E(void)
{ //B6
  A_REG = MemRead8(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Sta_E(void)
{ //B7
  MemWrite8(A_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(A_REG);
  cc[N] = NTEST8(A_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Eora_E(void)
{  //B8
  A_REG = A_REG ^ MemRead8(IMMADDRESS(PC_REG));
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Adca_E(void)
{ //B9
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = A_REG + postbyte + cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  cc[H] = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Ora_E(void)
{ //BA
  A_REG = A_REG | MemRead8(IMMADDRESS(PC_REG));
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Adda_E(void)
{ //BB
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = A_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  cc[N] = NTEST8(A_REG);
  cc[Z] = ZTEST(A_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Cmpx_E(void)
{ //BC
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = X_REG - postword;
  cc[C] = temp16 > X_REG;
  cc[V] = OVERFLOW16(cc[C], postword, temp16, X_REG);
  cc[N] = NTEST16(temp16);
  cc[Z] = ZTEST(temp16);
  PC_REG += 2;
  CycleCounter += NatEmuCycles75;
}

void Bsr_E(void)
{ //BD
  postword = IMMADDRESS(PC_REG);
  PC_REG += 2;
  S_REG--;
  MemWrite8(pc.B.lsb, S_REG--);
  MemWrite8(pc.B.msb, S_REG);
  PC_REG = postword;
  CycleCounter += NatEmuCycles87;
}

void Ldx_E(void)
{ //BE
  X_REG = MemRead16(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(X_REG);
  cc[N] = NTEST16(X_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Stx_E(void)
{ //BF
  MemWrite16(X_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(X_REG);
  cc[N] = NTEST16(X_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Subb_M(void)
{ //C0
  postbyte = MemRead8(PC_REG++);
  temp16 = B_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  CycleCounter += 2;
}

void Cmpb_M(void)
{ //C1
  postbyte = MemRead8(PC_REG++);
  temp8 = B_REG - postbyte;
  cc[C] = temp8 > B_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, B_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += 2;
}

void Sbcb_M(void)
{ //C2
  postbyte = MemRead8(PC_REG++);
  temp16 = B_REG - postbyte - cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += 2;
}

void Addd_M(void)
{ //C3
  temp16 = IMMADDRESS(PC_REG);
  temp32 = D_REG + temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, D_REG);
  D_REG = temp32;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles43;
}

void Andb_M(void)
{ //C4 LOOK
  B_REG = B_REG & MemRead8(PC_REG++);
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  CycleCounter += 2;
}

void Bitb_M(void)
{ //C5
  temp8 = B_REG & MemRead8(PC_REG++);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  cc[V] = 0;
  CycleCounter += 2;
}

void Ldb_M(void)
{ //C6
  B_REG = MemRead8(PC_REG++);
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  cc[V] = 0;
  CycleCounter += 2;
}

void Eorb_M(void)
{ //C8
  B_REG = B_REG ^ MemRead8(PC_REG++);
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  CycleCounter += 2;
}

void Adcb_M(void)
{ //C9
  postbyte = MemRead8(PC_REG++);
  temp16 = B_REG + postbyte + cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  cc[H] = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += 2;
}

void Orb_M(void)
{ //CA
  B_REG = B_REG | MemRead8(PC_REG++);
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  CycleCounter += 2;
}

void Addb_M(void)
{ //CB
  postbyte = MemRead8(PC_REG++);
  temp16 = B_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += 2;
}

void Ldd_M(void)
{ //CC
  D_REG = IMMADDRESS(PC_REG);
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += 3;
}

void Ldq_M(void)
{ //CD 6309 WORK
  Q_REG = MemRead32(PC_REG);
  PC_REG += 4;
  cc[Z] = ZTEST(Q_REG);
  cc[N] = NTEST32(Q_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Ldu_M(void)
{ //CE
  U_REG = IMMADDRESS(PC_REG);
  cc[Z] = ZTEST(U_REG);
  cc[N] = NTEST16(U_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += 3;
}

void Subb_D(void)
{ //D0
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = B_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  CycleCounter += NatEmuCycles43;
}

void Cmpb_D(void)
{ //D1
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp8 = B_REG - postbyte;
  cc[C] = temp8 > B_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, B_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += NatEmuCycles43;
}

void Sbcb_D(void)
{ //D2
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = B_REG - postbyte - cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += NatEmuCycles43;
}

void Addd_D(void)
{ //D3
  temp16 = MemRead16(DPADDRESS(PC_REG++));
  temp32 = D_REG + temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, D_REG);
  D_REG = temp32;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  CycleCounter += NatEmuCycles64;
}

void Andb_D(void)
{ //D4 
  B_REG = B_REG & MemRead8(DPADDRESS(PC_REG++));
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Bitb_D(void)
{ //D5
  temp8 = B_REG & MemRead8(DPADDRESS(PC_REG++));
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Ldb_D(void)
{ //D6
  B_REG = MemRead8(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Stb_D(void)
{ //D7
  MemWrite8(B_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Eorb_D(void)
{ //D8	
  B_REG = B_REG ^ MemRead8(DPADDRESS(PC_REG++));
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Adcb_D(void)
{ //D9
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = B_REG + postbyte + cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  cc[H] = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += NatEmuCycles43;
}

void Orb_D(void)
{ //DA
  B_REG = B_REG | MemRead8(DPADDRESS(PC_REG++));
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles43;
}

void Addb_D(void)
{ //DB
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = B_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += NatEmuCycles43;
}

void Ldd_D(void)
{ //DC
  D_REG = MemRead16(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Std_D(void)
{ //DD
  MemWrite16(D_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Ldu_D(void)
{ //DE
  U_REG = MemRead16(DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(U_REG);
  cc[N] = NTEST16(U_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Stu_D(void)
{ //DF
  MemWrite16(U_REG, DPADDRESS(PC_REG++));
  cc[Z] = ZTEST(U_REG);
  cc[N] = NTEST16(U_REG);
  cc[V] = 0;
  CycleCounter += NatEmuCycles54;
}

void Subb_X(void)
{ //E0
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = B_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  CycleCounter += 4;
}

void Cmpb_X(void)
{ //E1
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp8 = B_REG - postbyte;
  cc[C] = temp8 > B_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, B_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  CycleCounter += 4;
}

void Sbcb_X(void)
{ //E2
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = B_REG - postbyte - cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += 4;
}

void Addd_X(void)
{ //E3 
  temp16 = MemRead16(INDADDRESS(PC_REG++));
  temp32 = D_REG + temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, D_REG);
  D_REG = temp32;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  CycleCounter += NatEmuCycles65;
}

void Andb_X(void)
{ //E4
  B_REG = B_REG & MemRead8(INDADDRESS(PC_REG++));
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Bitb_X(void)
{ //E5 
  temp8 = B_REG & MemRead8(INDADDRESS(PC_REG++));
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  cc[V] = 0;
  CycleCounter += 4;
}

void Ldb_X(void)
{ //E6
  B_REG = MemRead8(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Stb_X(void)
{ //E7
  MemWrite8(B_REG, hd6309_CalculateEA(MemRead8(PC_REG++)));
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Eorb_X(void)
{ //E8
  B_REG = B_REG ^ MemRead8(INDADDRESS(PC_REG++));
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Adcb_X(void)
{ //E9
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = B_REG + postbyte + cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  cc[H] = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += 4;
}

void Orb_X(void)
{ //EA 
  B_REG = B_REG | MemRead8(INDADDRESS(PC_REG++));
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  CycleCounter += 4;
}

void Addb_X(void)
{ //EB
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = B_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  CycleCounter += 4;
}

void Ldd_X(void)
{ //EC
  D_REG = MemRead16(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Std_X(void)
{ //ED
  MemWrite16(D_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Ldu_X(void)
{ //EE
  U_REG = MemRead16(INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(U_REG);
  cc[N] = NTEST16(U_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Stu_X(void)
{ //EF
  MemWrite16(U_REG, INDADDRESS(PC_REG++));
  cc[Z] = ZTEST(U_REG);
  cc[N] = NTEST16(U_REG);
  cc[V] = 0;
  CycleCounter += 5;
}

void Subb_E(void)
{ //F0
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = B_REG - postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Cmpb_E(void)
{ //F1
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp8 = B_REG - postbyte;
  cc[C] = temp8 > B_REG;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp8, B_REG);
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Sbcb_E(void)
{ //F2
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = B_REG - postbyte - cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Addd_E(void)
{ //F3
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = D_REG + temp16;
  cc[C] = (temp32 & 0x10000) >> 16;
  cc[V] = OVERFLOW16(cc[C], temp32, temp16, D_REG);
  D_REG = temp32;
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles76;
}

void Andb_E(void)
{  //F4
  B_REG = B_REG & MemRead8(IMMADDRESS(PC_REG));
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Bitb_E(void)
{ //F5
  temp8 = B_REG & MemRead8(IMMADDRESS(PC_REG));
  cc[N] = NTEST8(temp8);
  cc[Z] = ZTEST(temp8);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Ldb_E(void)
{ //F6
  B_REG = MemRead8(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Stb_E(void)
{ //F7 
  MemWrite8(B_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(B_REG);
  cc[N] = NTEST8(B_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Eorb_E(void)
{ //F8
  B_REG = B_REG ^ MemRead8(IMMADDRESS(PC_REG));
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Adcb_E(void)
{ //F9
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = B_REG + postbyte + cc[C];
  cc[C] = (temp16 & 0x100) >> 8;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  cc[H] = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Orb_E(void)
{ //FA
  B_REG = B_REG | MemRead8(IMMADDRESS(PC_REG));
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Addb_E(void)
{ //FB
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = B_REG + postbyte;
  cc[C] = (temp16 & 0x100) >> 8;
  cc[H] = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  cc[V] = OVERFLOW8(cc[C], postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  cc[N] = NTEST8(B_REG);
  cc[Z] = ZTEST(B_REG);
  PC_REG += 2;
  CycleCounter += NatEmuCycles54;
}

void Ldd_E(void)
{ //FC
  D_REG = MemRead16(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Std_E(void)
{ //FD
  MemWrite16(D_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(D_REG);
  cc[N] = NTEST16(D_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Ldu_E(void)
{ //FE
  U_REG = MemRead16(IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(U_REG);
  cc[N] = NTEST16(U_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}

void Stu_E(void)
{ //FF
  MemWrite16(U_REG, IMMADDRESS(PC_REG));
  cc[Z] = ZTEST(U_REG);
  cc[N] = NTEST16(U_REG);
  cc[V] = 0;
  PC_REG += 2;
  CycleCounter += NatEmuCycles65;
}
