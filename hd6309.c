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
#include "hd6309opcodes.h"

#include "MmuAccessors.h"
#include "MemRead8.h"
#include "MemWrite8.h"
#include "MemRead16.h"
#include "MemWrite16.h"
#include "hd6309state.h"

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
void MemWrite32(unsigned int, unsigned short);
unsigned int MemRead32(unsigned short);

void HD6309Reset(void)
{
  char index;

  for (index = 0; index <= 6; index++) {		//Set all register to 0 except V
    *xfreg16[index] = 0;
  }

  for (index = 0; index <= 7; index++) {
    *ureg8[index] = 0;
  }

  for (index = 0; index <= 7; index++) {
    cc[index] = 0;
  }

  for (index = 0; index <= 7; index++) {
    md[index] = 0;
  }

  mdbits = getmd();
  dp.Reg = 0;
  cc[I] = 1;
  cc[F] = 1;
  SyncWaiting = 0;
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
  ureg8[3] = (unsigned char*)&dp.B.msb;
  ureg8[4] = (unsigned char*)&z.B.msb;
  ureg8[5] = (unsigned char*)&z.B.lsb;
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

  cc[I] = 1;
  cc[F] = 1;
}


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

int HD6309Exec(int CycleFor)
{
  //static unsigned char opcode = 0;
  CycleCounter = 0;
  gCycleFor = CycleFor;

  while (CycleCounter < CycleFor) {

    if (PendingInterrupts)
    {
      if (PendingInterrupts & 4)
        cpu_nmi();

      if (PendingInterrupts & 2)
        cpu_firq();

      if (PendingInterrupts & 1)
      {
        if (IRQWaiter == 0)	// This is needed to fix a subtle timming problem
          cpu_irq();		// It allows the CPU to see $FF03 bit 7 high before
        else				// The IRQ is asserted.
          IRQWaiter -= 1;
      }
    }

    if (SyncWaiting == 1)	//Abort the run nothing happens asyncronously from the CPU
      return(0); // WDZ - Experimental SyncWaiting should still return used cycles (and not zero) by breaking from loop

    JmpVec1[MemRead8(PC_REG++)](); // Execute instruction pointed to by PC_REG
  }

  return(CycleFor - CycleCounter);
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
  if (!cc[F])
  {
    InInterrupt = 1; //Flag to indicate FIRQ has been asserted
    switch (md[FIRQMODE])
    {
    case 0:
      cc[E] = 0; // Turn E flag off
      MemWrite8(pc.B.lsb, --S_REG);
      MemWrite8(pc.B.msb, --S_REG);
      MemWrite8(getcc(), --S_REG);
      cc[I] = 1;
      cc[F] = 1;
      PC_REG = MemRead16(VFIRQ);
      break;

    case 1:		//6309
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
      }

      MemWrite8(B_REG, --S_REG);
      MemWrite8(A_REG, --S_REG);
      MemWrite8(getcc(), --S_REG);
      cc[I] = 1;
      cc[F] = 1;
      PC_REG = MemRead16(VFIRQ);
      break;
    }
  }

  PendingInterrupts = PendingInterrupts & 253;
}

void cpu_irq(void)
{
  if (InInterrupt == 1) //If FIRQ is running postpone the IRQ
    return;

  if ((!cc[I]))
  {
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
    }

    MemWrite8(B_REG, --S_REG);
    MemWrite8(A_REG, --S_REG);
    MemWrite8(getcc(), --S_REG);
    PC_REG = MemRead16(VIRQ);
    cc[I] = 1;
  }

  PendingInterrupts = PendingInterrupts & 254;
}

void cpu_nmi(void)
{
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
  }

  MemWrite8(B_REG, --S_REG);
  MemWrite8(A_REG, --S_REG);
  MemWrite8(getcc(), --S_REG);
  cc[I] = 1;
  cc[F] = 1;
  PC_REG = MemRead16(VNMI);
  PendingInterrupts = PendingInterrupts & 251;
}

