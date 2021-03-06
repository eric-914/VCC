#include "HD6309.h"
#include "TC1014MMU.h"

#include "HD6309Macros.h"

static HD6309State* instance = GetHD6309State();

static signed short stemp16;
static int stemp32;

static unsigned char temp8;
static unsigned short temp16;
static unsigned int temp32;

static unsigned char postbyte = 0;
static short unsigned postword = 0;
static signed char* spostbyte = (signed char*)&postbyte;
static signed short* spostword = (signed short*)&postword;

static unsigned char Source = 0;
static unsigned char Dest = 0;

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

    instance->CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(HD6309_getcc(), --S_REG);

  PC_REG = MemRead16(VTRAP);

  instance->CycleCounter += (12 + instance->NatEmuCycles54);	//One for each byte +overhead? Guessing from PSHS
}

void InvalidInsHandler(void)
{
  MD_ILLEGAL = 1;
  instance->mdbits = HD6309_getmd();

  ErrorVector();
}

void DivbyZero(void)
{
  MD_ZERODIV = 1;

  instance->mdbits = HD6309_getmd();

  ErrorVector();
}

void Page_2(void);
void Page_3(void);

void Neg_D(void)
{ //0
  temp16 = DPADDRESS(PC_REG++);
  postbyte = MemRead8(temp16);
  temp8 = 0 - postbyte;
  
  CC_C = temp8 > 0;
  CC_V = (postbyte == 0x80);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  
  MemWrite8(temp8, temp16);
  
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Oim_D(void)
{//1 6309
  postbyte = MemRead8(PC_REG++);
  temp16 = DPADDRESS(PC_REG++);
  postbyte |= MemRead8(temp16);
  
  MemWrite8(postbyte, temp16);
  
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  
  instance->CycleCounter += 6;
}

void Aim_D(void)
{//2 Phase 2 6309
  postbyte = MemRead8(PC_REG++);
  temp16 = DPADDRESS(PC_REG++);
  postbyte &= MemRead8(temp16);
  
  MemWrite8(postbyte, temp16);
  
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  
  instance->CycleCounter += 6;
}

void Com_D(void)
{ //03
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  temp8 = 0xFF - temp8;
  
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  CC_C = 1;
  CC_V = 0;

  MemWrite8(temp8, temp16);

  instance->CycleCounter += instance->NatEmuCycles65;
}

void Lsr_D(void)
{ //04 S2
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  CC_C = temp8 & 1;
  temp8 = temp8 >> 1;
  CC_Z = ZTEST(temp8);
  CC_N = 0;
  MemWrite8(temp8, temp16);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Eim_D(void)
{ //05 6309 Untested
  postbyte = MemRead8(PC_REG++);
  temp16 = DPADDRESS(PC_REG++);
  postbyte ^= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Ror_D(void)
{ //06 S2
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  postbyte = CC_C << 7;
  CC_C = temp8 & 1;
  temp8 = (temp8 >> 1) | postbyte;
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Asr_D(void)
{ //7
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  CC_C = temp8 & 1;
  temp8 = (temp8 & 0x80) | (temp8 >> 1);
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Asl_D(void)
{ //8 
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  CC_C = (temp8 & 0x80) >> 7;
  CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
  temp8 = temp8 << 1;
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Rol_D(void)
{	//9
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  postbyte = CC_C;
  CC_C = (temp8 & 0x80) >> 7;
  CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
  temp8 = (temp8 << 1) | postbyte;
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Dec_D(void)
{ //A
  temp16 = DPADDRESS(PC_REG++);
  temp8 = MemRead8(temp16) - 1;
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  CC_V = temp8 == 0x7F;
  MemWrite8(temp8, temp16);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Tim_D(void)
{	//B 6309 Untested wcreate
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  postbyte &= temp8;
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Inc_D(void)
{ //C
  temp16 = (DPADDRESS(PC_REG++));
  temp8 = MemRead8(temp16) + 1;
  CC_Z = ZTEST(temp8);
  CC_V = temp8 == 0x80;
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Tst_D(void)
{ //D
  temp8 = MemRead8(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles64;
}

void Jmp_D(void)
{	//E
  PC_REG = ((DP_REG | MemRead8(PC_REG)));
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Clr_D(void)
{	//F
  MemWrite8(0, DPADDRESS(PC_REG++));
  CC_Z = 1;
  CC_N = 0;
  CC_V = 0;
  CC_C = 0;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void LBeq_R(void)
{ //1027
  if (CC_Z) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }
  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBrn_R(void)
{ //1021
  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBhi_R(void)
{ //1022
  if (!(CC_C | CC_Z)) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBls_R(void)
{ //1023
  if (CC_C | CC_Z) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBhs_R(void)
{ //1024
  if (!CC_C) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 6;
}

void LBcs_R(void)
{ //1025
  if (CC_C) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBne_R(void)
{ //1026
  if (!CC_Z) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBvc_R(void)
{ //1028
  if (!CC_V) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBvs_R(void)
{ //1029
  if (CC_V) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBpl_R(void)
{ //102A
  if (!CC_N) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBmi_R(void)
{ //102B
  if (CC_N) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBge_R(void)
{ //102C
  if (!(CC_N ^ CC_V)) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBlt_R(void)
{ //102D
  if (CC_V ^ CC_N) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBgt_R(void)
{ //102E
  if (!(CC_Z | (CC_N ^ CC_V))) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
}

void LBle_R(void)
{	//102F
  if (CC_Z | (CC_N ^ CC_V)) {
    *spostword = IMMADDRESS(PC_REG);
    PC_REG += *spostword;
    instance->CycleCounter += 1;
  }

  PC_REG += 2;
  instance->CycleCounter += 5;
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
      dest8 = HD6309_getcc();
    }
    else {
      dest8 = PUR(Dest);
    }

    if (Source > 7) { // 8 bit source
      Source &= 7;

      if (Source == 2) {
        source8 = HD6309_getcc();
      }
      else {
        source8 = PUR(Source);
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)PXF(Source);
    }

    temp16 = source8 + dest8;

    switch (Dest)
    {
    case 2: 				HD6309_setcc((unsigned char)temp16); break;
    case 4: case 5: break; // never assign to zero reg
    default: 				PUR(Dest) = (unsigned char)temp16; break;
    }

    CC_C = (temp16 & 0x100) >> 8;
    CC_V = OVERFLOW8(CC_C, source8, dest8, temp16);
    CC_N = NTEST8(PUR(Dest));
    CC_Z = ZTEST(PUR(Dest));
  }
  else // 16 bit dest
  {
    dest16 = PXF(Dest);

    if (Source < 8) // 16 bit source
    {
      source16 = PXF(Source);
    }
    else // 8 bit source - promote to 16 bit
    {
      Source &= 7;

      switch (Source)
      {
      case 0: case 1: source16 = D_REG; break; // A & B Reg
      case 2:	        source16 = (unsigned short)HD6309_getcc(); break; // CC
      case 3:	        source16 = (unsigned short)DP_REG; break; // DP
      case 4: case 5: source16 = 0; break; // Zero Reg
      case 6: case 7: source16 = W_REG; break; // E & F Reg
      }
    }

    temp32 = source16 + dest16;
    PXF(Dest) = (unsigned short)temp32;
    CC_C = (temp32 & 0x10000) >> 16;
    CC_V = OVERFLOW16(CC_C, source16, dest16, temp32);
    CC_N = NTEST16(PXF(Dest));
    CC_Z = ZTEST(PXF(Dest));
  }

  instance->CycleCounter += 4;
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
      dest8 = HD6309_getcc();
    }
    else {
      dest8 = PUR(Dest);
    }

    if (Source > 7) { // 8 bit source
      Source &= 7;

      if (Source == 2) {
        source8 = HD6309_getcc();
      }
      else {
        source8 = PUR(Source);
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)PXF(Source);
    }

    temp16 = source8 + dest8 + CC_C;

    switch (Dest)
    {
    case 2:
      HD6309_setcc((unsigned char)temp16);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      PUR(Dest) = (unsigned char)temp16;
      break;
    }

    CC_C = (temp16 & 0x100) >> 8;
    CC_V = OVERFLOW8(CC_C, source8, dest8, temp16);
    CC_N = NTEST8(PUR(Dest));
    CC_Z = ZTEST(PUR(Dest));
  }
  else // 16 bit dest
  {
    dest16 = PXF(Dest);

    if (Source < 8) // 16 bit source
    {
      source16 = PXF(Source);
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
        source16 = (unsigned short)HD6309_getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)DP_REG;
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

    temp32 = source16 + dest16 + CC_C;
    PXF(Dest) = (unsigned short)temp32;
    CC_C = (temp32 & 0x10000) >> 16;
    CC_V = OVERFLOW16(CC_C, source16, dest16, temp32);
    CC_N = NTEST16(PXF(Dest));
    CC_Z = ZTEST(PXF(Dest));
  }

  instance->CycleCounter += 4;
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
      dest8 = HD6309_getcc();
    }
    else {
      dest8 = PUR(Dest);
    }

    if (Source > 7) { // 8 bit source
      Source &= 7;

      if (Source == 2) {
        source8 = HD6309_getcc();
      }
      else {
        source8 = PUR(Source);
      }
    }
    else { // 16 bit source - demote to 8 bit
      Source &= 7;
      source8 = (unsigned char)PXF(Source);
    }

    temp16 = dest8 - source8;

    switch (Dest)
    {
    case 2:
      HD6309_setcc((unsigned char)temp16);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      PUR(Dest) = (unsigned char)temp16;
      break;
    }

    CC_C = (temp16 & 0x100) >> 8;
    CC_V = CC_C ^ ((dest8 ^ PUR(Dest) ^ source8) >> 7);
    CC_N = PUR(Dest) >> 7;
    CC_Z = ZTEST(PUR(Dest));
  }
  else // 16 bit dest
  {
    dest16 = PXF(Dest);

    if (Source < 8) // 16 bit source
    {
      source16 = PXF(Source);
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
        source16 = (unsigned short)HD6309_getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)DP_REG;
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
    CC_C = (temp32 & 0x10000) >> 16;
    CC_V = !!((dest16 ^ source16 ^ temp32 ^ (temp32 >> 1)) & 0x8000);
    PXF(Dest) = (unsigned short)temp32;
    CC_N = (temp32 & 0x8000) >> 15;
    CC_Z = ZTEST(temp32);
  }

  instance->CycleCounter += 4;
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
      dest8 = HD6309_getcc();
    }
    else {
      dest8 = PUR(Dest);
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = HD6309_getcc();
      }
      else {
        source8 = PUR(Source);
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)PXF(Source);
    }

    temp16 = dest8 - source8 - CC_C;

    switch (Dest)
    {
    case 2:
      HD6309_setcc((unsigned char)temp16);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      PUR(Dest) = (unsigned char)temp16;
      break;
    }

    CC_C = (temp16 & 0x100) >> 8;
    CC_V = CC_C ^ ((dest8 ^ PUR(Dest) ^ source8) >> 7);
    CC_N = PUR(Dest) >> 7;
    CC_Z = ZTEST(PUR(Dest));
  }
  else // 16 bit dest
  {
    dest16 = PXF(Dest);

    if (Source < 8) // 16 bit source
    {
      source16 = PXF(Source);
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
        source16 = (unsigned short)HD6309_getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)DP_REG;
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

    temp32 = dest16 - source16 - CC_C;
    CC_C = (temp32 & 0x10000) >> 16;
    CC_V = !!((dest16 ^ source16 ^ temp32 ^ (temp32 >> 1)) & 0x8000);
    PXF(Dest) = (unsigned short)temp32;
    CC_N = (temp32 & 0x8000) >> 15;
    CC_Z = ZTEST(temp32);
  }

  instance->CycleCounter += 4;
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
      dest8 = HD6309_getcc();
    }
    else {
      dest8 = PUR(Dest);
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = HD6309_getcc();
      }
      else {
        source8 = PUR(Source);
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)PXF(Source);
    }

    temp8 = dest8 & source8;

    switch (Dest)
    {
    case 2:
      HD6309_setcc((unsigned char)temp8);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      PUR(Dest) = (unsigned char)temp8;
      break;
    }

    CC_N = temp8 >> 7;
    CC_Z = ZTEST(temp8);
  }
  else // 16 bit dest
  {
    dest16 = PXF(Dest);

    if (Source < 8) // 16 bit source
    {
      source16 = PXF(Source);
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
        source16 = (unsigned short)HD6309_getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)DP_REG;
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
    PXF(Dest) = temp16;
    CC_N = temp16 >> 15;
    CC_Z = ZTEST(temp16);
  }
  CC_V = 0;
  instance->CycleCounter += 4;
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
      dest8 = HD6309_getcc();
    }
    else {
      dest8 = PUR(Dest);
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = HD6309_getcc();
      }
      else {
        source8 = PUR(Source);
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)PXF(Source);
    }

    temp8 = dest8 | source8;

    switch (Dest)
    {
    case 2:
      HD6309_setcc((unsigned char)temp8);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      PUR(Dest) = (unsigned char)temp8;
      break;
    }

    CC_N = temp8 >> 7;
    CC_Z = ZTEST(temp8);
  }
  else // 16 bit dest
  {
    dest16 = PXF(Dest);

    if (Source < 8) // 16 bit source
    {
      source16 = PXF(Source);
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
        source16 = (unsigned short)HD6309_getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)DP_REG;
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
    PXF(Dest) = temp16;
    CC_N = temp16 >> 15;
    CC_Z = ZTEST(temp16);
  }

  CC_V = 0;
  instance->CycleCounter += 4;
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
      dest8 = HD6309_getcc();
    }
    else {
      dest8 = PUR(Dest);
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = HD6309_getcc();
      }
      else {
        source8 = PUR(Source);
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)PXF(Source);
    }

    temp8 = dest8 ^ source8;

    switch (Dest)
    {
    case 2:
      HD6309_setcc((unsigned char)temp8);
      break;

    case 4:
    case 5:
      break; // never assign to zero reg

    default:
      PUR(Dest) = (unsigned char)temp8;
      break;
    }

    CC_N = temp8 >> 7;
    CC_Z = ZTEST(temp8);
  }
  else // 16 bit dest
  {
    dest16 = PXF(Dest);

    if (Source < 8) // 16 bit source
    {
      source16 = PXF(Source);
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
        source16 = (unsigned short)HD6309_getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)DP_REG;
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
    PXF(Dest) = temp16;
    CC_N = temp16 >> 15;
    CC_Z = ZTEST(temp16);
  }

  CC_V = 0;
  instance->CycleCounter += 4;
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
      dest8 = HD6309_getcc();
    }
    else {
      dest8 = PUR(Dest);
    }

    if (Source > 7) // 8 bit source
    {
      Source &= 7;

      if (Source == 2) {
        source8 = HD6309_getcc();
      }
      else {
        source8 = PUR(Source);
      }
    }
    else // 16 bit source - demote to 8 bit
    {
      Source &= 7;
      source8 = (unsigned char)PXF(Source);
    }

    temp16 = dest8 - source8;
    temp8 = (unsigned char)temp16;
    CC_C = (temp16 & 0x100) >> 8;
    CC_V = CC_C ^ ((dest8 ^ temp8 ^ source8) >> 7);
    CC_N = temp8 >> 7;
    CC_Z = ZTEST(temp8);
  }
  else // 16 bit dest
  {
    dest16 = PXF(Dest);

    if (Source < 8) // 16 bit source
    {
      source16 = PXF(Source);
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
        source16 = (unsigned short)HD6309_getcc();
        break; // CC

      case 3:
        source16 = (unsigned short)DP_REG;
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
    CC_C = (temp32 & 0x10000) >> 16;
    CC_V = !!((dest16 ^ source16 ^ temp32 ^ (temp32 >> 1)) & 0x8000);
    CC_N = (temp32 & 0x8000) >> 15;
    CC_Z = ZTEST(temp32);
  }

  instance->CycleCounter += 4;
}

void Pshsw(void)
{ //1038 DONE 6309
  MemWrite8((F_REG), --S_REG);
  MemWrite8((E_REG), --S_REG);
  instance->CycleCounter += 6;
}

void Pulsw(void)
{	//1039 6309 Untested wcreate
  E_REG = MemRead8(S_REG++);
  F_REG = MemRead8(S_REG++);
  instance->CycleCounter += 6;
}

void Pshuw(void)
{ //103A 6309 Untested
  MemWrite8((F_REG), --U_REG);
  MemWrite8((E_REG), --U_REG);
  instance->CycleCounter += 6;
}

void Puluw(void)
{ //103B 6309 Untested
  E_REG = MemRead8(U_REG++);
  F_REG = MemRead8(U_REG++);
  instance->CycleCounter += 6;
}

void Swi2_I(void)
{ //103F
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
    instance->CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(HD6309_getcc(), --S_REG);
  PC_REG = MemRead16(VSWI2);
  instance->CycleCounter += 20;
}

void Negd_I(void)
{ //1040 Phase 5 6309
  D_REG = 0 - D_REG;
  CC_C = temp16 > 0;
  CC_V = D_REG == 0x8000;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Comd_I(void)
{ //1043 6309
  D_REG = 0xFFFF - D_REG;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  CC_C = 1;
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Lsrd_I(void)
{ //1044 6309
  CC_C = D_REG & 1;
  D_REG = D_REG >> 1;
  CC_Z = ZTEST(D_REG);
  CC_N = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Rord_I(void)
{ //1046 6309 Untested
  postword = CC_C << 15;
  CC_C = D_REG & 1;
  D_REG = (D_REG >> 1) | postword;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Asrd_I(void)
{ //1047 6309 Untested TESTED NITRO MULTIVUE
  CC_C = D_REG & 1;
  D_REG = (D_REG & 0x8000) | (D_REG >> 1);
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Asld_I(void)
{ //1048 6309
  CC_C = D_REG >> 15;
  CC_V = CC_C ^ ((D_REG & 0x4000) >> 14);
  D_REG = D_REG << 1;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Rold_I(void)
{ //1049 6309 Untested
  postword = CC_C;
  CC_C = D_REG >> 15;
  CC_V = CC_C ^ ((D_REG & 0x4000) >> 14);
  D_REG = (D_REG << 1) | postword;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Decd_I(void)
{ //104A 6309
  D_REG--;
  CC_Z = ZTEST(D_REG);
  CC_V = D_REG == 0x7FFF;
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Incd_I(void)
{ //104C 6309
  D_REG++;
  CC_Z = ZTEST(D_REG);
  CC_V = D_REG == 0x8000;
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Tstd_I(void)
{ //104D 6309
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Clrd_I(void)
{ //104F 6309
  D_REG = 0;
  CC_C = 0;
  CC_V = 0;
  CC_N = 0;
  CC_Z = 1;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Comw_I(void)
{ //1053 6309 Untested
  W_REG = 0xFFFF - W_REG;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  CC_C = 1;
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Lsrw_I(void)
{ //1054 6309 Untested
  CC_C = W_REG & 1;
  W_REG = W_REG >> 1;
  CC_Z = ZTEST(W_REG);
  CC_N = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Rorw_I(void)
{ //1056 6309 Untested
  postword = CC_C << 15;
  CC_C = W_REG & 1;
  W_REG = (W_REG >> 1) | postword;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Rolw_I(void)
{ //1059 6309
  postword = CC_C;
  CC_C = W_REG >> 15;
  CC_V = CC_C ^ ((W_REG & 0x4000) >> 14);
  W_REG = (W_REG << 1) | postword;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Decw_I(void)
{ //105A 6309
  W_REG--;
  CC_Z = ZTEST(W_REG);
  CC_V = W_REG == 0x7FFF;
  CC_N = NTEST16(W_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Incw_I(void)
{ //105C 6309
  W_REG++;
  CC_Z = ZTEST(W_REG);
  CC_V = W_REG == 0x8000;
  CC_N = NTEST16(W_REG);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Tstw_I(void)
{ //105D Untested 6309 wcreate
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Clrw_I(void)
{ //105F 6309
  W_REG = 0;
  CC_C = 0;
  CC_V = 0;
  CC_N = 0;
  CC_Z = 1;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Subw_M(void)
{ //1080 6309 CHECK
  postword = IMMADDRESS(PC_REG);
  temp16 = W_REG - postword;
  CC_C = temp16 > W_REG;
  CC_V = OVERFLOW16(CC_C, temp16, W_REG, postword);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  W_REG = temp16;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmpw_M(void)
{ //1081 6309 CHECK
  postword = IMMADDRESS(PC_REG);
  temp16 = W_REG - postword;
  CC_C = temp16 > W_REG;
  CC_V = OVERFLOW16(CC_C, temp16, W_REG, postword);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Sbcd_M(void)
{ //1082 6309
  postword = IMMADDRESS(PC_REG);
  temp32 = D_REG - postword - CC_C;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, D_REG, postword);
  D_REG = temp32;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmpd_M(void)
{ //1083
  postword = IMMADDRESS(PC_REG);
  temp16 = D_REG - postword;
  CC_C = temp16 > D_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, D_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Andd_M(void)
{ //1084 6309
  D_REG &= IMMADDRESS(PC_REG);
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Bitd_M(void)
{ //1085 6309 Untested
  temp16 = D_REG & IMMADDRESS(PC_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Ldw_M(void)
{ //1086 6309
  W_REG = IMMADDRESS(PC_REG);
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Eord_M(void)
{ //1088 6309 Untested
  D_REG ^= IMMADDRESS(PC_REG);
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Adcd_M(void)
{ //1089 6309
  postword = IMMADDRESS(PC_REG);
  temp32 = D_REG + postword + CC_C;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, postword, temp32, D_REG);
  CC_H = ((D_REG ^ temp32 ^ postword) & 0x100) >> 8;
  D_REG = temp32;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Ord_M(void)
{ //108A 6309 Untested
  D_REG |= IMMADDRESS(PC_REG);
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Addw_M(void)
{ //108B Phase 5 6309
  temp16 = IMMADDRESS(PC_REG);
  temp32 = W_REG + temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, W_REG);
  W_REG = temp32;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmpy_M(void)
{ //108C
  postword = IMMADDRESS(PC_REG);
  temp16 = Y_REG - postword;
  CC_C = temp16 > Y_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, Y_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Ldy_M(void)
{ //108E
  Y_REG = IMMADDRESS(PC_REG);
  CC_Z = ZTEST(Y_REG);
  CC_N = NTEST16(Y_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Subw_D(void)
{ //1090 Untested 6309
  temp16 = MemRead16(DPADDRESS(PC_REG++));
  temp32 = W_REG - temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, W_REG);
  W_REG = temp32;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Cmpw_D(void)
{ //1091 6309 Untested
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = W_REG - postword;
  CC_C = temp16 > W_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, W_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Sbcd_D(void)
{ //1092 6309
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp32 = D_REG - postword - CC_C;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, D_REG, postword);
  D_REG = temp32;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Cmpd_D(void)
{ //1093
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = D_REG - postword;
  CC_C = temp16 > D_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, D_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Andd_D(void)
{ //1094 6309 Untested
  postword = MemRead16(DPADDRESS(PC_REG++));
  D_REG &= postword;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Bitd_D(void)
{ //1095 6309 Untested
  temp16 = D_REG & MemRead16(DPADDRESS(PC_REG++));
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Ldw_D(void)
{ //1096 6309
  W_REG = MemRead16(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Stw_D(void)
{ //1097 6309
  MemWrite16(W_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Eord_D(void)
{ //1098 6309 Untested
  D_REG ^= MemRead16(DPADDRESS(PC_REG++));
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Adcd_D(void)
{ //1099 6309
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp32 = D_REG + postword + CC_C;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, postword, temp32, D_REG);
  CC_H = ((D_REG ^ temp32 ^ postword) & 0x100) >> 8;
  D_REG = temp32;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Ord_D(void)
{ //109A 6309 Untested
  D_REG |= MemRead16(DPADDRESS(PC_REG++));
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Addw_D(void)
{ //109B 6309
  temp16 = MemRead16(DPADDRESS(PC_REG++));
  temp32 = W_REG + temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, W_REG);
  W_REG = temp32;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Cmpy_D(void)
{	//109C
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = Y_REG - postword;
  CC_C = temp16 > Y_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, Y_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Ldy_D(void)
{ //109E
  Y_REG = MemRead16(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(Y_REG);
  CC_N = NTEST16(Y_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Sty_D(void)
{ //109F
  MemWrite16(Y_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(Y_REG);
  CC_N = NTEST16(Y_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Subw_X(void)
{ //10A0 6309 MODDED
  temp16 = MemRead16(INDADDRESS(PC_REG++));
  temp32 = W_REG - temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, W_REG);
  W_REG = temp32;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Cmpw_X(void)
{ //10A1 6309
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = W_REG - postword;
  CC_C = temp16 > W_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, W_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Sbcd_X(void)
{ //10A2 6309
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp32 = D_REG - postword - CC_C;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, postword, temp32, D_REG);
  D_REG = temp32;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Cmpd_X(void)
{ //10A3
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = D_REG - postword;
  CC_C = temp16 > D_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, D_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Andd_X(void)
{ //10A4 6309
  D_REG &= MemRead16(INDADDRESS(PC_REG++));
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Bitd_X(void)
{ //10A5 6309 Untested
  temp16 = D_REG & MemRead16(INDADDRESS(PC_REG++));
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Ldw_X(void)
{ //10A6 6309
  W_REG = MemRead16(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Stw_X(void)
{ //10A7 6309
  MemWrite16(W_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Eord_X(void)
{ //10A8 6309 Untested TESTED NITRO 
  D_REG ^= MemRead16(INDADDRESS(PC_REG++));
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Adcd_X(void)
{ //10A9 6309 untested
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp32 = D_REG + postword + CC_C;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, postword, temp32, D_REG);
  CC_H = (((D_REG ^ temp32 ^ postword) & 0x100) >> 8);
  D_REG = temp32;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Ord_X(void)
{ //10AA 6309 Untested wcreate
  D_REG |= MemRead16(INDADDRESS(PC_REG++));
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Addw_X(void)
{ //10AB 6309 Untested TESTED NITRO CHECK no Half carry?
  temp16 = MemRead16(INDADDRESS(PC_REG++));
  temp32 = W_REG + temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, W_REG);
  W_REG = temp32;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Cmpy_X(void)
{ //10AC
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = Y_REG - postword;
  CC_C = temp16 > Y_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, Y_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Ldy_X(void)
{ //10AE
  Y_REG = MemRead16(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(Y_REG);
  CC_N = NTEST16(Y_REG);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Sty_X(void)
{ //10AF
  MemWrite16(Y_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(Y_REG);
  CC_N = NTEST16(Y_REG);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Subw_E(void)
{ //10B0 6309 Untested
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = W_REG - temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, W_REG);
  W_REG = temp32;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Cmpw_E(void)
{ //10B1 6309 Untested
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = W_REG - postword;
  CC_C = temp16 > W_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, W_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Sbcd_E(void)
{ //10B2 6309 Untested
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = D_REG - temp16 - CC_C;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, D_REG);
  D_REG = temp32;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Cmpd_E(void)
{ //10B3
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = D_REG - postword;
  CC_C = temp16 > D_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, D_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Andd_E(void)
{ //10B4 6309 Untested
  D_REG &= MemRead16(IMMADDRESS(PC_REG));
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Bitd_E(void)
{ //10B5 6309 Untested CHECK NITRO
  temp16 = D_REG & MemRead16(IMMADDRESS(PC_REG));
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Ldw_E(void)
{ //10B6 6309
  W_REG = MemRead16(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Stw_E(void)
{ //10B7 6309
  MemWrite16(W_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Eord_E(void)
{ //10B8 6309 Untested
  D_REG ^= MemRead16(IMMADDRESS(PC_REG));
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Adcd_E(void)
{ //10B9 6309 untested
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp32 = D_REG + postword + CC_C;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, postword, temp32, D_REG);
  CC_H = (((D_REG ^ temp32 ^ postword) & 0x100) >> 8);
  D_REG = temp32;
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Ord_E(void)
{ //10BA 6309 Untested
  D_REG |= MemRead16(IMMADDRESS(PC_REG));
  CC_N = NTEST16(D_REG);
  CC_Z = ZTEST(D_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Addw_E(void)
{ //10BB 6309 Untested
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = W_REG + temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, W_REG);
  W_REG = temp32;
  CC_Z = ZTEST(W_REG);
  CC_N = NTEST16(W_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Cmpy_E(void)
{ //10BC
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = Y_REG - postword;
  CC_C = temp16 > Y_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, Y_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Ldy_E(void)
{ //10BE
  Y_REG = MemRead16(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(Y_REG);
  CC_N = NTEST16(Y_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Sty_E(void)
{ //10BF
  MemWrite16(Y_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(Y_REG);
  CC_N = NTEST16(Y_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Lds_I(void)
{  //10CE
  S_REG = IMMADDRESS(PC_REG);
  CC_Z = ZTEST(S_REG);
  CC_N = NTEST16(S_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += 4;
}

void Ldq_D(void)
{ //10DC 6309
  Q_REG = MemRead32(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(Q_REG);
  CC_N = NTEST32(Q_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles87;
}

void Stq_D(void)
{ //10DD 6309
  MemWrite32(Q_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(Q_REG);
  CC_N = NTEST32(Q_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles87;
}

void Lds_D(void)
{ //10DE
  S_REG = MemRead16(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(S_REG);
  CC_N = NTEST16(S_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Sts_D(void)
{ //10DF 6309
  MemWrite16(S_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(S_REG);
  CC_N = NTEST16(S_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Ldq_X(void)
{ //10EC 6309
  Q_REG = MemRead32(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(Q_REG);
  CC_N = NTEST32(Q_REG);
  CC_V = 0;
  instance->CycleCounter += 8;
}

void Stq_X(void)
{ //10ED 6309 DONE
  MemWrite32(Q_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(Q_REG);
  CC_N = NTEST32(Q_REG);
  CC_V = 0;
  instance->CycleCounter += 8;
}

void Lds_X(void)
{ //10EE
  S_REG = MemRead16(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(S_REG);
  CC_N = NTEST16(S_REG);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Sts_X(void)
{ //10EF 6309
  MemWrite16(S_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(S_REG);
  CC_N = NTEST16(S_REG);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Ldq_E(void)
{ //10FC 6309
  Q_REG = MemRead32(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(Q_REG);
  CC_N = NTEST32(Q_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles98;
}

void Stq_E(void)
{ //10FD 6309
  MemWrite32(Q_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(Q_REG);
  CC_N = NTEST32(Q_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles98;
}

void Lds_E(void)
{ //10FE
  S_REG = MemRead16(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(S_REG);
  CC_N = NTEST16(S_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Sts_E(void)
{ //10FF 6309
  MemWrite16(S_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(S_REG);
  CC_N = NTEST16(S_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
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
      PUR(postbyte) &= ~(1 << Dest);
      break;

    case 2: // CC Reg
      HD6309_setcc(HD6309_getcc() & ~(1 << Dest));
      break;
    }
  }

  // Else nothing changes
  instance->CycleCounter += instance->NatEmuCycles76;
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
      PUR(postbyte) &= ~(1 << Dest);
      break;

    case 2: // CC Reg
      HD6309_setcc(HD6309_getcc() & ~(1 << Dest));
      break;
    }
  }

  // Else do nothing
  instance->CycleCounter += instance->NatEmuCycles76;
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
      PUR(postbyte) |= (1 << Dest);
      break;

    case 2: // CC Reg
      HD6309_setcc(HD6309_getcc() | (1 << Dest));
      break;
    }
  }

  // Else do nothing
  instance->CycleCounter += instance->NatEmuCycles76;
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
      PUR(postbyte) |= (1 << Dest);
      break;

    case 2: // CC Reg
      HD6309_setcc(HD6309_getcc() | (1 << Dest));
      break;
    }
  }

  // Else do nothing
  instance->CycleCounter += instance->NatEmuCycles76;
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
      PUR(postbyte) ^= (1 << Dest);
      break;

    case 2: // CC Reg
      HD6309_setcc(HD6309_getcc() ^ (1 << Dest));
      break;
    }
  }

  instance->CycleCounter += instance->NatEmuCycles76;
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
      PUR(postbyte) ^= (1 << Dest);
      break;

    case 2: // CC Reg
      HD6309_setcc(HD6309_getcc() ^ (1 << Dest));
      break;
    }
  }

  instance->CycleCounter += instance->NatEmuCycles76;
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
      PUR(postbyte) |= (1 << Dest);
      break;

    case 2: // CC Reg
      HD6309_setcc(HD6309_getcc() | (1 << Dest));
      break;
    }
  }
  else
  {
    switch (postbyte)
    {
    case 0: // A Reg
    case 1: // B Reg
      PUR(postbyte) &= ~(1 << Dest);
      break;

    case 2: // CC Reg
      HD6309_setcc(HD6309_getcc() & ~(1 << Dest));
      break;
    }
  }

  instance->CycleCounter += instance->NatEmuCycles76;
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
    postbyte = PUR(postbyte);
    break;

  case 2: // CC Reg
    postbyte = HD6309_getcc();
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
  instance->CycleCounter += instance->NatEmuCycles87;
}

void Tfm1(void)
{ //1138 TFM R+,R+ 6309
  if (W_REG == 0)
  {
    instance->CycleCounter += 6;
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

  temp8 = MemRead8(PXF(Source));
  MemWrite8(temp8, PXF(Dest));
  (PXF(Dest))++;
  (PXF(Source))++;
  W_REG--;
  instance->CycleCounter += 3;
  PC_REG -= 2;
}

void Tfm2(void)
{ //1139 TFM R-,R- Phase 3 6309
  if (W_REG == 0)
  {
    instance->CycleCounter += 6;
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

  temp8 = MemRead8(PXF(Source));
  MemWrite8(temp8, PXF(Dest));
  (PXF(Dest))--;
  (PXF(Source))--;
  W_REG--;
  instance->CycleCounter += 3;
  PC_REG -= 2;
}

void Tfm3(void)
{ //113A 6309 TFM R+,R 6309
  if (W_REG == 0)
  {
    instance->CycleCounter += 6;
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

  temp8 = MemRead8(PXF(Source));
  MemWrite8(temp8, PXF(Dest));
  (PXF(Source))++;
  W_REG--;
  PC_REG -= 2; //Hit the same instruction on the next loop if not done copying
  instance->CycleCounter += 3;
}

void Tfm4(void)
{ //113B TFM R,R+ 6309 
  if (W_REG == 0)
  {
    instance->CycleCounter += 6;
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

  temp8 = MemRead8(PXF(Source));
  MemWrite8(temp8, PXF(Dest));
  (PXF(Dest))++;
  W_REG--;
  PC_REG -= 2; //Hit the same instruction on the next loop if not done copying
  instance->CycleCounter += 3;
}

void Bitmd_M(void)
{ //113C  6309
  postbyte = MemRead8(PC_REG++) & 0xC0;
  temp8 = HD6309_getmd() & postbyte;
  CC_Z = ZTEST(temp8);

  if (temp8 & 0x80) MD_ZERODIV = 0;
  if (temp8 & 0x40) MD_ILLEGAL = 0;

  instance->CycleCounter += 4;
}

void Ldmd_M(void)
{ //113D DONE 6309
  instance->mdbits = MemRead8(PC_REG++) & 0x03;
  HD6309_setmd(instance->mdbits);
  instance->CycleCounter += 5;
}

void Swi3_I(void)
{ //113F
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
    instance->CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(HD6309_getcc(), --S_REG);
  PC_REG = MemRead16(VSWI3);
  instance->CycleCounter += 20;
}

void Come_I(void)
{ //1143 6309 Untested
  E_REG = 0xFF - E_REG;
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_C = 1;
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Dece_I(void)
{ //114A 6309
  E_REG--;
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = E_REG == 0x7F;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Ince_I(void)
{ //114C 6309
  E_REG++;
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = E_REG == 0x80;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Tste_I(void)
{ //114D 6309 Untested TESTED NITRO
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Clre_I(void)
{ //114F 6309
  E_REG = 0;
  CC_C = 0;
  CC_V = 0;
  CC_N = 0;
  CC_Z = 1;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Comf_I(void)
{ //1153 6309 Untested
  F_REG = 0xFF - F_REG;
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_C = 1;
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Decf_I(void)
{ //115A 6309
  F_REG--;
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = F_REG == 0x7F;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Incf_I(void)
{ //115C 6309 Untested
  F_REG++;
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = F_REG == 0x80;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Tstf_I(void)
{ //115D 6309
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Clrf_I(void)
{ //115F 6309 Untested wcreate
  F_REG = 0;
  CC_C = 0;
  CC_V = 0;
  CC_N = 0;
  CC_Z = 1;
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Sube_M(void)
{ //1180 6309 Untested
  postbyte = MemRead8(PC_REG++);
  temp16 = E_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  instance->CycleCounter += 3;
}

void Cmpe_M(void)
{ //1181 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = E_REG - postbyte;
  CC_C = temp8 > E_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, E_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += 3;
}

void Cmpu_M(void)
{ //1183
  postword = IMMADDRESS(PC_REG);
  temp16 = U_REG - postword;
  CC_C = temp16 > U_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, U_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Lde_M(void)
{ //1186 6309
  E_REG = MemRead8(PC_REG++);
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = 0;
  instance->CycleCounter += 3;
}

void Adde_M(void)
{ //118B 6309
  postbyte = MemRead8(PC_REG++);
  temp16 = E_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((E_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  CC_N = NTEST8(E_REG);
  CC_Z = ZTEST(E_REG);
  instance->CycleCounter += 3;
}

void Cmps_M(void)
{ //118C
  postword = IMMADDRESS(PC_REG);
  temp16 = S_REG - postword;
  CC_C = temp16 > S_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, S_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Divd_M(void)
{ //118D 6309
  postbyte = MemRead8(PC_REG++);

  if (postbyte == 0)
  {
    instance->CycleCounter += 3;
    DivbyZero();
    return;
  }

  postword = D_REG;
  stemp16 = (signed short)postword / (signed char)postbyte;

  if ((stemp16 > 255) || (stemp16 < -256)) //Abort
  {
    CC_V = 1;
    CC_N = 0;
    CC_Z = 0;
    CC_C = 0;
    instance->CycleCounter += 17;
    return;
  }

  A_REG = (unsigned char)((signed short)postword % (signed char)postbyte);
  B_REG = (unsigned char)stemp16;

  if ((stemp16 > 127) || (stemp16 < -128))
  {
    CC_V = 1;
    CC_N = 1;
  }
  else
  {
    CC_Z = ZTEST(B_REG);
    CC_N = NTEST8(B_REG);
    CC_V = 0;
  }

  CC_C = B_REG & 1;
  instance->CycleCounter += 25;
}

void Divq_M(void)
{ //118E 6309
  postword = MemRead16(PC_REG);
  PC_REG += 2;

  if (postword == 0)
  {
    instance->CycleCounter += 4;
    DivbyZero();
    return;
  }

  temp32 = Q_REG;
  stemp32 = (signed int)temp32 / (signed short int)postword;

  if ((stemp32 > 65535) || (stemp32 < -65536)) //Abort
  {
    CC_V = 1;
    CC_N = 0;
    CC_Z = 0;
    CC_C = 0;
    instance->CycleCounter += 34 - 21;
    return;
  }

  D_REG = (unsigned short)((signed int)temp32 % (signed short int)postword);
  W_REG = stemp32;

  if ((stemp16 > 32767) || (stemp16 < -32768))
  {
    CC_V = 1;
    CC_N = 1;
  }
  else
  {
    CC_Z = ZTEST(W_REG);
    CC_N = NTEST16(W_REG);
    CC_V = 0;
  }

  CC_C = B_REG & 1;
  instance->CycleCounter += 34;
}

void Muld_M(void)
{ //118F Phase 5 6309
  Q_REG = (signed short)D_REG * (signed short)IMMADDRESS(PC_REG);
  CC_C = 0;
  CC_Z = ZTEST(Q_REG);
  CC_V = 0;
  CC_N = NTEST32(Q_REG);
  PC_REG += 2;
  instance->CycleCounter += 28;
}

void Sube_D(void)
{ //1190 6309 Untested HERE
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = E_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmpe_D(void)
{ //1191 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp8 = E_REG - postbyte;
  CC_C = temp8 > E_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, E_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmpu_D(void)
{ //1193
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = U_REG - postword;
  CC_C = temp16 > U_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, U_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Lde_D(void)
{ //1196 6309
  E_REG = MemRead8(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Ste_D(void)
{ //1197 Phase 5 6309
  MemWrite8(E_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Adde_D(void)
{ //119B 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = E_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((E_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  CC_N = NTEST8(E_REG);
  CC_Z = ZTEST(E_REG);
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmps_D(void)
{ //119C
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = S_REG - postword;
  CC_C = temp16 > S_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, S_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Divd_D(void)
{ //119D 6309 02292008
  postbyte = MemRead8(DPADDRESS(PC_REG++));

  if (postbyte == 0)
  {
    instance->CycleCounter += 3;
    DivbyZero();
    return;
  }

  postword = D_REG;
  stemp16 = (signed short)postword / (signed char)postbyte;

  if ((stemp16 > 255) || (stemp16 < -256)) //Abort
  {
    CC_V = 1;
    CC_N = 0;
    CC_Z = 0;
    CC_C = 0;
    instance->CycleCounter += 19;
    return;
  }

  A_REG = (unsigned char)((signed short)postword % (signed char)postbyte);
  B_REG = (unsigned char)stemp16;

  if ((stemp16 > 127) || (stemp16 < -128))
  {
    CC_V = 1;
    CC_N = 1;
  }
  else
  {
    CC_Z = ZTEST(B_REG);
    CC_N = NTEST8(B_REG);
    CC_V = 0;
  }

  CC_C = B_REG & 1;
  instance->CycleCounter += 27;
}

void Divq_D(void)
{ //119E 6309
  postword = MemRead16(DPADDRESS(PC_REG++));

  if (postword == 0)
  {
    instance->CycleCounter += 4;
    DivbyZero();
    return;
  }

  temp32 = Q_REG;
  stemp32 = (signed int)temp32 / (signed short int)postword;

  if ((stemp32 > 65535) || (stemp32 < -65536)) //Abort
  {
    CC_V = 1;
    CC_N = 0;
    CC_Z = 0;
    CC_C = 0;
    instance->CycleCounter += instance->NatEmuCycles3635 - 21;
    return;
  }

  D_REG = (unsigned short)((signed int)temp32 % (signed short int)postword);
  W_REG = stemp32;

  if ((stemp16 > 32767) || (stemp32 < -32768))
  {
    CC_V = 1;
    CC_N = 1;
  }
  else
  {
    CC_Z = ZTEST(W_REG);
    CC_N = NTEST16(W_REG);
    CC_V = 0;
  }

  CC_C = B_REG & 1;
  instance->CycleCounter += instance->NatEmuCycles3635;
}

void Muld_D(void)
{ //119F 6309 02292008
  Q_REG = (signed short)D_REG * (signed short)MemRead16(DPADDRESS(PC_REG++));
  CC_C = 0;
  CC_Z = ZTEST(Q_REG);
  CC_V = 0;
  CC_N = NTEST32(Q_REG);
  instance->CycleCounter += instance->NatEmuCycles3029;
}

void Sube_X(void)
{ //11A0 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = E_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  instance->CycleCounter += 5;
}

void Cmpe_X(void)
{ //11A1 6309
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp8 = E_REG - postbyte;
  CC_C = temp8 > E_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, E_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += 5;
}

void Cmpu_X(void)
{ //11A3
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = U_REG - postword;
  CC_C = temp16 > U_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, U_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Lde_X(void)
{ //11A6 6309
  E_REG = MemRead8(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Ste_X(void)
{ //11A7 6309
  MemWrite8(E_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Adde_X(void)
{ //11AB 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = E_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((E_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  CC_N = NTEST8(E_REG);
  CC_Z = ZTEST(E_REG);
  instance->CycleCounter += 5;
}

void Cmps_X(void)
{  //11AC
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = S_REG - postword;
  CC_C = temp16 > S_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, S_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Divd_X(void)
{ //11AD wcreate  6309
  postbyte = MemRead8(INDADDRESS(PC_REG++));

  if (postbyte == 0)
  {
    instance->CycleCounter += 3;
    DivbyZero();
    return;
  }

  postword = D_REG;
  stemp16 = (signed short)postword / (signed char)postbyte;

  if ((stemp16 > 255) || (stemp16 < -256)) //Abort
  {
    CC_V = 1;
    CC_N = 0;
    CC_Z = 0;
    CC_C = 0;
    instance->CycleCounter += 19;
    return;
  }

  A_REG = (unsigned char)((signed short)postword % (signed char)postbyte);
  B_REG = (unsigned char)stemp16;

  if ((stemp16 > 127) || (stemp16 < -128))
  {
    CC_V = 1;
    CC_N = 1;
  }
  else
  {
    CC_Z = ZTEST(B_REG);
    CC_N = NTEST8(B_REG);
    CC_V = 0;
  }
  CC_C = B_REG & 1;
  instance->CycleCounter += 27;
}

void Divq_X(void)
{ //11AE Phase 5 6309 CHECK
  postword = MemRead16(INDADDRESS(PC_REG++));

  if (postword == 0)
  {
    instance->CycleCounter += 4;
    DivbyZero();
    return;
  }

  temp32 = Q_REG;
  stemp32 = (signed int)temp32 / (signed short int)postword;

  if ((stemp32 > 65535) || (stemp32 < -65536)) //Abort
  {
    CC_V = 1;
    CC_N = 0;
    CC_Z = 0;
    CC_C = 0;
    instance->CycleCounter += instance->NatEmuCycles3635 - 21;
    return;
  }

  D_REG = (unsigned short)((signed int)temp32 % (signed short int)postword);
  W_REG = stemp32;

  if ((stemp16 > 32767) || (stemp16 < -32768))
  {
    CC_V = 1;
    CC_N = 1;
  }
  else
  {
    CC_Z = ZTEST(W_REG);
    CC_N = NTEST16(W_REG);
    CC_V = 0;
  }

  CC_C = B_REG & 1;
  instance->CycleCounter += instance->NatEmuCycles3635;
}

void Muld_X(void)
{ //11AF 6309 CHECK
  Q_REG = (signed short)D_REG * (signed short)MemRead16(INDADDRESS(PC_REG++));
  CC_C = 0;
  CC_Z = ZTEST(Q_REG);
  CC_V = 0;
  CC_N = NTEST32(Q_REG);
  instance->CycleCounter += 30;
}

void Sube_E(void)
{ //11B0 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = E_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Cmpe_E(void)
{ //11B1 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp8 = E_REG - postbyte;
  CC_C = temp8 > E_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, E_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Cmpu_E(void)
{ //11B3
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = U_REG - postword;
  CC_C = temp16 > U_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, U_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Lde_E(void)
{ //11B6 6309
  E_REG = MemRead8(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Ste_E(void)
{ //11B7 6309
  MemWrite8(E_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(E_REG);
  CC_N = NTEST8(E_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Adde_E(void)
{ //11BB 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = E_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((E_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, E_REG);
  E_REG = (unsigned char)temp16;
  CC_N = NTEST8(E_REG);
  CC_Z = ZTEST(E_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Cmps_E(void)
{ //11BC
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = S_REG - postword;
  CC_C = temp16 > S_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, S_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles86;
}

void Divd_E(void)
{ //11BD 6309 02292008 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  PC_REG += 2;

  if (postbyte == 0)
  {
    instance->CycleCounter += 3;
    DivbyZero();
    return;
  }

  postword = D_REG;
  stemp16 = (signed short)postword / (signed char)postbyte;

  if ((stemp16 > 255) || (stemp16 < -256)) //Abort
  {
    CC_V = 1;
    CC_N = 0;
    CC_Z = 0;
    CC_C = 0;
    instance->CycleCounter += 17;
    return;
  }

  A_REG = (unsigned char)((signed short)postword % (signed char)postbyte);
  B_REG = (unsigned char)stemp16;

  if ((stemp16 > 127) || (stemp16 < -128))
  {
    CC_V = 1;
    CC_N = 1;
  }
  else
  {
    CC_Z = ZTEST(B_REG);
    CC_N = NTEST8(B_REG);
    CC_V = 0;
  }
  CC_C = B_REG & 1;
  instance->CycleCounter += 25;
}

void Divq_E(void)
{ //11BE Phase 5 6309 CHECK
  postword = MemRead16(IMMADDRESS(PC_REG));
  PC_REG += 2;

  if (postword == 0)
  {
    instance->CycleCounter += 4;
    DivbyZero();
    return;
  }

  temp32 = Q_REG;
  stemp32 = (signed int)temp32 / (signed short int)postword;

  if ((stemp32 > 65535) || (stemp32 < -65536)) //Abort
  {
    CC_V = 1;
    CC_N = 0;
    CC_Z = 0;
    CC_C = 0;
    instance->CycleCounter += instance->NatEmuCycles3635 - 21;
    return;
  }

  D_REG = (unsigned short)((signed int)temp32 % (signed short int)postword);
  W_REG = stemp32;

  if ((stemp16 > 32767) || (stemp16 < -32768))
  {
    CC_V = 1;
    CC_N = 1;
  }
  else
  {
    CC_Z = ZTEST(W_REG);
    CC_N = NTEST16(W_REG);
    CC_V = 0;
  }
  CC_C = B_REG & 1;
  instance->CycleCounter += instance->NatEmuCycles3635;
}

void Muld_E(void)
{ //11BF 6309
  Q_REG = (signed short)D_REG * (signed short)MemRead16(IMMADDRESS(PC_REG));
  PC_REG += 2;
  CC_C = 0;
  CC_Z = ZTEST(Q_REG);
  CC_V = 0;
  CC_N = NTEST32(Q_REG);
  instance->CycleCounter += instance->NatEmuCycles3130;
}

void Subf_M(void)
{ //11C0 6309 Untested
  postbyte = MemRead8(PC_REG++);
  temp16 = F_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  instance->CycleCounter += 3;
}

void Cmpf_M(void)
{ //11C1 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = F_REG - postbyte;
  CC_C = temp8 > F_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, F_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += 3;
}

void Ldf_M(void)
{ //11C6 6309
  F_REG = MemRead8(PC_REG++);
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = 0;
  instance->CycleCounter += 3;
}

void Addf_M(void)
{ //11CB 6309 Untested
  postbyte = MemRead8(PC_REG++);
  temp16 = F_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((F_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  CC_N = NTEST8(F_REG);
  CC_Z = ZTEST(F_REG);
  instance->CycleCounter += 3;
}

void Subf_D(void)
{ //11D0 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = F_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmpf_D(void)
{ //11D1 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp8 = F_REG - postbyte;
  CC_C = temp8 > F_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, F_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Ldf_D(void)
{ //11D6 6309 Untested wcreate
  F_REG = MemRead8(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Stf_D(void)
{ //11D7 Phase 5 6309
  MemWrite8(F_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Addf_D(void)
{ //11DB 6309 Untested
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = F_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((F_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  CC_N = NTEST8(F_REG);
  CC_Z = ZTEST(F_REG);
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Subf_X(void)
{ //11E0 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = F_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  instance->CycleCounter += 5;
}

void Cmpf_X(void)
{ //11E1 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp8 = F_REG - postbyte;
  CC_C = temp8 > F_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, F_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += 5;
}

void Ldf_X(void)
{ //11E6 6309
  F_REG = MemRead8(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Stf_X(void)
{ //11E7 Phase 5 6309
  MemWrite8(F_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Addf_X(void)
{ //11EB 6309 Untested
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = F_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((F_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  CC_N = NTEST8(F_REG);
  CC_Z = ZTEST(F_REG);
  instance->CycleCounter += 5;
}

void Subf_E(void)
{ //11F0 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = F_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Cmpf_E(void)
{ //11F1 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp8 = F_REG - postbyte;
  CC_C = temp8 > F_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, F_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Ldf_E(void)
{ //11F6 6309
  F_REG = MemRead8(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Stf_E(void)
{ //11F7 Phase 5 6309
  MemWrite8(F_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(F_REG);
  CC_N = NTEST8(F_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Addf_E(void)
{ //11FB 6309 Untested
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = F_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((F_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, F_REG);
  F_REG = (unsigned char)temp16;
  CC_N = NTEST8(F_REG);
  CC_Z = ZTEST(F_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Nop_I(void)
{	//12
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Sync_I(void)
{ //13
  instance->CycleCounter = instance->gCycleFor;
  instance->SyncWaiting = 1;
}

void Sexw_I(void)
{ //14 6309 CHECK
  if (W_REG & 32768)
    D_REG = 0xFFFF;
  else
    D_REG = 0;

  CC_Z = ZTEST(Q_REG);
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += 4;
}

void Lbra_R(void)
{ //16
  *spostword = IMMADDRESS(PC_REG);
  PC_REG += 2;
  PC_REG += *spostword;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Lbsr_R(void)
{ //17
  *spostword = IMMADDRESS(PC_REG);
  PC_REG += 2;
  S_REG--;
  MemWrite8(PC_L, S_REG--);
  MemWrite8(PC_H, S_REG);
  PC_REG += *spostword;
  instance->CycleCounter += instance->NatEmuCycles97;
}

void Daa_I(void)
{ //19
  static unsigned char msn, lsn;

  msn = (A_REG & 0xF0);
  lsn = (A_REG & 0xF);
  temp8 = 0;

  if (CC_H || (lsn > 9))
    temp8 |= 0x06;

  if ((msn > 0x80) && (lsn > 9))
    temp8 |= 0x60;

  if ((msn > 0x90) || CC_C)
    temp8 |= 0x60;

  temp16 = A_REG + temp8;
  CC_C |= ((temp16 & 0x100) >> 8);
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Orcc_M(void)
{ //1A
  postbyte = MemRead8(PC_REG++);
  temp8 = HD6309_getcc();
  temp8 = (temp8 | postbyte);
  HD6309_setcc(temp8);
  instance->CycleCounter += instance->NatEmuCycles32;
}

void Andcc_M(void)
{ //1C
  postbyte = MemRead8(PC_REG++);
  temp8 = HD6309_getcc();
  temp8 = (temp8 & postbyte);
  HD6309_setcc(temp8);
  instance->CycleCounter += 3;
}

void Sex_I(void)
{ //1D
  A_REG = 0 - (B_REG >> 7);
  CC_Z = ZTEST(D_REG);
  CC_N = D_REG >> 15;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Exg_M(void)
{ //1E
  postbyte = MemRead8(PC_REG++);
  Source = postbyte >> 4;
  Dest = postbyte & 15;

  instance->ccbits = HD6309_getcc();

  if ((Source & 0x08) == (Dest & 0x08)) //Verify like size registers
  {
    if (Dest & 0x08) //8 bit EXG
    {
      Source &= 0x07;
      Dest &= 0x07;
      temp8 = (PUR(Source));
      PUR(Source) = (PUR(Dest));
      PUR(Dest) = temp8;
      O_REG = 0;
    }
    else // 16 bit EXG
    {
      Source &= 0x07;
      Dest &= 0x07;
      temp16 = (PXF(Source));
      (PXF(Source)) = (PXF(Dest));
      (PXF(Dest)) = temp16;
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
      (PXF(Dest)) = 0; // Source is Zero reg. Just zero the Destination.
      break;

    case 0x00: // A
    case 0x03: // DP
    case 0x06: // E
      temp8 = PUR(Source);
      temp16 = (temp8 << 8) | temp8;
      (PUR(Source)) = (PXF(Dest)) >> 8; // A, DP, E get high byte of 16 bit Dest
      (PXF(Dest)) = temp16; // Place 8 bit source in both halves of 16 bit Dest
      break;

    case 0x01: // B
    case 0x02: // CC
    case 0x07: // F
      temp8 = PUR(Source);
      temp16 = (temp8 << 8) | temp8;
      (PUR(Source)) = (PXF(Dest)) & 0xFF; // B, CC, F get low byte of 16 bit Dest
      (PXF(Dest)) = temp16; // Place 8 bit source in both halves of 16 bit Dest
      break;
    }
  }

  HD6309_setcc(instance->ccbits);
  instance->CycleCounter += instance->NatEmuCycles85;
}

void Tfr_M(void)
{ //1F
  postbyte = MemRead8(PC_REG++);
  Source = postbyte >> 4;
  Dest = postbyte & 15;

  if (Dest < 8)
  {
    if (Source < 8) {
      PXF(Dest) = PXF(Source);
    }
    else {
      PXF(Dest) = (PUR(Source & 7) << 8) | PUR(Source & 7);
    }
  }
  else
  {
    instance->ccbits = HD6309_getcc();
    Dest &= 7;

    if (Source < 8)
      switch (Dest)
      {
      case 0:  // A
      case 3: // DP
      case 6: // E
        PUR(Dest) = PXF(Source) >> 8;
        break;
      case 1:  // B
      case 2: // CC
      case 7: // F
        PUR(Dest) = PXF(Source) & 0xFF;
        break;
      }
    else {
      PUR(Dest) = PUR(Source & 7);
    }

    O_REG = 0;
    HD6309_setcc(instance->ccbits);
  }

  instance->CycleCounter += instance->NatEmuCycles64;
}

void Bra_R(void)
{ //20
  *spostbyte = MemRead8(PC_REG++);
  PC_REG += *spostbyte;
  instance->CycleCounter += 3;
}

void Brn_R(void)
{ //21
  instance->CycleCounter += 3;
  PC_REG++;
}

void Bhi_R(void)
{ //22
  if (!(CC_C | CC_Z))
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Bls_R(void)
{ //23
  if (CC_C | CC_Z)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Bhs_R(void)
{ //24
  if (!CC_C)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Blo_R(void)
{ //25
  if (CC_C)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Bne_R(void)
{ //26
  if (!CC_Z)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Beq_R(void)
{ //27
  if (CC_Z)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Bvc_R(void)
{ //28
  if (!CC_V)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Bvs_R(void)
{ //29
  if (CC_V)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Bpl_R(void)
{ //2A
  if (!CC_N)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Bmi_R(void)
{ //2B
  if (CC_N)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Bge_R(void)
{ //2C
  if (!(CC_N ^ CC_V))
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Blt_R(void)
{ //2D
  if (CC_V ^ CC_N)
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Bgt_R(void)
{ //2E
  if (!(CC_Z | (CC_N ^ CC_V)))
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Ble_R(void)
{ //2F
  if (CC_Z | (CC_N ^ CC_V))
    PC_REG += (signed char)MemRead8(PC_REG);

  PC_REG++;
  instance->CycleCounter += 3;
}

void Leax_X(void)
{ //30
  X_REG = INDADDRESS(PC_REG++);
  CC_Z = ZTEST(X_REG);
  instance->CycleCounter += 4;
}

void Leay_X(void)
{ //31
  Y_REG = INDADDRESS(PC_REG++);
  CC_Z = ZTEST(Y_REG);
  instance->CycleCounter += 4;
}

void Leas_X(void)
{ //32
  S_REG = INDADDRESS(PC_REG++);
  instance->CycleCounter += 4;
}

void Leau_X(void)
{ //33
  U_REG = INDADDRESS(PC_REG++);
  instance->CycleCounter += 4;
}

void Pshs_M(void)
{ //34
  postbyte = MemRead8(PC_REG++);

  if (postbyte & 0x80)
  {
    MemWrite8(PC_L, --S_REG);
    MemWrite8(PC_H, --S_REG);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x40)
  {
    MemWrite8(U_L, --S_REG);
    MemWrite8(U_H, --S_REG);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x20)
  {
    MemWrite8(Y_L, --S_REG);
    MemWrite8(Y_H, --S_REG);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x10)
  {
    MemWrite8(X_L, --S_REG);
    MemWrite8(X_H, --S_REG);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x08)
  {
    MemWrite8(DPA, --S_REG);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x04)
  {
    MemWrite8(B_REG, --S_REG);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x02)
  {
    MemWrite8(A_REG, --S_REG);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x01)
  {
    MemWrite8(HD6309_getcc(), --S_REG);
    instance->CycleCounter += 1;
  }

  instance->CycleCounter += instance->NatEmuCycles54;
}

void Puls_M(void)
{ //35
  postbyte = MemRead8(PC_REG++);

  if (postbyte & 0x01)
  {
    HD6309_setcc(MemRead8(S_REG++));
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x02)
  {
    A_REG = MemRead8(S_REG++);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x04)
  {
    B_REG = MemRead8(S_REG++);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x08)
  {
    DPA = MemRead8(S_REG++);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x10)
  {
    X_H = MemRead8(S_REG++);
    X_L = MemRead8(S_REG++);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x20)
  {
    Y_H = MemRead8(S_REG++);
    Y_L = MemRead8(S_REG++);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x40)
  {
    U_H = MemRead8(S_REG++);
    U_L = MemRead8(S_REG++);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x80)
  {
    PC_H = MemRead8(S_REG++);
    PC_L = MemRead8(S_REG++);
    instance->CycleCounter += 2;
  }

  instance->CycleCounter += instance->NatEmuCycles54;
}

void Pshu_M(void)
{ //36
  postbyte = MemRead8(PC_REG++);

  if (postbyte & 0x80)
  {
    MemWrite8(PC_L, --U_REG);
    MemWrite8(PC_H, --U_REG);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x40)
  {
    MemWrite8(S_L, --U_REG);
    MemWrite8(S_H, --U_REG);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x20)
  {
    MemWrite8(Y_L, --U_REG);
    MemWrite8(Y_H, --U_REG);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x10)
  {
    MemWrite8(X_L, --U_REG);
    MemWrite8(X_H, --U_REG);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x08)
  {
    MemWrite8(DPA, --U_REG);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x04)
  {
    MemWrite8(B_REG, --U_REG);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x02)
  {
    MemWrite8(A_REG, --U_REG);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x01)
  {
    MemWrite8(HD6309_getcc(), --U_REG);
    instance->CycleCounter += 1;
  }

  instance->CycleCounter += instance->NatEmuCycles54;
}

void Pulu_M(void)
{ //37
  postbyte = MemRead8(PC_REG++);

  if (postbyte & 0x01)
  {
    HD6309_setcc(MemRead8(U_REG++));
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x02)
  {
    A_REG = MemRead8(U_REG++);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x04)
  {
    B_REG = MemRead8(U_REG++);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x08)
  {
    DPA = MemRead8(U_REG++);
    instance->CycleCounter += 1;
  }

  if (postbyte & 0x10)
  {
    X_H = MemRead8(U_REG++);
    X_L = MemRead8(U_REG++);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x20)
  {
    Y_H = MemRead8(U_REG++);
    Y_L = MemRead8(U_REG++);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x40)
  {
    S_H = MemRead8(U_REG++);
    S_L = MemRead8(U_REG++);
    instance->CycleCounter += 2;
  }

  if (postbyte & 0x80)
  {
    PC_H = MemRead8(U_REG++);
    PC_L = MemRead8(U_REG++);
    instance->CycleCounter += 2;
  }

  instance->CycleCounter += instance->NatEmuCycles54;
}

void Rts_I(void)
{ //39
  PC_H = MemRead8(S_REG++);
  PC_L = MemRead8(S_REG++);
  instance->CycleCounter += instance->NatEmuCycles51;
}

void Abx_I(void)
{ //3A
  X_REG = X_REG + B_REG;
  instance->CycleCounter += instance->NatEmuCycles31;
}

void Rti_I(void)
{ //3B
  HD6309_setcc(MemRead8(S_REG++));
  instance->CycleCounter += 6;
  instance->InInterrupt = 0;

  if (CC_E)
  {
    A_REG = MemRead8(S_REG++);
    B_REG = MemRead8(S_REG++);

    if (MD_NATIVE6309)
    {
      (E_REG) = MemRead8(S_REG++);
      (F_REG) = MemRead8(S_REG++);
      instance->CycleCounter += 2;
    }

    DPA = MemRead8(S_REG++);
    X_H = MemRead8(S_REG++);
    X_L = MemRead8(S_REG++);
    Y_H = MemRead8(S_REG++);
    Y_L = MemRead8(S_REG++);
    U_H = MemRead8(S_REG++);
    U_L = MemRead8(S_REG++);
    instance->CycleCounter += 9;
  }

  PC_H = MemRead8(S_REG++);
  PC_L = MemRead8(S_REG++);
}

void Cwai_I(void)
{ //3C
  postbyte = MemRead8(PC_REG++);
  instance->ccbits = HD6309_getcc();
  instance->ccbits = instance->ccbits & postbyte;
  HD6309_setcc(instance->ccbits);
  instance->CycleCounter = instance->gCycleFor;
  instance->SyncWaiting = 1;
}

void Mul_I(void)
{ //3D
  D_REG = A_REG * B_REG;
  CC_C = B_REG > 0x7F;
  CC_Z = ZTEST(D_REG);
  instance->CycleCounter += instance->NatEmuCycles1110;
}

void Reset(void) // 3E
{	//Undocumented
  HD6309Reset();
}

void Swi1_I(void)
{ //3F
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
    instance->CycleCounter += 2;
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(HD6309_getcc(), --S_REG);
  PC_REG = MemRead16(VSWI);
  instance->CycleCounter += 19;
  CC_I = 1;
  CC_F = 1;
}

void Nega_I(void)
{ //40
  temp8 = 0 - A_REG;
  CC_C = temp8 > 0;
  CC_V = A_REG == 0x80; //CC_C ^ ((A_REG^temp8)>>7);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  A_REG = temp8;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Coma_I(void)
{ //43
  A_REG = 0xFF - A_REG;
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  CC_C = 1;
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Lsra_I(void)
{ //44
  CC_C = A_REG & 1;
  A_REG = A_REG >> 1;
  CC_Z = ZTEST(A_REG);
  CC_N = 0;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Rora_I(void)
{ //46
  postbyte = CC_C << 7;
  CC_C = A_REG & 1;
  A_REG = (A_REG >> 1) | postbyte;
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Asra_I(void)
{ //47
  CC_C = A_REG & 1;
  A_REG = (A_REG & 0x80) | (A_REG >> 1);
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Asla_I(void)
{ //48
  CC_C = A_REG > 0x7F;
  CC_V = CC_C ^ ((A_REG & 0x40) >> 6);
  A_REG = A_REG << 1;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Rola_I(void)
{ //49
  postbyte = CC_C;
  CC_C = A_REG > 0x7F;
  CC_V = CC_C ^ ((A_REG & 0x40) >> 6);
  A_REG = (A_REG << 1) | postbyte;
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Deca_I(void)
{ //4A
  A_REG--;
  CC_Z = ZTEST(A_REG);
  CC_V = A_REG == 0x7F;
  CC_N = NTEST8(A_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Inca_I(void)
{ //4C
  A_REG++;
  CC_Z = ZTEST(A_REG);
  CC_V = A_REG == 0x80;
  CC_N = NTEST8(A_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Tsta_I(void)
{ //4D
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Clra_I(void)
{ //4F
  A_REG = 0;
  CC_C = 0;
  CC_V = 0;
  CC_N = 0;
  CC_Z = 1;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Negb_I(void)
{ //50
  temp8 = 0 - B_REG;
  CC_C = temp8 > 0;
  CC_V = B_REG == 0x80;
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  B_REG = temp8;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Comb_I(void)
{ //53
  B_REG = 0xFF - B_REG;
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  CC_C = 1;
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Lsrb_I(void)
{ //54
  CC_C = B_REG & 1;
  B_REG = B_REG >> 1;
  CC_Z = ZTEST(B_REG);
  CC_N = 0;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Rorb_I(void)
{ //56
  postbyte = CC_C << 7;
  CC_C = B_REG & 1;
  B_REG = (B_REG >> 1) | postbyte;
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Asrb_I(void)
{ //57
  CC_C = B_REG & 1;
  B_REG = (B_REG & 0x80) | (B_REG >> 1);
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Aslb_I(void)
{ //58
  CC_C = B_REG > 0x7F;
  CC_V = CC_C ^ ((B_REG & 0x40) >> 6);
  B_REG = B_REG << 1;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Rolb_I(void)
{ //59
  postbyte = CC_C;
  CC_C = B_REG > 0x7F;
  CC_V = CC_C ^ ((B_REG & 0x40) >> 6);
  B_REG = (B_REG << 1) | postbyte;
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Decb_I(void)
{ //5A
  B_REG--;
  CC_Z = ZTEST(B_REG);
  CC_V = B_REG == 0x7F;
  CC_N = NTEST8(B_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Incb_I(void)
{ //5C
  B_REG++;
  CC_Z = ZTEST(B_REG);
  CC_V = B_REG == 0x80;
  CC_N = NTEST8(B_REG);
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Tstb_I(void)
{ //5D
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Clrb_I(void)
{ //5F
  B_REG = 0;
  CC_C = 0;
  CC_N = 0;
  CC_V = 0;
  CC_Z = 1;
  instance->CycleCounter += instance->NatEmuCycles21;
}

void Neg_X(void)
{ //60
  temp16 = INDADDRESS(PC_REG++);
  postbyte = MemRead8(temp16);
  temp8 = 0 - postbyte;
  CC_C = temp8 > 0;
  CC_V = postbyte == 0x80;
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += 6;
}

void Oim_X(void)
{ //61 6309 DONE
  postbyte = MemRead8(PC_REG++);
  temp16 = INDADDRESS(PC_REG++);
  postbyte |= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Aim_X(void)
{ //62 6309 Phase 2
  postbyte = MemRead8(PC_REG++);
  temp16 = INDADDRESS(PC_REG++);
  postbyte &= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  instance->CycleCounter += 6;
}

void Com_X(void)
{ //63
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  temp8 = 0xFF - temp8;
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  CC_V = 0;
  CC_C = 1;
  MemWrite8(temp8, temp16);
  instance->CycleCounter += 6;
}

void Lsr_X(void)
{ //64
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  CC_C = temp8 & 1;
  temp8 = temp8 >> 1;
  CC_Z = ZTEST(temp8);
  CC_N = 0;
  MemWrite8(temp8, temp16);
  instance->CycleCounter += 6;
}

void Eim_X(void)
{ //65 6309 Untested TESTED NITRO
  postbyte = MemRead8(PC_REG++);
  temp16 = INDADDRESS(PC_REG++);
  postbyte ^= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  instance->CycleCounter += 7;
}

void Ror_X(void)
{ //66
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  postbyte = CC_C << 7;
  CC_C = (temp8 & 1);
  temp8 = (temp8 >> 1) | postbyte;
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += 6;
}

void Asr_X(void)
{ //67
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  CC_C = temp8 & 1;
  temp8 = (temp8 & 0x80) | (temp8 >> 1);
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += 6;
}

void Asl_X(void)
{ //68 
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  CC_C = temp8 > 0x7F;
  CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
  temp8 = temp8 << 1;
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += 6;
}

void Rol_X(void)
{ //69
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  postbyte = CC_C;
  CC_C = temp8 > 0x7F;
  CC_V = (CC_C ^ ((temp8 & 0x40) >> 6));
  temp8 = ((temp8 << 1) | postbyte);
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += 6;
}

void Dec_X(void)
{ //6A
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  temp8--;
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  CC_V = (temp8 == 0x7F);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += 6;
}

void Tim_X(void)
{ //6B 6309
  postbyte = MemRead8(PC_REG++);
  temp8 = MemRead8(INDADDRESS(PC_REG++));
  postbyte &= temp8;
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  instance->CycleCounter += 7;
}

void Inc_X(void)
{ //6C
  temp16 = INDADDRESS(PC_REG++);
  temp8 = MemRead8(temp16);
  temp8++;
  CC_V = (temp8 == 0x80);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  instance->CycleCounter += 6;
}

void Tst_X(void)
{ //6D
  temp8 = MemRead8(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Jmp_X(void)
{ //6E
  PC_REG = INDADDRESS(PC_REG++);
  instance->CycleCounter += 3;
}

void Clr_X(void)
{ //6F
  MemWrite8(0, INDADDRESS(PC_REG++));
  CC_C = 0;
  CC_N = 0;
  CC_V = 0;
  CC_Z = 1;
  instance->CycleCounter += 6;
}

void Neg_E(void)
{ //70
  temp16 = IMMADDRESS(PC_REG);
  postbyte = MemRead8(temp16);
  temp8 = 0 - postbyte;
  CC_C = temp8 > 0;
  CC_V = postbyte == 0x80;
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Oim_E(void)
{ //71 6309 Phase 2
  postbyte = MemRead8(PC_REG++);
  temp16 = IMMADDRESS(PC_REG);
  postbyte |= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += 7;
}

void Aim_E(void)
{ //72 6309 Untested CHECK NITRO
  postbyte = MemRead8(PC_REG++);
  temp16 = IMMADDRESS(PC_REG);
  postbyte &= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += 7;
}

void Com_E(void)
{ //73
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  temp8 = 0xFF - temp8;
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  CC_C = 1;
  CC_V = 0;
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Lsr_E(void)
{  //74
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  CC_C = temp8 & 1;
  temp8 = temp8 >> 1;
  CC_Z = ZTEST(temp8);
  CC_N = 0;
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Eim_E(void)
{ //75 6309 Untested CHECK NITRO
  postbyte = MemRead8(PC_REG++);
  temp16 = IMMADDRESS(PC_REG);
  postbyte ^= MemRead8(temp16);
  MemWrite8(postbyte, temp16);
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += 7;
}

void Ror_E(void)
{ //76
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  postbyte = CC_C << 7;
  CC_C = temp8 & 1;
  temp8 = (temp8 >> 1) | postbyte;
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Asr_E(void)
{ //77
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  CC_C = temp8 & 1;
  temp8 = (temp8 & 0x80) | (temp8 >> 1);
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Asl_E(void)
{ //78 6309
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  CC_C = temp8 > 0x7F;
  CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
  temp8 = temp8 << 1;
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Rol_E(void)
{ //79
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  postbyte = CC_C;
  CC_C = temp8 > 0x7F;
  CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
  temp8 = ((temp8 << 1) | postbyte);
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Dec_E(void)
{ //7A
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  temp8--;
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  CC_V = temp8 == 0x7F;
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Tim_E(void)
{ //7B 6309 NITRO 
  postbyte = MemRead8(PC_REG++);
  temp16 = IMMADDRESS(PC_REG);
  postbyte &= MemRead8(temp16);
  CC_N = NTEST8(postbyte);
  CC_Z = ZTEST(postbyte);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += 7;
}

void Inc_E(void)
{ //7C
  temp16 = IMMADDRESS(PC_REG);
  temp8 = MemRead8(temp16);
  temp8++;
  CC_Z = ZTEST(temp8);
  CC_V = temp8 == 0x80;
  CC_N = NTEST8(temp8);
  MemWrite8(temp8, temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Tst_E(void)
{ //7D
  temp8 = MemRead8(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(temp8);
  CC_N = NTEST8(temp8);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Jmp_E(void)
{ //7E
  PC_REG = IMMADDRESS(PC_REG);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Clr_E(void)
{ //7F
  MemWrite8(0, IMMADDRESS(PC_REG));
  CC_C = 0;
  CC_N = 0;
  CC_V = 0;
  CC_Z = 1;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Suba_M(void)
{ //80
  postbyte = MemRead8(PC_REG++);
  temp16 = A_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  instance->CycleCounter += 2;
}

void Cmpa_M(void)
{ //81
  postbyte = MemRead8(PC_REG++);
  temp8 = A_REG - postbyte;
  CC_C = temp8 > A_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, A_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += 2;
}

void Sbca_M(void)
{  //82
  postbyte = MemRead8(PC_REG++);
  temp16 = A_REG - postbyte - CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += 2;
}

void Subd_M(void)
{ //83
  temp16 = IMMADDRESS(PC_REG);
  temp32 = D_REG - temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, D_REG);
  D_REG = temp32;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Anda_M(void)
{ //84
  A_REG = A_REG & MemRead8(PC_REG++);
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Bita_M(void)
{ //85
  temp8 = A_REG & MemRead8(PC_REG++);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Lda_M(void)
{ //86
  A_REG = MemRead8(PC_REG++);
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Eora_M(void)
{ //88
  A_REG = A_REG ^ MemRead8(PC_REG++);
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Adca_M(void)
{ //89
  postbyte = MemRead8(PC_REG++);
  temp16 = A_REG + postbyte + CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  CC_H = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += 2;
}

void Ora_M(void)
{ //8A
  A_REG = A_REG | MemRead8(PC_REG++);
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Adda_M(void)
{ //8B
  postbyte = MemRead8(PC_REG++);
  temp16 = A_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += 2;
}

void Cmpx_M(void)
{ //8C
  postword = IMMADDRESS(PC_REG);
  temp16 = X_REG - postword;
  CC_C = temp16 > X_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, X_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Bsr_R(void)
{ //8D
  *spostbyte = MemRead8(PC_REG++);
  S_REG--;
  MemWrite8(PC_L, S_REG--);
  MemWrite8(PC_H, S_REG);
  PC_REG += *spostbyte;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Ldx_M(void)
{ //8E
  X_REG = IMMADDRESS(PC_REG);
  CC_Z = ZTEST(X_REG);
  CC_N = NTEST16(X_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += 3;
}

void Suba_D(void)
{ //90
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = A_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Cmpa_D(void)
{ //91
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp8 = A_REG - postbyte;
  CC_C = temp8 > A_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, A_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Scba_D(void)
{ //92
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = A_REG - postbyte - CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Subd_D(void)
{ //93
  temp16 = MemRead16(DPADDRESS(PC_REG++));
  temp32 = D_REG - temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, D_REG);
  D_REG = temp32;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += instance->NatEmuCycles64;
}

void Anda_D(void)
{ //94
  A_REG = A_REG & MemRead8(DPADDRESS(PC_REG++));
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Bita_D(void)
{ //95
  temp8 = A_REG & MemRead8(DPADDRESS(PC_REG++));
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Lda_D(void)
{ //96
  A_REG = MemRead8(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Sta_D(void)
{ //97
  MemWrite8(A_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Eora_D(void)
{ //98
  A_REG = A_REG ^ MemRead8(DPADDRESS(PC_REG++));
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Adca_D(void)
{ //99
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = A_REG + postbyte + CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  CC_H = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Ora_D(void)
{ //9A
  A_REG = A_REG | MemRead8(DPADDRESS(PC_REG++));
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Adda_D(void)
{ //9B
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = A_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Cmpx_D(void)
{ //9C
  postword = MemRead16(DPADDRESS(PC_REG++));
  temp16 = X_REG - postword;
  CC_C = temp16 > X_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, X_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles64;
}

void Jsr_D(void)
{ //9D
  temp16 = DPADDRESS(PC_REG++);
  S_REG--;
  MemWrite8(PC_L, S_REG--);
  MemWrite8(PC_H, S_REG);
  PC_REG = temp16;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Ldx_D(void)
{ //9E
  X_REG = MemRead16(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(X_REG);
  CC_N = NTEST16(X_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Stx_D(void)
{ //9F
  MemWrite16(X_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(X_REG);
  CC_N = NTEST16(X_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Suba_X(void)
{ //A0
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = A_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  instance->CycleCounter += 4;
}

void Cmpa_X(void)
{ //A1
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp8 = A_REG - postbyte;
  CC_C = temp8 > A_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, A_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += 4;
}

void Sbca_X(void)
{ //A2
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = A_REG - postbyte - CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += 4;
}

void Subd_X(void)
{ //A3
  temp16 = MemRead16(INDADDRESS(PC_REG++));
  temp32 = D_REG - temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, D_REG);
  D_REG = temp32;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Anda_X(void)
{ //A4
  A_REG = A_REG & MemRead8(INDADDRESS(PC_REG++));
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Bita_X(void)
{  //A5
  temp8 = A_REG & MemRead8(INDADDRESS(PC_REG++));
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Lda_X(void)
{ //A6
  A_REG = MemRead8(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Sta_X(void)
{ //A7
  MemWrite8(A_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Eora_X(void)
{ //A8
  A_REG = A_REG ^ MemRead8(INDADDRESS(PC_REG++));
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Adca_X(void)
{ //A9	
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = A_REG + postbyte + CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  CC_H = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += 4;
}

void Ora_X(void)
{ //AA
  A_REG = A_REG | MemRead8(INDADDRESS(PC_REG++));
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Adda_X(void)
{ //AB
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = A_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  instance->CycleCounter += 4;
}

void Cmpx_X(void)
{ //AC
  postword = MemRead16(INDADDRESS(PC_REG++));
  temp16 = X_REG - postword;
  CC_C = temp16 > X_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, X_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Jsr_X(void)
{ //AD
  temp16 = INDADDRESS(PC_REG++);
  S_REG--;
  MemWrite8(PC_L, S_REG--);
  MemWrite8(PC_H, S_REG);
  PC_REG = temp16;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Ldx_X(void)
{ //AE
  X_REG = MemRead16(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(X_REG);
  CC_N = NTEST16(X_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Stx_X(void)
{ //AF
  MemWrite16(X_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(X_REG);
  CC_N = NTEST16(X_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Suba_E(void)
{ //B0
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = A_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmpa_E(void)
{ //B1
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp8 = A_REG - postbyte;
  CC_C = temp8 > A_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, A_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Sbca_E(void)
{ //B2
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = A_REG - postbyte - CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Subd_E(void)
{ //B3
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = D_REG - temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, D_REG);
  D_REG = temp32;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Anda_E(void)
{ //B4
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  A_REG = A_REG & postbyte;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Bita_E(void)
{ //B5
  temp8 = A_REG & MemRead8(IMMADDRESS(PC_REG));
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Lda_E(void)
{ //B6
  A_REG = MemRead8(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Sta_E(void)
{ //B7
  MemWrite8(A_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(A_REG);
  CC_N = NTEST8(A_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Eora_E(void)
{  //B8
  A_REG = A_REG ^ MemRead8(IMMADDRESS(PC_REG));
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Adca_E(void)
{ //B9
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = A_REG + postbyte + CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  CC_H = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Ora_E(void)
{ //BA
  A_REG = A_REG | MemRead8(IMMADDRESS(PC_REG));
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Adda_E(void)
{ //BB
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = A_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, A_REG);
  A_REG = (unsigned char)temp16;
  CC_N = NTEST8(A_REG);
  CC_Z = ZTEST(A_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmpx_E(void)
{ //BC
  postword = MemRead16(IMMADDRESS(PC_REG));
  temp16 = X_REG - postword;
  CC_C = temp16 > X_REG;
  CC_V = OVERFLOW16(CC_C, postword, temp16, X_REG);
  CC_N = NTEST16(temp16);
  CC_Z = ZTEST(temp16);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles75;
}

void Bsr_E(void)
{ //BD
  postword = IMMADDRESS(PC_REG);
  PC_REG += 2;
  S_REG--;
  MemWrite8(PC_L, S_REG--);
  MemWrite8(PC_H, S_REG);
  PC_REG = postword;
  instance->CycleCounter += instance->NatEmuCycles87;
}

void Ldx_E(void)
{ //BE
  X_REG = MemRead16(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(X_REG);
  CC_N = NTEST16(X_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Stx_E(void)
{ //BF
  MemWrite16(X_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(X_REG);
  CC_N = NTEST16(X_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Subb_M(void)
{ //C0
  postbyte = MemRead8(PC_REG++);
  temp16 = B_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  instance->CycleCounter += 2;
}

void Cmpb_M(void)
{ //C1
  postbyte = MemRead8(PC_REG++);
  temp8 = B_REG - postbyte;
  CC_C = temp8 > B_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, B_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += 2;
}

void Sbcb_M(void)
{ //C2
  postbyte = MemRead8(PC_REG++);
  temp16 = B_REG - postbyte - CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += 2;
}

void Addd_M(void)
{ //C3
  temp16 = IMMADDRESS(PC_REG);
  temp32 = D_REG + temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, D_REG);
  D_REG = temp32;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Andb_M(void)
{ //C4 LOOK
  B_REG = B_REG & MemRead8(PC_REG++);
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Bitb_M(void)
{ //C5
  temp8 = B_REG & MemRead8(PC_REG++);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Ldb_M(void)
{ //C6
  B_REG = MemRead8(PC_REG++);
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Eorb_M(void)
{ //C8
  B_REG = B_REG ^ MemRead8(PC_REG++);
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Adcb_M(void)
{ //C9
  postbyte = MemRead8(PC_REG++);
  temp16 = B_REG + postbyte + CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  CC_H = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += 2;
}

void Orb_M(void)
{ //CA
  B_REG = B_REG | MemRead8(PC_REG++);
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  instance->CycleCounter += 2;
}

void Addb_M(void)
{ //CB
  postbyte = MemRead8(PC_REG++);
  temp16 = B_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += 2;
}

void Ldd_M(void)
{ //CC
  D_REG = IMMADDRESS(PC_REG);
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += 3;
}

void Ldq_M(void)
{ //CD 6309 WORK
  Q_REG = MemRead32(PC_REG);
  PC_REG += 4;
  CC_Z = ZTEST(Q_REG);
  CC_N = NTEST32(Q_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Ldu_M(void)
{ //CE
  U_REG = IMMADDRESS(PC_REG);
  CC_Z = ZTEST(U_REG);
  CC_N = NTEST16(U_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += 3;
}

void Subb_D(void)
{ //D0
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = B_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Cmpb_D(void)
{ //D1
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp8 = B_REG - postbyte;
  CC_C = temp8 > B_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, B_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Sbcb_D(void)
{ //D2
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = B_REG - postbyte - CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Addd_D(void)
{ //D3
  temp16 = MemRead16(DPADDRESS(PC_REG++));
  temp32 = D_REG + temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, D_REG);
  D_REG = temp32;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += instance->NatEmuCycles64;
}

void Andb_D(void)
{ //D4 
  B_REG = B_REG & MemRead8(DPADDRESS(PC_REG++));
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Bitb_D(void)
{ //D5
  temp8 = B_REG & MemRead8(DPADDRESS(PC_REG++));
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Ldb_D(void)
{ //D6
  B_REG = MemRead8(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Stb_D(void)
{ //D7
  MemWrite8(B_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Eorb_D(void)
{ //D8	
  B_REG = B_REG ^ MemRead8(DPADDRESS(PC_REG++));
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Adcb_D(void)
{ //D9
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = B_REG + postbyte + CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  CC_H = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Orb_D(void)
{ //DA
  B_REG = B_REG | MemRead8(DPADDRESS(PC_REG++));
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Addb_D(void)
{ //DB
  postbyte = MemRead8(DPADDRESS(PC_REG++));
  temp16 = B_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += instance->NatEmuCycles43;
}

void Ldd_D(void)
{ //DC
  D_REG = MemRead16(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Std_D(void)
{ //DD
  MemWrite16(D_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Ldu_D(void)
{ //DE
  U_REG = MemRead16(DPADDRESS(PC_REG++));
  CC_Z = ZTEST(U_REG);
  CC_N = NTEST16(U_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Stu_D(void)
{ //DF
  MemWrite16(U_REG, DPADDRESS(PC_REG++));
  CC_Z = ZTEST(U_REG);
  CC_N = NTEST16(U_REG);
  CC_V = 0;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Subb_X(void)
{ //E0
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = B_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  instance->CycleCounter += 4;
}

void Cmpb_X(void)
{ //E1
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp8 = B_REG - postbyte;
  CC_C = temp8 > B_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, B_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  instance->CycleCounter += 4;
}

void Sbcb_X(void)
{ //E2
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = B_REG - postbyte - CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += 4;
}

void Addd_X(void)
{ //E3 
  temp16 = MemRead16(INDADDRESS(PC_REG++));
  temp32 = D_REG + temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, D_REG);
  D_REG = temp32;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Andb_X(void)
{ //E4
  B_REG = B_REG & MemRead8(INDADDRESS(PC_REG++));
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Bitb_X(void)
{ //E5 
  temp8 = B_REG & MemRead8(INDADDRESS(PC_REG++));
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Ldb_X(void)
{ //E6
  B_REG = MemRead8(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Stb_X(void)
{ //E7
  MemWrite8(B_REG, HD6309_CalculateEA(MemRead8(PC_REG++)));
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Eorb_X(void)
{ //E8
  B_REG = B_REG ^ MemRead8(INDADDRESS(PC_REG++));
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Adcb_X(void)
{ //E9
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = B_REG + postbyte + CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  CC_H = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += 4;
}

void Orb_X(void)
{ //EA 
  B_REG = B_REG | MemRead8(INDADDRESS(PC_REG++));
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  instance->CycleCounter += 4;
}

void Addb_X(void)
{ //EB
  postbyte = MemRead8(INDADDRESS(PC_REG++));
  temp16 = B_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  instance->CycleCounter += 4;
}

void Ldd_X(void)
{ //EC
  D_REG = MemRead16(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Std_X(void)
{ //ED
  MemWrite16(D_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Ldu_X(void)
{ //EE
  U_REG = MemRead16(INDADDRESS(PC_REG++));
  CC_Z = ZTEST(U_REG);
  CC_N = NTEST16(U_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Stu_X(void)
{ //EF
  MemWrite16(U_REG, INDADDRESS(PC_REG++));
  CC_Z = ZTEST(U_REG);
  CC_N = NTEST16(U_REG);
  CC_V = 0;
  instance->CycleCounter += 5;
}

void Subb_E(void)
{ //F0
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = B_REG - postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Cmpb_E(void)
{ //F1
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp8 = B_REG - postbyte;
  CC_C = temp8 > B_REG;
  CC_V = OVERFLOW8(CC_C, postbyte, temp8, B_REG);
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Sbcb_E(void)
{ //F2
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = B_REG - postbyte - CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Addd_E(void)
{ //F3
  temp16 = MemRead16(IMMADDRESS(PC_REG));
  temp32 = D_REG + temp16;
  CC_C = (temp32 & 0x10000) >> 16;
  CC_V = OVERFLOW16(CC_C, temp32, temp16, D_REG);
  D_REG = temp32;
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles76;
}

void Andb_E(void)
{  //F4
  B_REG = B_REG & MemRead8(IMMADDRESS(PC_REG));
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Bitb_E(void)
{ //F5
  temp8 = B_REG & MemRead8(IMMADDRESS(PC_REG));
  CC_N = NTEST8(temp8);
  CC_Z = ZTEST(temp8);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Ldb_E(void)
{ //F6
  B_REG = MemRead8(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Stb_E(void)
{ //F7 
  MemWrite8(B_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(B_REG);
  CC_N = NTEST8(B_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Eorb_E(void)
{ //F8
  B_REG = B_REG ^ MemRead8(IMMADDRESS(PC_REG));
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Adcb_E(void)
{ //F9
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = B_REG + postbyte + CC_C;
  CC_C = (temp16 & 0x100) >> 8;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  CC_H = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Orb_E(void)
{ //FA
  B_REG = B_REG | MemRead8(IMMADDRESS(PC_REG));
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Addb_E(void)
{ //FB
  postbyte = MemRead8(IMMADDRESS(PC_REG));
  temp16 = B_REG + postbyte;
  CC_C = (temp16 & 0x100) >> 8;
  CC_H = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
  CC_V = OVERFLOW8(CC_C, postbyte, temp16, B_REG);
  B_REG = (unsigned char)temp16;
  CC_N = NTEST8(B_REG);
  CC_Z = ZTEST(B_REG);
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles54;
}

void Ldd_E(void)
{ //FC
  D_REG = MemRead16(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Std_E(void)
{ //FD
  MemWrite16(D_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(D_REG);
  CC_N = NTEST16(D_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Ldu_E(void)
{ //FE
  U_REG = MemRead16(IMMADDRESS(PC_REG));
  CC_Z = ZTEST(U_REG);
  CC_N = NTEST16(U_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Stu_E(void)
{ //FF
  MemWrite16(U_REG, IMMADDRESS(PC_REG));
  CC_Z = ZTEST(U_REG);
  CC_N = NTEST16(U_REG);
  CC_V = 0;
  PC_REG += 2;
  instance->CycleCounter += instance->NatEmuCycles65;
}

void Page_1(void);
void Page_2(void);
void Page_3(void);

void(*JmpVec1[256])(void) = {
  Neg_D,		// 00
  Oim_D,		// 01
  Aim_D,		// 02
  Com_D,		// 03
  Lsr_D,		// 04
  Eim_D,		// 05
  Ror_D,		// 06
  Asr_D,		// 07
  Asl_D,		// 08
  Rol_D,		// 09
  Dec_D,		// 0A
  Tim_D,		// 0B
  Inc_D,		// 0C
  Tst_D,		// 0D
  Jmp_D,		// 0E
  Clr_D,		// 0F
  Page_2,		// 10
  Page_3,		// 11
  Nop_I,		// 12
  Sync_I,		// 13
  Sexw_I,		// 14
  InvalidInsHandler,	// 15
  Lbra_R,		// 16
  Lbsr_R,		// 17
  InvalidInsHandler,	// 18
  Daa_I,		// 19
  Orcc_M,		// 1A
  InvalidInsHandler,	// 1B
  Andcc_M,	// 1C
  Sex_I,		// 1D
  Exg_M,		// 1E
  Tfr_M,		// 1F
  Bra_R,		// 20
  Brn_R,		// 21
  Bhi_R,		// 22
  Bls_R,		// 23
  Bhs_R,		// 24
  Blo_R,		// 25
  Bne_R,		// 26
  Beq_R,		// 27
  Bvc_R,		// 28
  Bvs_R,		// 29
  Bpl_R,		// 2A
  Bmi_R,		// 2B
  Bge_R,		// 2C
  Blt_R,		// 2D
  Bgt_R,		// 2E
  Ble_R,		// 2F
  Leax_X,		// 30
  Leay_X,		// 31
  Leas_X,		// 32
  Leau_X,		// 33
  Pshs_M,		// 34
  Puls_M,		// 35
  Pshu_M,		// 36
  Pulu_M,		// 37
  InvalidInsHandler,	// 38
  Rts_I,		// 39
  Abx_I,		// 3A
  Rti_I,		// 3B
  Cwai_I,		// 3C
  Mul_I,		// 3D
  Reset,		// 3E
  Swi1_I,		// 3F
  Nega_I,		// 40
  InvalidInsHandler,	// 41
  InvalidInsHandler,	// 42
  Coma_I,		// 43
  Lsra_I,		// 44
  InvalidInsHandler,	// 45
  Rora_I,		// 46
  Asra_I,		// 47
  Asla_I,		// 48
  Rola_I,		// 49
  Deca_I,		// 4A
  InvalidInsHandler,	// 4B
  Inca_I,		// 4C
  Tsta_I,		// 4D
  InvalidInsHandler,	// 4E
  Clra_I,		// 4F
  Negb_I,		// 50
  InvalidInsHandler,	// 51
  InvalidInsHandler,	// 52
  Comb_I,		// 53
  Lsrb_I,		// 54
  InvalidInsHandler,	// 55
  Rorb_I,		// 56
  Asrb_I,		// 57
  Aslb_I,		// 58
  Rolb_I,		// 59
  Decb_I,		// 5A
  InvalidInsHandler,	// 5B
  Incb_I,		// 5C
  Tstb_I,		// 5D
  InvalidInsHandler,	// 5E
  Clrb_I,		// 5F
  Neg_X,		// 60
  Oim_X,		// 61
  Aim_X,		// 62
  Com_X,		// 63
  Lsr_X,		// 64
  Eim_X,		// 65
  Ror_X,		// 66
  Asr_X,		// 67
  Asl_X,		// 68
  Rol_X,		// 69
  Dec_X,		// 6A
  Tim_X,		// 6B
  Inc_X,		// 6C
  Tst_X,		// 6D
  Jmp_X,		// 6E
  Clr_X,		// 6F
  Neg_E,		// 70
  Oim_E,		// 71
  Aim_E,		// 72
  Com_E,		// 73
  Lsr_E,		// 74
  Eim_E,		// 75
  Ror_E,		// 76
  Asr_E,		// 77
  Asl_E,		// 78
  Rol_E,		// 79
  Dec_E,		// 7A
  Tim_E,		// 7B
  Inc_E,		// 7C
  Tst_E,		// 7D
  Jmp_E,		// 7E
  Clr_E,		// 7F
  Suba_M,		// 80
  Cmpa_M,		// 81
  Sbca_M,		// 82
  Subd_M,		// 83
  Anda_M,		// 84
  Bita_M,		// 85
  Lda_M,		// 86
  InvalidInsHandler,	// 87
  Eora_M,		// 88
  Adca_M,		// 89
  Ora_M,		// 8A
  Adda_M,		// 8B
  Cmpx_M,		// 8C
  Bsr_R,		// 8D
  Ldx_M,		// 8E
  InvalidInsHandler,	// 8F
  Suba_D,		// 90
  Cmpa_D,		// 91
  Scba_D,		// 92
  Subd_D,		// 93
  Anda_D,		// 94
  Bita_D,		// 95
  Lda_D,		// 96
  Sta_D,		// 97
  Eora_D,		// 98
  Adca_D,		// 99
  Ora_D,		// 9A
  Adda_D,		// 9B
  Cmpx_D,		// 9C
  Jsr_D,		// 9D
  Ldx_D,		// 9E
  Stx_D,		// 9A
  Suba_X,		// A0
  Cmpa_X,		// A1
  Sbca_X,		// A2
  Subd_X,		// A3
  Anda_X,		// A4
  Bita_X,		// A5
  Lda_X,		// A6
  Sta_X,		// A7
  Eora_X,		// a8
  Adca_X,		// A9
  Ora_X,		// AA
  Adda_X,		// AB
  Cmpx_X,		// AC
  Jsr_X,		// AD
  Ldx_X,		// AE
  Stx_X,		// AF
  Suba_E,		// B0
  Cmpa_E,		// B1
  Sbca_E,		// B2
  Subd_E,		// B3
  Anda_E,		// B4
  Bita_E,		// B5
  Lda_E,		// B6
  Sta_E,		// B7
  Eora_E,		// B8
  Adca_E,		// B9
  Ora_E,		// BA
  Adda_E,		// BB
  Cmpx_E,		// BC
  Bsr_E,		// BD
  Ldx_E,		// BE
  Stx_E,		// BF
  Subb_M,		// C0
  Cmpb_M,		// C1
  Sbcb_M,		// C2
  Addd_M,		// C3
  Andb_M,		// C4
  Bitb_M,		// C5
  Ldb_M,		// C6
  InvalidInsHandler,		// C7
  Eorb_M,		// C8
  Adcb_M,		// C9
  Orb_M,		// CA
  Addb_M,		// CB
  Ldd_M,		// CC
  Ldq_M,		// CD
  Ldu_M,		// CE
  InvalidInsHandler,		// CF
  Subb_D,		// D0
  Cmpb_D,		// D1
  Sbcb_D,		// D2
  Addd_D,		// D3
  Andb_D,		// D4
  Bitb_D,		// D5
  Ldb_D,		// D6
  Stb_D,		// D7
  Eorb_D,		// D8
  Adcb_D,		// D9
  Orb_D,		// DA
  Addb_D,		// DB
  Ldd_D,		// DC
  Std_D,		// DD
  Ldu_D,		// DE
  Stu_D,		// DF
  Subb_X,		// E0
  Cmpb_X,		// E1
  Sbcb_X,		// E2
  Addd_X,		// E3
  Andb_X,		// E4
  Bitb_X,		// E5
  Ldb_X,		// E6
  Stb_X,		// E7
  Eorb_X,		// E8
  Adcb_X,		// E9
  Orb_X,		// EA
  Addb_X,		// EB
  Ldd_X,		// EC
  Std_X,		// ED
  Ldu_X,		// EE
  Stu_X,		// EF
  Subb_E,		// F0
  Cmpb_E,		// F1
  Sbcb_E,		// F2
  Addd_E,		// F3
  Andb_E,		// F4
  Bitb_E,		// F5
  Ldb_E,		// F6
  Stb_E,		// F7
  Eorb_E,		// F8
  Adcb_E,		// F9
  Orb_E,		// FA
  Addb_E,		// FB
  Ldd_E,		// FC
  Std_E,		// FD
  Ldu_E,		// FE
  Stu_E,		// FF
};

void(*JmpVec2[256])(void) = {
  InvalidInsHandler,		// 00
  InvalidInsHandler,		// 01
  InvalidInsHandler,		// 02
  InvalidInsHandler,		// 03
  InvalidInsHandler,		// 04
  InvalidInsHandler,		// 05
  InvalidInsHandler,		// 06
  InvalidInsHandler,		// 07
  InvalidInsHandler,		// 08
  InvalidInsHandler,		// 09
  InvalidInsHandler,		// 0A
  InvalidInsHandler,		// 0B
  InvalidInsHandler,		// 0C
  InvalidInsHandler,		// 0D
  InvalidInsHandler,		// 0E
  InvalidInsHandler,		// 0F
  InvalidInsHandler,		// 10
  InvalidInsHandler,		// 11
  InvalidInsHandler,		// 12
  InvalidInsHandler,		// 13
  InvalidInsHandler,		// 14
  InvalidInsHandler,		// 15
  InvalidInsHandler,		// 16
  InvalidInsHandler,		// 17
  InvalidInsHandler,		// 18
  InvalidInsHandler,		// 19
  InvalidInsHandler,		// 1A
  InvalidInsHandler,		// 1B
  InvalidInsHandler,		// 1C
  InvalidInsHandler,		// 1D
  InvalidInsHandler,		// 1E
  InvalidInsHandler,		// 1F
  InvalidInsHandler,		// 20
  LBrn_R,		// 21
  LBhi_R,		// 22
  LBls_R,		// 23
  LBhs_R,		// 24
  LBcs_R,		// 25
  LBne_R,		// 26
  LBeq_R,		// 27
  LBvc_R,		// 28
  LBvs_R,		// 29
  LBpl_R,		// 2A
  LBmi_R,		// 2B
  LBge_R,		// 2C
  LBlt_R,		// 2D
  LBgt_R,		// 2E
  LBle_R,		// 2F
  Addr,		// 30
  Adcr,		// 31
  Subr,		// 32
  Sbcr,		// 33
  Andr,		// 34
  Orr,		// 35
  Eorr,		// 36
  Cmpr,		// 37
  Pshsw,		// 38
  Pulsw,		// 39
  Pshuw,		// 3A
  Puluw,		// 3B
  InvalidInsHandler,		// 3C
  InvalidInsHandler,		// 3D
  InvalidInsHandler,		// 3E
  Swi2_I,		// 3F
  Negd_I,		// 40
  InvalidInsHandler,		// 41
  InvalidInsHandler,		// 42
  Comd_I,		// 43
  Lsrd_I,		// 44
  InvalidInsHandler,		// 45
  Rord_I,		// 46
  Asrd_I,		// 47
  Asld_I,		// 48
  Rold_I,		// 49
  Decd_I,		// 4A
  InvalidInsHandler,		// 4B
  Incd_I,		// 4C
  Tstd_I,		// 4D
  InvalidInsHandler,		// 4E
  Clrd_I,		// 4F
  InvalidInsHandler,		// 50
  InvalidInsHandler,		// 51
  InvalidInsHandler,		// 52
  Comw_I,		// 53
  Lsrw_I,		// 54
  InvalidInsHandler,		// 55
  Rorw_I,		// 56
  InvalidInsHandler,		// 57
  InvalidInsHandler,		// 58
  Rolw_I,		// 59
  Decw_I,		// 5A
  InvalidInsHandler,		// 5B
  Incw_I,		// 5C
  Tstw_I,		// 5D
  InvalidInsHandler,		// 5E
  Clrw_I,		// 5F
  InvalidInsHandler,		// 60
  InvalidInsHandler,		// 61
  InvalidInsHandler,		// 62
  InvalidInsHandler,		// 63
  InvalidInsHandler,		// 64
  InvalidInsHandler,		// 65
  InvalidInsHandler,		// 66
  InvalidInsHandler,		// 67
  InvalidInsHandler,		// 68
  InvalidInsHandler,		// 69
  InvalidInsHandler,		// 6A
  InvalidInsHandler,		// 6B
  InvalidInsHandler,		// 6C
  InvalidInsHandler,		// 6D
  InvalidInsHandler,		// 6E
  InvalidInsHandler,		// 6F
  InvalidInsHandler,		// 70
  InvalidInsHandler,		// 71
  InvalidInsHandler,		// 72
  InvalidInsHandler,		// 73
  InvalidInsHandler,		// 74
  InvalidInsHandler,		// 75
  InvalidInsHandler,		// 76
  InvalidInsHandler,		// 77
  InvalidInsHandler,		// 78
  InvalidInsHandler,		// 79
  InvalidInsHandler,		// 7A
  InvalidInsHandler,		// 7B
  InvalidInsHandler,		// 7C
  InvalidInsHandler,		// 7D
  InvalidInsHandler,		// 7E
  InvalidInsHandler,		// 7F
  Subw_M,		// 80
  Cmpw_M,		// 81
  Sbcd_M,		// 82
  Cmpd_M,		// 83
  Andd_M,		// 84
  Bitd_M,		// 85
  Ldw_M,		// 86
  InvalidInsHandler,		// 87
  Eord_M,		// 88
  Adcd_M,		// 89
  Ord_M,		// 8A
  Addw_M,		// 8B
  Cmpy_M,		// 8C
  InvalidInsHandler,		// 8D
  Ldy_M,		// 8E
  InvalidInsHandler,		// 8F
  Subw_D,		// 90
  Cmpw_D,		// 91
  Sbcd_D,		// 92
  Cmpd_D,		// 93
  Andd_D,		// 94
  Bitd_D,		// 95
  Ldw_D,		// 96
  Stw_D,		// 97
  Eord_D,		// 98
  Adcd_D,		// 99
  Ord_D,		// 9A
  Addw_D,		// 9B
  Cmpy_D,		// 9C
  InvalidInsHandler,		// 9D
  Ldy_D,		// 9E
  Sty_D,		// 9F
  Subw_X,		// A0
  Cmpw_X,		// A1
  Sbcd_X,		// A2
  Cmpd_X,		// A3
  Andd_X,		// A4
  Bitd_X,		// A5
  Ldw_X,		// A6
  Stw_X,		// A7
  Eord_X,		// A8
  Adcd_X,		// A9
  Ord_X,		// AA
  Addw_X,		// AB
  Cmpy_X,		// AC
  InvalidInsHandler,		// AD
  Ldy_X,		// AE
  Sty_X,		// AF
  Subw_E,		// B0
  Cmpw_E,		// B1
  Sbcd_E,		// B2
  Cmpd_E,		// B3
  Andd_E,		// B4
  Bitd_E,		// B5
  Ldw_E,		// B6
  Stw_E,		// B7
  Eord_E,		// B8
  Adcd_E,		// B9
  Ord_E,		// BA
  Addw_E,		// BB
  Cmpy_E,		// BC
  InvalidInsHandler,		// BD
  Ldy_E,		// BE
  Sty_E,		// BF
  InvalidInsHandler,		// C0
  InvalidInsHandler,		// C1
  InvalidInsHandler,		// C2
  InvalidInsHandler,		// C3
  InvalidInsHandler,		// C4
  InvalidInsHandler,		// C5
  InvalidInsHandler,		// C6
  InvalidInsHandler,		// C7
  InvalidInsHandler,		// C8
  InvalidInsHandler,		// C9
  InvalidInsHandler,		// CA
  InvalidInsHandler,		// CB
  InvalidInsHandler,		// CC
  InvalidInsHandler,		// CD
  Lds_I,		// CE
  InvalidInsHandler,		// CF
  InvalidInsHandler,		// D0
  InvalidInsHandler,		// D1
  InvalidInsHandler,		// D2
  InvalidInsHandler,		// D3
  InvalidInsHandler,		// D4
  InvalidInsHandler,		// D5
  InvalidInsHandler,		// D6
  InvalidInsHandler,		// D7
  InvalidInsHandler,		// D8
  InvalidInsHandler,		// D9
  InvalidInsHandler,		// DA
  InvalidInsHandler,		// DB
  Ldq_D,		// DC
  Stq_D,		// DD
  Lds_D,		// DE
  Sts_D,		// DF
  InvalidInsHandler,		// E0
  InvalidInsHandler,		// E1
  InvalidInsHandler,		// E2
  InvalidInsHandler,		// E3
  InvalidInsHandler,		// E4
  InvalidInsHandler,		// E5
  InvalidInsHandler,		// E6
  InvalidInsHandler,		// E7
  InvalidInsHandler,		// E8
  InvalidInsHandler,		// E9
  InvalidInsHandler,		// EA
  InvalidInsHandler,		// EB
  Ldq_X,		// EC
  Stq_X,		// ED
  Lds_X,		// EE
  Sts_X,		// EF
  InvalidInsHandler,		// F0
  InvalidInsHandler,		// F1
  InvalidInsHandler,		// F2
  InvalidInsHandler,		// F3
  InvalidInsHandler,		// F4
  InvalidInsHandler,		// F5
  InvalidInsHandler,		// F6
  InvalidInsHandler,		// F7
  InvalidInsHandler,		// F8
  InvalidInsHandler,		// F9
  InvalidInsHandler,		// FA
  InvalidInsHandler,		// FB
  Ldq_E,		// FC
  Stq_E,		// FD
  Lds_E,		// FE
  Sts_E,		// FF
};

void(*JmpVec3[256])(void) = {
  InvalidInsHandler,		// 00
  InvalidInsHandler,		// 01
  InvalidInsHandler,		// 02
  InvalidInsHandler,		// 03
  InvalidInsHandler,		// 04
  InvalidInsHandler,		// 05
  InvalidInsHandler,		// 06
  InvalidInsHandler,		// 07
  InvalidInsHandler,		// 08
  InvalidInsHandler,		// 09
  InvalidInsHandler,		// 0A
  InvalidInsHandler,		// 0B
  InvalidInsHandler,		// 0C
  InvalidInsHandler,		// 0D
  InvalidInsHandler,		// 0E
  InvalidInsHandler,		// 0F
  InvalidInsHandler,		// 10
  InvalidInsHandler,		// 11
  InvalidInsHandler,		// 12
  InvalidInsHandler,		// 13
  InvalidInsHandler,		// 14
  InvalidInsHandler,		// 15
  InvalidInsHandler,		// 16
  InvalidInsHandler,		// 17
  InvalidInsHandler,		// 18
  InvalidInsHandler,		// 19
  InvalidInsHandler,		// 1A
  InvalidInsHandler,		// 1B
  InvalidInsHandler,		// 1C
  InvalidInsHandler,		// 1D
  InvalidInsHandler,		// 1E
  InvalidInsHandler,		// 1F
  InvalidInsHandler,		// 20
  InvalidInsHandler,		// 21
  InvalidInsHandler,		// 22
  InvalidInsHandler,		// 23
  InvalidInsHandler,		// 24
  InvalidInsHandler,		// 25
  InvalidInsHandler,		// 26
  InvalidInsHandler,		// 27
  InvalidInsHandler,		// 28
  InvalidInsHandler,		// 29
  InvalidInsHandler,		// 2A
  InvalidInsHandler,		// 2B
  InvalidInsHandler,		// 2C
  InvalidInsHandler,		// 2D
  InvalidInsHandler,		// 2E
  InvalidInsHandler,		// 2F
  Band,		// 30
  Biand,		// 31
  Bor,		// 32
  Bior,		// 33
  Beor,		// 34
  Bieor,		// 35
  Ldbt,		// 36
  Stbt,		// 37
  Tfm1,		// 38
  Tfm2,		// 39
  Tfm3,		// 3A
  Tfm4,		// 3B
  Bitmd_M,	// 3C
  Ldmd_M,		// 3D
  InvalidInsHandler,		// 3E
  Swi3_I,		// 3F
  InvalidInsHandler,		// 40
  InvalidInsHandler,		// 41
  InvalidInsHandler,		// 42
  Come_I,		// 43
  InvalidInsHandler,		// 44
  InvalidInsHandler,		// 45
  InvalidInsHandler,		// 46
  InvalidInsHandler,		// 47
  InvalidInsHandler,		// 48
  InvalidInsHandler,		// 49
  Dece_I,		// 4A
  InvalidInsHandler,		// 4B
  Ince_I,		// 4C
  Tste_I,		// 4D
  InvalidInsHandler,		// 4E
  Clre_I,		// 4F
  InvalidInsHandler,		// 50
  InvalidInsHandler,		// 51
  InvalidInsHandler,		// 52
  Comf_I,		// 53
  InvalidInsHandler,		// 54
  InvalidInsHandler,		// 55
  InvalidInsHandler,		// 56
  InvalidInsHandler,		// 57
  InvalidInsHandler,		// 58
  InvalidInsHandler,		// 59
  Decf_I,		// 5A
  InvalidInsHandler,		// 5B
  Incf_I,		// 5C
  Tstf_I,		// 5D
  InvalidInsHandler,		// 5E
  Clrf_I,		// 5F
  InvalidInsHandler,		// 60
  InvalidInsHandler,		// 61
  InvalidInsHandler,		// 62
  InvalidInsHandler,		// 63
  InvalidInsHandler,		// 64
  InvalidInsHandler,		// 65
  InvalidInsHandler,		// 66
  InvalidInsHandler,		// 67
  InvalidInsHandler,		// 68
  InvalidInsHandler,		// 69
  InvalidInsHandler,		// 6A
  InvalidInsHandler,		// 6B
  InvalidInsHandler,		// 6C
  InvalidInsHandler,		// 6D
  InvalidInsHandler,		// 6E
  InvalidInsHandler,		// 6F
  InvalidInsHandler,		// 70
  InvalidInsHandler,		// 71
  InvalidInsHandler,		// 72
  InvalidInsHandler,		// 73
  InvalidInsHandler,		// 74
  InvalidInsHandler,		// 75
  InvalidInsHandler,		// 76
  InvalidInsHandler,		// 77
  InvalidInsHandler,		// 78
  InvalidInsHandler,		// 79
  InvalidInsHandler,		// 7A
  InvalidInsHandler,		// 7B
  InvalidInsHandler,		// 7C
  InvalidInsHandler,		// 7D
  InvalidInsHandler,		// 7E
  InvalidInsHandler,		// 7F
  Sube_M,		// 80
  Cmpe_M,		// 81
  InvalidInsHandler,		// 82
  Cmpu_M,		// 83
  InvalidInsHandler,		// 84
  InvalidInsHandler,		// 85
  Lde_M,		// 86
  InvalidInsHandler,		// 87
  InvalidInsHandler,		// 88
  InvalidInsHandler,		// 89
  InvalidInsHandler,		// 8A
  Adde_M,		// 8B
  Cmps_M,		// 8C
  Divd_M,		// 8D
  Divq_M,		// 8E
  Muld_M,		// 8F
  Sube_D,		// 90
  Cmpe_D,		// 91
  InvalidInsHandler,		// 92
  Cmpu_D,		// 93
  InvalidInsHandler,		// 94
  InvalidInsHandler,		// 95
  Lde_D,		// 96
  Ste_D,		// 97
  InvalidInsHandler,		// 98
  InvalidInsHandler,		// 99
  InvalidInsHandler,		// 9A
  Adde_D,		// 9B
  Cmps_D,		// 9C
  Divd_D,		// 9D
  Divq_D,		// 9E
  Muld_D,		// 9F
  Sube_X,		// A0
  Cmpe_X,		// A1
  InvalidInsHandler,		// A2
  Cmpu_X,		// A3
  InvalidInsHandler,		// A4
  InvalidInsHandler,		// A5
  Lde_X,		// A6
  Ste_X,		// A7
  InvalidInsHandler,		// A8
  InvalidInsHandler,		// A9
  InvalidInsHandler,		// AA
  Adde_X,		// AB
  Cmps_X,		// AC
  Divd_X,		// AD
  Divq_X,		// AE
  Muld_X,		// AF
  Sube_E,		// B0
  Cmpe_E,		// B1
  InvalidInsHandler,		// B2
  Cmpu_E,		// B3
  InvalidInsHandler,		// B4
  InvalidInsHandler,		// B5
  Lde_E,		// B6
  Ste_E,		// B7
  InvalidInsHandler,		// B8
  InvalidInsHandler,		// B9
  InvalidInsHandler,		// BA
  Adde_E,		// BB
  Cmps_E,		// BC
  Divd_E,		// BD
  Divq_E,		// BE
  Muld_E,		// BF
  Subf_M,		// C0
  Cmpf_M,		// C1
  InvalidInsHandler,		// C2
  InvalidInsHandler,		// C3
  InvalidInsHandler,		// C4
  InvalidInsHandler,		// C5
  Ldf_M,		// C6
  InvalidInsHandler,		// C7
  InvalidInsHandler,		// C8
  InvalidInsHandler,		// C9
  InvalidInsHandler,		// CA
  Addf_M,		// CB
  InvalidInsHandler,		// CC
  InvalidInsHandler,		// CD
  InvalidInsHandler,		// CE
  InvalidInsHandler,		// CF
  Subf_D,		// D0
  Cmpf_D,		// D1
  InvalidInsHandler,		// D2
  InvalidInsHandler,		// D3
  InvalidInsHandler,		// D4
  InvalidInsHandler,		// D5
  Ldf_D,		// D6
  Stf_D,		// D7
  InvalidInsHandler,		// D8
  InvalidInsHandler,		// D9
  InvalidInsHandler,		// DA
  Addf_D,		// DB
  InvalidInsHandler,		// DC
  InvalidInsHandler,		// DD
  InvalidInsHandler,		// DE
  InvalidInsHandler,		// DF
  Subf_X,		// E0
  Cmpf_X,		// E1
  InvalidInsHandler,		// E2
  InvalidInsHandler,		// E3
  InvalidInsHandler,		// E4
  InvalidInsHandler,		// E5
  Ldf_X,		// E6
  Stf_X,		// E7
  InvalidInsHandler,		// E8
  InvalidInsHandler,		// E9
  InvalidInsHandler,		// EA
  Addf_X,		// EB
  InvalidInsHandler,		// EC
  InvalidInsHandler,		// ED
  InvalidInsHandler,		// EE
  InvalidInsHandler,		// EF
  Subf_E,		// F0
  Cmpf_E,		// F1
  InvalidInsHandler,		// F2
  InvalidInsHandler,		// F3
  InvalidInsHandler,		// F4
  InvalidInsHandler,		// F5
  Ldf_E,		// F6
  Stf_E,		// F7
  InvalidInsHandler,		// F8
  InvalidInsHandler,		// F9
  InvalidInsHandler,		// FA
  Addf_E,		// FB
  InvalidInsHandler,		// FC
  InvalidInsHandler,		// FD
  InvalidInsHandler,		// FE
  InvalidInsHandler,		// FF
};

//--Essentially Page_1()
extern "C" {
  __declspec(dllexport) void __cdecl HD6309ExecOpCode(int cycleFor, unsigned char opcode) {
    HD6309State* hd63096State = GetHD6309State();

    instance->gCycleFor = cycleFor;

    JmpVec1[opcode](); // Execute instruction pointed to by PC_REG
  }
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