unsigned short hd6309_CalculateEA(unsigned char postbyte)
{
  static unsigned short int ea = 0;
  static signed char byte = 0;
  static unsigned char Register;

  Register = ((postbyte >> 5) & 3) + 1;

  if (postbyte & 0x80)
  {
    switch (postbyte & 0x1F)
    {
    case 0: // Post inc by 1
      ea = (*xfreg16[Register]);
      (*xfreg16[Register])++;
      CycleCounter += NatEmuCycles21;
      break;

    case 1: // post in by 2
      ea = (*xfreg16[Register]);
      (*xfreg16[Register]) += 2;
      CycleCounter += NatEmuCycles32;
      break;

    case 2: // pre dec by 1
      (*xfreg16[Register]) -= 1;
      ea = (*xfreg16[Register]);
      CycleCounter += NatEmuCycles21;
      break;

    case 3: // pre dec by 2
      (*xfreg16[Register]) -= 2;
      ea = (*xfreg16[Register]);
      CycleCounter += NatEmuCycles32;
      break;

    case 4: // no offset
      ea = (*xfreg16[Register]);
      break;

    case 5: // B reg offset
      ea = (*xfreg16[Register]) + ((signed char)B_REG);
      CycleCounter += 1;
      break;

    case 6: // A reg offset
      ea = (*xfreg16[Register]) + ((signed char)A_REG);
      CycleCounter += 1;
      break;

    case 7: // E reg offset 
      ea = (*xfreg16[Register]) + ((signed char)E_REG);
      CycleCounter += 1;
      break;

    case 8: // 8 bit offset
      ea = (*xfreg16[Register]) + (signed char)MemRead8(PC_REG++);
      CycleCounter += 1;
      break;

    case 9: // 16 bit offset
      ea = (*xfreg16[Register]) + IMMADDRESS(PC_REG);
      CycleCounter += NatEmuCycles43;
      PC_REG += 2;
      break;

    case 10: // F reg offset
      ea = (*xfreg16[Register]) + ((signed char)F_REG);
      CycleCounter += 1;
      break;

    case 11: // D reg offset 
      ea = (*xfreg16[Register]) + D_REG; //Changed to unsigned 03/14/2005 NG Was signed
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
      ea = (*xfreg16[Register]) + W_REG;
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
      ea = (*xfreg16[Register]);
      (*xfreg16[Register]) += 2;
      ea = MemRead16(ea);
      CycleCounter += 6;
      break;

    case 18: // possibly illegal instruction
      CycleCounter += 6;
      break;

    case 19: // Indirect pre dec by 2
      (*xfreg16[Register]) -= 2;
      ea = (*xfreg16[Register]);
      ea = MemRead16(ea);
      CycleCounter += 6;
      break;

    case 20: // Indirect no offset 
      ea = (*xfreg16[Register]);
      ea = MemRead16(ea);
      CycleCounter += 3;
      break;

    case 21: // Indirect B reg offset
      ea = (*xfreg16[Register]) + ((signed char)B_REG);
      ea = MemRead16(ea);
      CycleCounter += 4;
      break;

    case 22: // indirect A reg offset
      ea = (*xfreg16[Register]) + ((signed char)A_REG);
      ea = MemRead16(ea);
      CycleCounter += 4;
      break;

    case 23: // indirect E reg offset
      ea = (*xfreg16[Register]) + ((signed char)E_REG);
      ea = MemRead16(ea);
      CycleCounter += 4;
      break;

    case 24: // indirect 8 bit offset
      ea = (*xfreg16[Register]) + (signed char)MemRead8(PC_REG++);
      ea = MemRead16(ea);
      CycleCounter += 4;
      break;

    case 25: // indirect 16 bit offset
      ea = (*xfreg16[Register]) + IMMADDRESS(PC_REG);
      ea = MemRead16(ea);
      CycleCounter += 7;
      PC_REG += 2;
      break;

    case 26: // indirect F reg offset
      ea = (*xfreg16[Register]) + ((signed char)F_REG);
      ea = MemRead16(ea);
      CycleCounter += 4;
      break;

    case 27: // indirect D reg offset
      ea = (*xfreg16[Register]) + D_REG;
      ea = MemRead16(ea);
      CycleCounter += 7;
      break;

    case 28: // indirect 8 bit PC relative
      ea = (signed short)PC_REG + (signed char)MemRead8(PC_REG) + 1;
      ea = MemRead16(ea);
      CycleCounter += 4;
      PC_REG++;
      break;

    case 29: //indirect 16 bit PC relative
      ea = PC_REG + IMMADDRESS(PC_REG) + 2;
      ea = MemRead16(ea);
      CycleCounter += 8;
      PC_REG += 2;
      break;

    case 30: // indirect W reg offset
      ea = (*xfreg16[Register]) + W_REG;
      ea = MemRead16(ea);
      CycleCounter += 7;
      break;

    case 31: // extended indirect
      ea = IMMADDRESS(PC_REG);
      ea = MemRead16(ea);
      CycleCounter += 8;
      PC_REG += 2;
      break;
    } //END Switch
  }
  else // 5 bit offset
  {
    byte = (postbyte & 31);
    byte = (byte << 3);
    byte = byte / 8;
    ea = *xfreg16[Register] + byte; //Was signed
    CycleCounter += 1;
  }

  return(ea);
}

void setcc(unsigned char bincc)
{
  unsigned char bit;
  ccbits = bincc;
  for (bit = 0;bit <= 7;bit++)
    cc[bit] = !!(bincc & (1 << bit));
  return;
}

unsigned char getcc(void)
{
  unsigned char bincc = 0, bit = 0;

  for (bit = 0;bit <= 7;bit++)
    if (cc[bit])
      bincc = bincc | (1 << bit);

  return(bincc);
}

void setmd(unsigned char binmd)
{
  unsigned char bit;

  for (bit = 0;bit <= 1;bit++)
    md[bit] = !!(binmd & (1 << bit));

  for (short i = 0; i < 24; i++)
  {
    *NatEmuCycles[i] = InsCycles[md[NATIVE6309]][i];
  }
}

unsigned char getmd(void)
{
  unsigned char binmd = 0, bit = 0;

  for (bit = 6;bit <= 7;bit++)
    if (md[bit])
      binmd = binmd | (1 << bit);

  return(binmd);
}

void HD6309AssertInterrupt(unsigned char interrupt, unsigned char waiter)// 4 nmi 2 firq 1 irq
{
  SyncWaiting = 0;
  PendingInterrupts = PendingInterrupts | (1 << (interrupt - 1));
  IRQWaiter = waiter;
}

void HD6309DeAssertInterrupt(unsigned char interrupt)// 4 nmi 2 firq 1 irq
{
  PendingInterrupts = PendingInterrupts & ~(1 << (interrupt - 1));
  InInterrupt = 0;
}

void InvalidInsHandler(void)
{
  md[ILLEGAL] = 1;
  mdbits = getmd();
  ErrorVector();
}

void DivbyZero(void)
{
  md[ZERODIV] = 1;
  mdbits = getmd();
  ErrorVector();
}

void ErrorVector(void)
{
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
  PC_REG = MemRead16(VTRAP);
  CycleCounter += (12 + NatEmuCycles54);	//One for each byte +overhead? Guessing from PSHS
}

unsigned char GetSorceReg(unsigned char Tmp)
{
  unsigned char Source = (Tmp >> 4);
  unsigned char Dest = Tmp & 15;
  unsigned char Translate[] = { 0,0 };

  if ((Source & 8) == (Dest & 8)) //like size registers
    return(Source);

  return(0);
}

void HD6309ForcePC(unsigned short NewPC)
{
  PC_REG = NewPC;
  PendingInterrupts = 0;
  SyncWaiting = 0;
}

/*****************************************************************
* 32 bit memory handling routines                                *
*****************************************************************/

unsigned int MemRead32(unsigned short addr)
{
  return (MemRead16(addr) << 16 | MemRead16(addr + 2));
}

void MemWrite32(unsigned int data, unsigned short addr)
{
  MemWrite16(data >> 16, addr);
  MemWrite16(data & 0xFFFF, addr + 2);
}
