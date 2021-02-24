#include "MC6809.h"
#include "TC1014MMU.h"

#include "MC6809Macros.h"

static MC6809State* instance = GetMC6809State();

extern "C" {
  __declspec(dllexport) void __cdecl MC6809ExecOpCode(int cycleFor, unsigned char opCode) {
    unsigned char msn, lsn;
    unsigned char source = 0;
    unsigned char dest = 0;

    unsigned char temp8;
    unsigned short temp16;
    unsigned int temp32;

    unsigned char postbyte = 0;
    short unsigned postword = 0;
    signed char* spostbyte = (signed char*)&postbyte;
    signed short* spostword = (signed short*)&postword;

    switch (opCode) {
    case NEG_D: //0
      temp16 = (DP_REG | MemRead8(PC_REG++));
      postbyte = MemRead8(temp16);
      temp8 = 0 - postbyte;
      CC_C = temp8 > 0;
      CC_V = (postbyte == 0x80);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case COM_D:
      temp16 = (DP_REG | MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      temp8 = 0xFF - temp8;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      CC_C = 1;
      CC_V = 0;
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case LSR_D: //S2
      temp16 = (DP_REG | MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      CC_C = temp8 & 1;
      temp8 = temp8 >> 1;
      CC_Z = ZTEST(temp8);
      CC_N = 0;
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case ROR_D: //S2
      temp16 = (DP_REG | MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      postbyte = CC_C << 7;
      CC_C = temp8 & 1;
      temp8 = (temp8 >> 1) | postbyte;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case ASR_D: //7
      temp16 = (DP_REG | MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      CC_C = temp8 & 1;
      temp8 = (temp8 & 0x80) | (temp8 >> 1);
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case ASL_D: //8
      temp16 = (DP_REG | MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      CC_C = (temp8 & 0x80) >> 7;
      CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
      temp8 = temp8 << 1;
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case ROL_D:	//9
      temp16 = (DP_REG | MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      postbyte = CC_C;
      CC_C = (temp8 & 0x80) >> 7;
      CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
      temp8 = (temp8 << 1) | postbyte;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case DEC_D: //A
      temp16 = (DP_REG | MemRead8(PC_REG++));
      temp8 = MemRead8(temp16) - 1;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      CC_V = temp8 == 0x7F;
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case INC_D: //C
      temp16 = (DP_REG | MemRead8(PC_REG++));
      temp8 = MemRead8(temp16) + 1;
      CC_Z = ZTEST(temp8);
      CC_V = temp8 == 0x80;
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case TST_D: //D
      temp8 = MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      CC_V = 0;
      instance->CycleCounter += 6;
      break;

    case JMP_D:	//E
      PC_REG = ((DP_REG | MemRead8(PC_REG)));
      instance->CycleCounter += 3;
      break;

    case CLR_D:	//F
      MemWrite8(0, (DP_REG | MemRead8(PC_REG++)));
      CC_Z = 1;
      CC_N = 0;
      CC_V = 0;
      CC_C = 0;
      instance->CycleCounter += 6;
      break;

    case Page2:
      switch (MemRead8(PC_REG++))
      {
      case LBEQ_R: //1027
        if (CC_Z)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBRN_R: //1021
        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBHI_R: //1022
        if (!(CC_C | CC_Z))
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBLS_R: //1023
        if (CC_C | CC_Z)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBHS_R: //1024
        if (!CC_C)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 6;
        break;

      case LBCS_R: //1025
        if (CC_C)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBNE_R: //1026
        if (!CC_Z)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBVC_R: //1028
        if (!CC_V)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBVS_R: //1029
        if (CC_V)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBPL_R: //102A
        if (!CC_N)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBMI_R: //102B
        if (CC_N)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBGE_R: //102C
        if (!(CC_N ^ CC_V))
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBLT_R: //102D
        if (CC_V ^ CC_N)
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBGT_R: //102E
        if (!(CC_Z | (CC_N ^ CC_V)))
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LBLE_R:	//102F
        if (CC_Z | (CC_N ^ CC_V))
        {
          *spostword = MemRead16(PC_REG);
          PC_REG += *spostword;
          instance->CycleCounter += 1;
        }

        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case SWI2_I: //103F
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
        MemWrite8(B_REG, --S_REG);
        MemWrite8(A_REG, --S_REG);
        MemWrite8(MC6809_getcc(), --S_REG);

        PC_REG = MemRead16(VSWI2);
        instance->CycleCounter += 20;
        break;

      case CMPD_M: //1083
        postword = MemRead16(PC_REG);
        temp16 = D_REG - postword;
        CC_C = temp16 > D_REG;
        CC_V = OTEST16(CC_C, postword, temp16, D_REG);//CC_C^((postword^temp16^D_REG)>>15);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case CMPY_M: //108C
        postword = MemRead16(PC_REG);
        temp16 = Y_REG - postword;
        CC_C = temp16 > Y_REG;
        CC_V = OTEST16(CC_C, postword, temp16, Y_REG);//CC_C^((postword^temp16^Y_REG)>>15);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case LDY_M: //108E
        Y_REG = MemRead16(PC_REG);
        CC_Z = ZTEST(Y_REG);
        CC_N = NTEST16(Y_REG);
        CC_V = 0;
        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case CMPD_D: //1093
        postword = MemRead16((DP_REG | MemRead8(PC_REG++)));
        temp16 = D_REG - postword;
        CC_C = temp16 > D_REG;
        CC_V = OTEST16(CC_C, postword, temp16, D_REG); //CC_C^((postword^temp16^D_REG)>>15);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        instance->CycleCounter += 7;
        break;

      case CMPY_D:	//109C
        postword = MemRead16((DP_REG | MemRead8(PC_REG++)));
        temp16 = Y_REG - postword;
        CC_C = temp16 > Y_REG;
        CC_V = OTEST16(CC_C, postword, temp16, Y_REG);//CC_C^((postword^temp16^Y_REG)>>15);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        instance->CycleCounter += 7;
        break;

      case LDY_D: //109E
        Y_REG = MemRead16((DP_REG | MemRead8(PC_REG++)));
        CC_Z = ZTEST(Y_REG);
        CC_N = NTEST16(Y_REG);
        CC_V = 0;
        instance->CycleCounter += 6;
        break;

      case STY_D: //109F
        MemWrite16(Y_REG, (DP_REG | MemRead8(PC_REG++)));
        CC_Z = ZTEST(Y_REG);
        CC_N = NTEST16(Y_REG);
        CC_V = 0;
        instance->CycleCounter += 6;
        break;

      case CMPD_X: //10A3
        postword = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
        temp16 = D_REG - postword;
        CC_C = temp16 > D_REG;
        CC_V = OTEST16(CC_C, postword, temp16, D_REG);//CC_C^((postword^temp16^D_REG)>>15);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        instance->CycleCounter += 7;
        break;

      case CMPY_X: //10AC
        postword = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
        temp16 = Y_REG - postword;
        CC_C = temp16 > Y_REG;
        CC_V = OTEST16(CC_C, postword, temp16, Y_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        instance->CycleCounter += 7;
        break;

      case LDY_X: //10AE
        Y_REG = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
        CC_Z = ZTEST(Y_REG);
        CC_N = NTEST16(Y_REG);
        CC_V = 0;
        instance->CycleCounter += 6;
        break;

      case STY_X: //10AF
        MemWrite16(Y_REG, MC6809_CalculateEA(MemRead8(PC_REG++)));
        CC_Z = ZTEST(Y_REG);
        CC_N = NTEST16(Y_REG);
        CC_V = 0;
        instance->CycleCounter += 6;
        break;

      case CMPD_E: //10B3
        postword = MemRead16(MemRead16(PC_REG));
        temp16 = D_REG - postword;
        CC_C = temp16 > D_REG;
        CC_V = OTEST16(CC_C, postword, temp16, D_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        PC_REG += 2;
        instance->CycleCounter += 8;
        break;

      case CMPY_E: //10BC
        postword = MemRead16(MemRead16(PC_REG));
        temp16 = Y_REG - postword;
        CC_C = temp16 > Y_REG;
        CC_V = OTEST16(CC_C, postword, temp16, Y_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        PC_REG += 2;
        instance->CycleCounter += 8;
        break;

      case LDY_E: //10BE
        Y_REG = MemRead16(MemRead16(PC_REG));
        CC_Z = ZTEST(Y_REG);
        CC_N = NTEST16(Y_REG);
        CC_V = 0;
        PC_REG += 2;
        instance->CycleCounter += 7;
        break;

      case STY_E: //10BF
        MemWrite16(Y_REG, MemRead16(PC_REG));
        CC_Z = ZTEST(Y_REG);
        CC_N = NTEST16(Y_REG);
        CC_V = 0;
        PC_REG += 2;
        instance->CycleCounter += 7;
        break;

      case LDS_I:  //10CE
        S_REG = MemRead16(PC_REG);
        CC_Z = ZTEST(S_REG);
        CC_N = NTEST16(S_REG);
        CC_V = 0;
        PC_REG += 2;
        instance->CycleCounter += 4;
        break;

      case LDS_D: //10DE
        S_REG = MemRead16((DP_REG | MemRead8(PC_REG++)));
        CC_Z = ZTEST(S_REG);
        CC_N = NTEST16(S_REG);
        CC_V = 0;
        instance->CycleCounter += 6;
        break;

      case STS_D: //10DF
        MemWrite16(S_REG, (DP_REG | MemRead8(PC_REG++)));
        CC_Z = ZTEST(S_REG);
        CC_N = NTEST16(S_REG);
        CC_V = 0;
        instance->CycleCounter += 6;
        break;

      case LDS_X: //10EE
        S_REG = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
        CC_Z = ZTEST(S_REG);
        CC_N = NTEST16(S_REG);
        CC_V = 0;
        instance->CycleCounter += 6;
        break;

      case STS_X: //10EF
        MemWrite16(S_REG, MC6809_CalculateEA(MemRead8(PC_REG++)));
        CC_Z = ZTEST(S_REG);
        CC_N = NTEST16(S_REG);
        CC_V = 0;
        instance->CycleCounter += 6;
        break;

      case LDS_E: //10FE
        S_REG = MemRead16(MemRead16(PC_REG));
        CC_Z = ZTEST(S_REG);
        CC_N = NTEST16(S_REG);
        CC_V = 0;
        PC_REG += 2;
        instance->CycleCounter += 7;
        break;

      case STS_E: //10FF
        MemWrite16(S_REG, MemRead16(PC_REG));
        CC_Z = ZTEST(S_REG);
        CC_N = NTEST16(S_REG);
        CC_V = 0;
        PC_REG += 2;
        instance->CycleCounter += 7;
        break;

      default:
        break;
      } //Page 2 switch END
      break;

    case	Page3:

      switch (MemRead8(PC_REG++))
      {
      case SWI3_I: //113F
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
        MemWrite8(B_REG, --S_REG);
        MemWrite8(A_REG, --S_REG);
        MemWrite8(MC6809_getcc(), --S_REG);

        PC_REG = MemRead16(VSWI3);
        instance->CycleCounter += 20;
        break;

      case CMPU_M: //1183
        postword = MemRead16(PC_REG);
        temp16 = U_REG - postword;
        CC_C = temp16 > U_REG;
        CC_V = OTEST16(CC_C, postword, temp16, U_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case CMPS_M: //118C
        postword = MemRead16(PC_REG);
        temp16 = S_REG - postword;
        CC_C = temp16 > S_REG;
        CC_V = OTEST16(CC_C, postword, temp16, S_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        PC_REG += 2;
        instance->CycleCounter += 5;
        break;

      case CMPU_D: //1193
        postword = MemRead16((DP_REG | MemRead8(PC_REG++)));
        temp16 = U_REG - postword;
        CC_C = temp16 > U_REG;
        CC_V = OTEST16(CC_C, postword, temp16, U_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        instance->CycleCounter += 7;
        break;

      case CMPS_D: //119C
        postword = MemRead16((DP_REG | MemRead8(PC_REG++)));
        temp16 = S_REG - postword;
        CC_C = temp16 > S_REG;
        CC_V = OTEST16(CC_C, postword, temp16, S_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        instance->CycleCounter += 7;
        break;

      case CMPU_X: //11A3
        postword = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
        temp16 = U_REG - postword;
        CC_C = temp16 > U_REG;
        CC_V = OTEST16(CC_C, postword, temp16, U_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        instance->CycleCounter += 7;
        break;

      case CMPS_X:  //11AC
        postword = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
        temp16 = S_REG - postword;
        CC_C = temp16 > S_REG;
        CC_V = OTEST16(CC_C, postword, temp16, S_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        instance->CycleCounter += 7;
        break;

      case CMPU_E: //11B3
        postword = MemRead16(MemRead16(PC_REG));
        temp16 = U_REG - postword;
        CC_C = temp16 > U_REG;
        CC_V = OTEST16(CC_C, postword, temp16, U_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        PC_REG += 2;
        instance->CycleCounter += 8;
        break;

      case CMPS_E: //11BC
        postword = MemRead16(MemRead16(PC_REG));
        temp16 = S_REG - postword;
        CC_C = temp16 > S_REG;
        CC_V = OTEST16(CC_C, postword, temp16, S_REG);
        CC_N = NTEST16(temp16);
        CC_Z = ZTEST(temp16);
        PC_REG += 2;
        instance->CycleCounter += 8;
        break;

      default:
        break;

      } //Page 3 switch END
      break;

    case NOP_I:	//12
      instance->CycleCounter += 2;
      break;

    case SYNC_I: //13
      instance->CycleCounter = cycleFor;
      instance->SyncWaiting = 1;
      break;

    case LBRA_R: //16
      *spostword = MemRead16(PC_REG);
      PC_REG += 2;
      PC_REG += *spostword;
      instance->CycleCounter += 5;
      break;

    case	LBSR_R: //17
      *spostword = MemRead16(PC_REG);
      PC_REG += 2;
      S_REG--;
      MemWrite8(PC_L, S_REG--);
      MemWrite8(PC_H, S_REG);
      PC_REG += *spostword;
      instance->CycleCounter += 9;
      break;

    case DAA_I: //19
      msn = (A_REG & 0xF0);
      lsn = (A_REG & 0xF);
      temp8 = 0;

      if (CC_H || (lsn > 9)) {
        temp8 |= 0x06;
      }

      if ((msn > 0x80) && (lsn > 9)) {
        temp8 |= 0x60;
      }

      if ((msn > 0x90) || CC_C) {
        temp8 |= 0x60;
      }

      temp16 = A_REG + temp8;
      CC_C |= ((temp16 & 0x100) >> 8);
      A_REG = temp16 & 0xFF;
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 2;
      break;

    case ORCC_M: //1A
      postbyte = MemRead8(PC_REG++);
      temp8 = MC6809_getcc();
      temp8 = (temp8 | postbyte);
      mc6809_setcc(temp8);
      instance->CycleCounter += 3;
      break;

    case ANDCC_M: //1C
      postbyte = MemRead8(PC_REG++);
      temp8 = MC6809_getcc();
      temp8 = (temp8 & postbyte);
      mc6809_setcc(temp8);
      instance->CycleCounter += 3;
      break;

    case SEX_I: //1D
      A_REG = 0 - (B_REG >> 7);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      instance->CycleCounter += 2;
      break;

    case EXG_M: //1E
      postbyte = MemRead8(PC_REG++);
      instance->ccbits = MC6809_getcc();

      if (((postbyte & 0x80) >> 4) == (postbyte & 0x08)) //Verify like size registers
      {
        if (postbyte & 0x08) //8 bit EXG
        {
          temp8 = PUR(((postbyte & 0x70) >> 4));
          PUR(((postbyte & 0x70) >> 4)) = PUR(postbyte & 0x07);
          PUR(postbyte & 0x07) = temp8;
        }
        else // 16 bit EXG
        {
          temp16 = PXF(((postbyte & 0x70) >> 4));
          PXF(((postbyte & 0x70) >> 4)) = PXF(postbyte & 0x07);
          PXF(postbyte & 0x07) = temp16;
        }
      }

      mc6809_setcc(instance->ccbits);
      instance->CycleCounter += 8;
      break;

    case TFR_M: //1F
      postbyte = MemRead8(PC_REG++);
      source = postbyte >> 4;
      dest = postbyte & 15;

      switch (dest)
      {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        PXF(dest) = 0xFFFF;

        if ((source == 12) || (source == 13))
        {
          PXF(dest) = 0;
        }
        else if (source <= 7)
        {
          //make sure the source is valud
          if (instance->xfreg16[source])
          {
            PXF(dest) = PXF(source);
          }
        }
        break;

      case 8:
      case 9:
      case 10:
      case 11:
      case 14:
      case 15:
        instance->ccbits = MC6809_getcc();
        PUR(dest & 7) = 0xFF;

        if ((source == 12) || (source == 13)) {
          PUR(dest & 7) = 0;
        }
        else if (source > 7) {
          PUR(dest & 7) = PUR(source & 7);
        }

        mc6809_setcc(instance->ccbits);
        break;
      }
      instance->CycleCounter += 6;
      break;

    case BRA_R: //20
      *spostbyte = MemRead8(PC_REG++);
      PC_REG += *spostbyte;
      instance->CycleCounter += 3;
      break;

    case BRN_R: //21
      instance->CycleCounter += 3;
      PC_REG++;
      break;

    case BHI_R: //22
      if (!(CC_C | CC_Z)) {
        PC_REG += (signed char)MemRead8(PC_REG);
      }

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BLS_R: //23
      if (CC_C | CC_Z) {
        PC_REG += (signed char)MemRead8(PC_REG);
      }

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BHS_R: //24
      if (!CC_C) {
        PC_REG += (signed char)MemRead8(PC_REG);
      }

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BLO_R: //25
      if (CC_C) {
        PC_REG += (signed char)MemRead8(PC_REG);
      }

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BNE_R: //26
      if (!CC_Z) {
        PC_REG += (signed char)MemRead8(PC_REG);
      }

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BEQ_R: //27
      if (CC_Z) {
        PC_REG += (signed char)MemRead8(PC_REG);
      }

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BVC_R: //28
      if (!CC_V) {
        PC_REG += (signed char)MemRead8(PC_REG);
      }

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BVS_R: //29
      if (CC_V) {
        PC_REG += (signed char)MemRead8(PC_REG);
      }

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BPL_R: //2A
      if (!CC_N) {
        PC_REG += (signed char)MemRead8(PC_REG);
      }

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BMI_R: //2B
      if (CC_N)
        PC_REG += (signed char)MemRead8(PC_REG);

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BGE_R: //2C
      if (!(CC_N ^ CC_V))
        PC_REG += (signed char)MemRead8(PC_REG);

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BLT_R: //2D
      if (CC_V ^ CC_N)
        PC_REG += (signed char)MemRead8(PC_REG);

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BGT_R: //2E
      if (!(CC_Z | (CC_N ^ CC_V)))
        PC_REG += (signed char)MemRead8(PC_REG);

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case BLE_R: //2F
      if (CC_Z | (CC_N ^ CC_V))
        PC_REG += (signed char)MemRead8(PC_REG);

      PC_REG++;
      instance->CycleCounter += 3;
      break;

    case LEAX_X: //30
      X_REG = MC6809_CalculateEA(MemRead8(PC_REG++));
      CC_Z = ZTEST(X_REG);
      instance->CycleCounter += 4;
      break;

    case LEAY_X: //31
      Y_REG = MC6809_CalculateEA(MemRead8(PC_REG++));
      CC_Z = ZTEST(Y_REG);
      instance->CycleCounter += 4;
      break;

    case LEAS_X: //32
      S_REG = MC6809_CalculateEA(MemRead8(PC_REG++));
      instance->CycleCounter += 4;
      break;

    case LEAU_X: //33
      U_REG = MC6809_CalculateEA(MemRead8(PC_REG++));
      instance->CycleCounter += 4;
      break;

    case PSHS_M: //34
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
        MemWrite8(MC6809_getcc(), --S_REG);
        instance->CycleCounter += 1;
      }

      instance->CycleCounter += 5;
      break;

    case PULS_M: //35
      postbyte = MemRead8(PC_REG++);

      if (postbyte & 0x01)
      {
        mc6809_setcc(MemRead8(S_REG++));
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

      instance->CycleCounter += 5;
      break;

    case PSHU_M: //36
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
        MemWrite8(MC6809_getcc(), --U_REG);
        instance->CycleCounter += 1;
      }

      instance->CycleCounter += 5;
      break;

    case PULU_M: //37
      postbyte = MemRead8(PC_REG++);

      if (postbyte & 0x01)
      {
        mc6809_setcc(MemRead8(U_REG++));
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

      instance->CycleCounter += 5;
      break;

    case RTS_I: //39
      PC_H = MemRead8(S_REG++);
      PC_L = MemRead8(S_REG++);
      instance->CycleCounter += 5;
      break;

    case ABX_I: //3A
      X_REG = X_REG + B_REG;
      instance->CycleCounter += 3;
      break;

    case RTI_I: //3B
      mc6809_setcc(MemRead8(S_REG++));
      instance->CycleCounter += 6;
      instance->InInterrupt = 0;

      if (CC_E)
      {
        A_REG = MemRead8(S_REG++);
        B_REG = MemRead8(S_REG++);
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
      break;

    case CWAI_I: //3C
      postbyte = MemRead8(PC_REG++);

      instance->ccbits = MC6809_getcc();
      instance->ccbits = instance->ccbits & postbyte;
      mc6809_setcc(instance->ccbits);

      instance->CycleCounter = cycleFor;
      instance->SyncWaiting = 1;
      break;

    case MUL_I: //3D
      D_REG = A_REG * B_REG;
      CC_C = B_REG > 0x7F;
      CC_Z = ZTEST(D_REG);
      instance->CycleCounter += 11;
      break;

    case RESET:	//Undocumented
      MC6809Reset();
      break;

    case SWI1_I: //3F
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
      MemWrite8(B_REG, --S_REG);
      MemWrite8(A_REG, --S_REG);
      MemWrite8(MC6809_getcc(), --S_REG);
      PC_REG = MemRead16(VSWI);
      instance->CycleCounter += 19;
      CC_I = 1;
      CC_F = 1;
      break;

    case NEGA_I: //40
      temp8 = 0 - A_REG;
      CC_C = temp8 > 0;
      CC_V = A_REG == 0x80; //CC_C ^ ((A_REG^temp8)>>7);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      A_REG = temp8;
      instance->CycleCounter += 2;
      break;

    case COMA_I: //43
      A_REG = 0xFF - A_REG;
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      CC_C = 1;
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case LSRA_I: //44
      CC_C = A_REG & 1;
      A_REG = A_REG >> 1;
      CC_Z = ZTEST(A_REG);
      CC_N = 0;
      instance->CycleCounter += 2;
      break;

    case RORA_I: //46
      postbyte = CC_C << 7;
      CC_C = A_REG & 1;
      A_REG = (A_REG >> 1) | postbyte;
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      instance->CycleCounter += 2;
      break;

    case ASRA_I: //47
      CC_C = A_REG & 1;
      A_REG = (A_REG & 0x80) | (A_REG >> 1);
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      instance->CycleCounter += 2;
      break;

    case ASLA_I: //48 JF
      CC_C = A_REG > 0x7F;
      CC_V = CC_C ^ ((A_REG & 0x40) >> 6);
      A_REG = A_REG << 1;
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 2;
      break;

    case ROLA_I: //49
      postbyte = CC_C;
      CC_C = A_REG > 0x7F;
      CC_V = CC_C ^ ((A_REG & 0x40) >> 6);
      A_REG = (A_REG << 1) | postbyte;
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      instance->CycleCounter += 2;
      break;

    case DECA_I: //4A
      A_REG--;
      CC_Z = ZTEST(A_REG);
      CC_V = A_REG == 0x7F;
      CC_N = NTEST8(A_REG);
      instance->CycleCounter += 2;
      break;

    case INCA_I: //4C
      A_REG++;
      CC_Z = ZTEST(A_REG);
      CC_V = A_REG == 0x80;
      CC_N = NTEST8(A_REG);
      instance->CycleCounter += 2;
      break;

    case TSTA_I: //4D
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case CLRA_I: //4F
      A_REG = 0;
      CC_C = 0;
      CC_V = 0;
      CC_N = 0;
      CC_Z = 1;
      instance->CycleCounter += 2;
      break;

    case NEGB_I: //50
      temp8 = 0 - B_REG;
      CC_C = temp8 > 0;
      CC_V = B_REG == 0x80;
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      B_REG = temp8;
      instance->CycleCounter += 2;
      break;

    case COMB_I: //53
      B_REG = 0xFF - B_REG;
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      CC_C = 1;
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case LSRB_I: //54
      CC_C = B_REG & 1;
      B_REG = B_REG >> 1;
      CC_Z = ZTEST(B_REG);
      CC_N = 0;
      instance->CycleCounter += 2;
      break;

    case RORB_I: //56
      postbyte = CC_C << 7;
      CC_C = B_REG & 1;
      B_REG = (B_REG >> 1) | postbyte;
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      instance->CycleCounter += 2;
      break;

    case ASRB_I: //57
      CC_C = B_REG & 1;
      B_REG = (B_REG & 0x80) | (B_REG >> 1);
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      instance->CycleCounter += 2;
      break;

    case ASLB_I: //58
      CC_C = B_REG > 0x7F;
      CC_V = CC_C ^ ((B_REG & 0x40) >> 6);
      B_REG = B_REG << 1;
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 2;
      break;

    case ROLB_I: //59
      postbyte = CC_C;
      CC_C = B_REG > 0x7F;
      CC_V = CC_C ^ ((B_REG & 0x40) >> 6);
      B_REG = (B_REG << 1) | postbyte;
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      instance->CycleCounter += 2;
      break;

    case DECB_I: //5A
      B_REG--;
      CC_Z = ZTEST(B_REG);
      CC_V = B_REG == 0x7F;
      CC_N = NTEST8(B_REG);
      instance->CycleCounter += 2;
      break;

    case INCB_I: //5C
      B_REG++;
      CC_Z = ZTEST(B_REG);
      CC_V = B_REG == 0x80;
      CC_N = NTEST8(B_REG);
      instance->CycleCounter += 2;
      break;

    case TSTB_I: //5D
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case CLRB_I: //5F
      B_REG = 0;
      CC_C = 0;
      CC_N = 0;
      CC_V = 0;
      CC_Z = 1;
      instance->CycleCounter += 2;
      break;

    case NEG_X: //60
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      postbyte = MemRead8(temp16);
      temp8 = 0 - postbyte;
      CC_C = temp8 > 0;
      CC_V = postbyte == 0x80;
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case COM_X: //63
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      temp8 = 0xFF - temp8;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      CC_V = 0;
      CC_C = 1;
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case LSR_X: //64
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      CC_C = temp8 & 1;
      temp8 = temp8 >> 1;
      CC_Z = ZTEST(temp8);
      CC_N = 0;
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case ROR_X: //66
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      postbyte = CC_C << 7;
      CC_C = (temp8 & 1);
      temp8 = (temp8 >> 1) | postbyte;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case ASR_X: //67
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      CC_C = temp8 & 1;
      temp8 = (temp8 & 0x80) | (temp8 >> 1);
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case ASL_X: //68
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      CC_C = temp8 > 0x7F;
      CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
      temp8 = temp8 << 1;
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case ROL_X: //69
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      postbyte = CC_C;
      CC_C = temp8 > 0x7F;
      CC_V = (CC_C ^ ((temp8 & 0x40) >> 6));
      temp8 = ((temp8 << 1) | postbyte);
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case DEC_X: //6A
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      temp8--;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      CC_V = (temp8 == 0x7F);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case INC_X: //6C
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      temp8 = MemRead8(temp16);
      temp8++;
      CC_V = (temp8 == 0x80);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      MemWrite8(temp8, temp16);
      instance->CycleCounter += 6;
      break;

    case TST_X: //6D
      temp8 = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      CC_V = 0;
      instance->CycleCounter += 6;
      break;

    case JMP_X: //6E
      PC_REG = MC6809_CalculateEA(MemRead8(PC_REG++));
      instance->CycleCounter += 3;
      break;

    case CLR_X: //6F
      MemWrite8(0, MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_C = 0;
      CC_N = 0;
      CC_V = 0;
      CC_Z = 1;
      instance->CycleCounter += 6;
      break;

    case NEG_E: //70
      temp16 = MemRead16(PC_REG);
      postbyte = MemRead8(temp16);
      temp8 = 0 - postbyte;
      CC_C = temp8 > 0;
      CC_V = postbyte == 0x80;
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      MemWrite8(temp8, temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case COM_E: //73
      temp16 = MemRead16(PC_REG);
      temp8 = MemRead8(temp16);
      temp8 = 0xFF - temp8;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      CC_C = 1;
      CC_V = 0;
      MemWrite8(temp8, temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case LSR_E:  //74
      temp16 = MemRead16(PC_REG);
      temp8 = MemRead8(temp16);
      CC_C = temp8 & 1;
      temp8 = temp8 >> 1;
      CC_Z = ZTEST(temp8);
      CC_N = 0;
      MemWrite8(temp8, temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case ROR_E: //76
      temp16 = MemRead16(PC_REG);
      temp8 = MemRead8(temp16);
      postbyte = CC_C << 7;
      CC_C = temp8 & 1;
      temp8 = (temp8 >> 1) | postbyte;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case ASR_E: //77
      temp16 = MemRead16(PC_REG);
      temp8 = MemRead8(temp16);
      CC_C = temp8 & 1;
      temp8 = (temp8 & 0x80) | (temp8 >> 1);
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case ASL_E: //78
      temp16 = MemRead16(PC_REG);
      temp8 = MemRead8(temp16);
      CC_C = temp8 > 0x7F;
      CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
      temp8 = temp8 << 1;
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      MemWrite8(temp8, temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case ROL_E: //79
      temp16 = MemRead16(PC_REG);
      temp8 = MemRead8(temp16);
      postbyte = CC_C;
      CC_C = temp8 > 0x7F;
      CC_V = CC_C ^ ((temp8 & 0x40) >> 6);
      temp8 = ((temp8 << 1) | postbyte);
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case DEC_E: //7A
      temp16 = MemRead16(PC_REG);
      temp8 = MemRead8(temp16);
      temp8--;
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      CC_V = temp8 == 0x7F;
      MemWrite8(temp8, temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case INC_E: //7C
      temp16 = MemRead16(PC_REG);
      temp8 = MemRead8(temp16);
      temp8++;
      CC_Z = ZTEST(temp8);
      CC_V = temp8 == 0x80;
      CC_N = NTEST8(temp8);
      MemWrite8(temp8, temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case TST_E: //7D
      temp8 = MemRead8(MemRead16(PC_REG));
      CC_Z = ZTEST(temp8);
      CC_N = NTEST8(temp8);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case JMP_E: //7E
      PC_REG = MemRead16(PC_REG);
      instance->CycleCounter += 4;
      break;

    case CLR_E: //7F
      MemWrite8(0, MemRead16(PC_REG));
      CC_C = 0;
      CC_N = 0;
      CC_V = 0;
      CC_Z = 1;
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case SUBA_M: //80
      postbyte = MemRead8(PC_REG++);
      temp16 = A_REG - postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      instance->CycleCounter += 2;
      break;

    case CMPA_M: //81
      postbyte = MemRead8(PC_REG++);
      temp8 = A_REG - postbyte;
      CC_C = temp8 > A_REG;
      CC_V = OTEST8(CC_C, postbyte, temp8, A_REG);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      instance->CycleCounter += 2;
      break;

    case SBCA_M:  //82
      postbyte = MemRead8(PC_REG++);
      temp16 = A_REG - postbyte - CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 2;
      break;

    case SUBD_M: //83
      temp16 = MemRead16(PC_REG);
      temp32 = D_REG - temp16;
      CC_C = (temp32 & 0x10000) >> 16;
      CC_V = OTEST16(CC_C, temp32, temp16, D_REG);
      D_REG = (temp32 & 0xFFFF);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      PC_REG += 2;
      instance->CycleCounter += 4;
      break;

    case ANDA_M: //84
      A_REG = A_REG & MemRead8(PC_REG++);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case BITA_M: //85
      temp8 = A_REG & MemRead8(PC_REG++);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case LDA_M: //86
      A_REG = MemRead8(PC_REG++);
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case EORA_M: //88
      A_REG = A_REG ^ MemRead8(PC_REG++);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case ADCA_M: //89
      postbyte = MemRead8(PC_REG++);
      temp16 = A_REG + postbyte + CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      CC_H = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 2;
      break;

    case ORA_M: //8A
      A_REG = A_REG | MemRead8(PC_REG++);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case ADDA_M: //8B
      postbyte = MemRead8(PC_REG++);
      temp16 = A_REG + postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_H = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 2;
      break;

    case CMPX_M: //8C
      postword = MemRead16(PC_REG);
      temp16 = X_REG - postword;
      CC_C = temp16 > X_REG;
      CC_V = OTEST16(CC_C, postword, temp16, X_REG);
      CC_N = NTEST16(temp16);
      CC_Z = ZTEST(temp16);
      PC_REG += 2;
      instance->CycleCounter += 4;
      break;

    case BSR_R: //8D
      *spostbyte = MemRead8(PC_REG++);
      S_REG--;
      MemWrite8(PC_L, S_REG--);
      MemWrite8(PC_H, S_REG);
      PC_REG += *spostbyte;
      instance->CycleCounter += 7;
      break;

    case LDX_M: //8E
      X_REG = MemRead16(PC_REG);
      CC_Z = ZTEST(X_REG);
      CC_N = NTEST16(X_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 3;
      break;

    case SUBA_D: //90
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp16 = A_REG - postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      instance->CycleCounter += 4;
      break;

    case CMPA_D: //91
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp8 = A_REG - postbyte;
      CC_C = temp8 > A_REG;
      CC_V = OTEST8(CC_C, postbyte, temp8, A_REG);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      instance->CycleCounter += 4;
      break;

    case SBCA_D: //92
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp16 = A_REG - postbyte - CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 4;
      break;

    case SUBD_D: //93
      temp16 = MemRead16((DP_REG | MemRead8(PC_REG++)));
      temp32 = D_REG - temp16;
      CC_C = (temp32 & 0x10000) >> 16;
      CC_V = OTEST16(CC_C, temp32, temp16, D_REG);
      D_REG = (temp32 & 0xFFFF);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      instance->CycleCounter += 6;
      break;

    case ANDA_D: //94
      A_REG = A_REG & MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case BITA_D: //95
      temp8 = A_REG & MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case LDA_D: //96
      A_REG = MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case STA_D: //97
      MemWrite8(A_REG, (DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case EORA_D: //98
      A_REG = A_REG ^ MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case ADCA_D: //99
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp16 = A_REG + postbyte + CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      CC_H = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 4;
      break;

    case ORA_D: //9A
      A_REG = A_REG | MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case ADDA_D: //9B
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp16 = A_REG + postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_H = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 4;
      break;

    case CMPX_D: //9C
      postword = MemRead16((DP_REG | MemRead8(PC_REG++)));
      temp16 = X_REG - postword;
      CC_C = temp16 > X_REG;
      CC_V = OTEST16(CC_C, postword, temp16, X_REG);
      CC_N = NTEST16(temp16);
      CC_Z = ZTEST(temp16);
      instance->CycleCounter += 6;
      break;

    case BSR_D: //9D
      temp16 = (DP_REG | MemRead8(PC_REG++));
      S_REG--;
      MemWrite8(PC_L, S_REG--);
      MemWrite8(PC_H, S_REG);
      PC_REG = temp16;
      instance->CycleCounter += 7;
      break;

    case LDX_D: //9E
      X_REG = MemRead16((DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(X_REG);
      CC_N = NTEST16(X_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case STX_D: //9F
      MemWrite16(X_REG, (DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(X_REG);
      CC_N = NTEST16(X_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case SUBA_X: //A0
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp16 = A_REG - postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      instance->CycleCounter += 4;
      break;

    case CMPA_X: //A1
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp8 = A_REG - postbyte;
      CC_C = temp8 > A_REG;
      CC_V = OTEST8(CC_C, postbyte, temp8, A_REG);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      instance->CycleCounter += 4;
      break;

    case SBCA_X: //A2
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp16 = A_REG - postbyte - CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 4;
      break;

    case SUBD_X: //A3
      temp16 = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp32 = D_REG - temp16;
      CC_C = (temp32 & 0x10000) >> 16;
      CC_V = OTEST16(CC_C, temp32, temp16, D_REG);
      D_REG = (temp32 & 0xFFFF);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      instance->CycleCounter += 6;
      break;

    case ANDA_X: //A4
      A_REG = A_REG & MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case BITA_X:  //A5
      temp8 = A_REG & MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case LDA_X: //A6
      A_REG = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case STA_X: //A7
      MemWrite8(A_REG, MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case EORA_X: //A8
      A_REG = A_REG ^ MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case ADCA_X: //A9
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp16 = A_REG + postbyte + CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      CC_H = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 4;
      break;

    case ORA_X: //AA
      A_REG = A_REG | MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case ADDA_X: //AB
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp16 = A_REG + postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_H = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      instance->CycleCounter += 4;
      break;

    case CMPX_X: //AC
      postword = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp16 = X_REG - postword;
      CC_C = temp16 > X_REG;
      CC_V = OTEST16(CC_C, postword, temp16, X_REG);
      CC_N = NTEST16(temp16);
      CC_Z = ZTEST(temp16);
      instance->CycleCounter += 6;
      break;

    case BSR_X: //AD
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      S_REG--;
      MemWrite8(PC_L, S_REG--);
      MemWrite8(PC_H, S_REG);
      PC_REG = temp16;
      instance->CycleCounter += 7;
      break;

    case LDX_X: //AE
      X_REG = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(X_REG);
      CC_N = NTEST16(X_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case STX_X: //AF
      MemWrite16(X_REG, MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(X_REG);
      CC_N = NTEST16(X_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case SUBA_E: //B0
      postbyte = MemRead8(MemRead16(PC_REG));
      temp16 = A_REG - postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case	CMPA_E: //B1
      postbyte = MemRead8(MemRead16(PC_REG));
      temp8 = A_REG - postbyte;
      CC_C = temp8 > A_REG;
      CC_V = OTEST8(CC_C, postbyte, temp8, A_REG);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case SBCA_E: //B2
      postbyte = MemRead8(MemRead16(PC_REG));
      temp16 = A_REG - postbyte - CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case SUBD_E: //B3
      temp16 = MemRead16(MemRead16(PC_REG));
      temp32 = D_REG - temp16;
      CC_C = (temp32 & 0x10000) >> 16;
      CC_V = OTEST16(CC_C, temp32, temp16, D_REG);
      D_REG = (temp32 & 0xFFFF);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case ANDA_E: //B4
      postbyte = MemRead8(MemRead16(PC_REG));
      A_REG = A_REG & postbyte;
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case BITA_E: //B5
      temp8 = A_REG & MemRead8(MemRead16(PC_REG));
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case LDA_E: //B6
      A_REG = MemRead8(MemRead16(PC_REG));
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case STA_E: //B7
      MemWrite8(A_REG, MemRead16(PC_REG));
      CC_Z = ZTEST(A_REG);
      CC_N = NTEST8(A_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case EORA_E:  //B8
      A_REG = A_REG ^ MemRead8(MemRead16(PC_REG));
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case ADCA_E: //B9
      postbyte = MemRead8(MemRead16(PC_REG));
      temp16 = A_REG + postbyte + CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      CC_H = ((A_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case ORA_E: //BA
      A_REG = A_REG | MemRead8(MemRead16(PC_REG));
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case ADDA_E: //BB
      postbyte = MemRead8(MemRead16(PC_REG));
      temp16 = A_REG + postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_H = ((A_REG ^ postbyte ^ temp16) & 0x10) >> 4;
      CC_V = OTEST8(CC_C, postbyte, temp16, A_REG);
      A_REG = (temp16 & 0xFF);
      CC_N = NTEST8(A_REG);
      CC_Z = ZTEST(A_REG);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case CMPX_E: //BC
      postword = MemRead16(MemRead16(PC_REG));
      temp16 = X_REG - postword;
      CC_C = temp16 > X_REG;
      CC_V = OTEST16(CC_C, postword, temp16, X_REG);
      CC_N = NTEST16(temp16);
      CC_Z = ZTEST(temp16);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case BSR_E: //BD
      postword = MemRead16(PC_REG);
      PC_REG += 2;
      S_REG--;
      MemWrite8(PC_L, S_REG--);
      MemWrite8(PC_H, S_REG);
      PC_REG = postword;
      instance->CycleCounter += 8;
      break;

    case LDX_E: //BE
      X_REG = MemRead16(MemRead16(PC_REG));
      CC_Z = ZTEST(X_REG);
      CC_N = NTEST16(X_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 6;
      break;

    case STX_E: //BF
      MemWrite16(X_REG, MemRead16(PC_REG));
      CC_Z = ZTEST(X_REG);
      CC_N = NTEST16(X_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 6;
      break;

    case SUBB_M: //C0
      postbyte = MemRead8(PC_REG++);
      temp16 = B_REG - postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      instance->CycleCounter += 2;
      break;

    case CMPB_M: //C1
      postbyte = MemRead8(PC_REG++);
      temp8 = B_REG - postbyte;
      CC_C = temp8 > B_REG;
      CC_V = OTEST8(CC_C, postbyte, temp8, B_REG);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      instance->CycleCounter += 2;
      break;

    case SBCB_M: //C3
      postbyte = MemRead8(PC_REG++);
      temp16 = B_REG - postbyte - CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 2;
      break;

    case ADDD_M: //C3
      temp16 = MemRead16(PC_REG);
      temp32 = D_REG + temp16;
      CC_C = (temp32 & 0x10000) >> 16;
      CC_V = OTEST16(CC_C, temp32, temp16, D_REG);
      D_REG = (temp32 & 0xFFFF);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      PC_REG += 2;
      instance->CycleCounter += 4;
      break;

    case ANDB_M: //C4
      B_REG = B_REG & MemRead8(PC_REG++);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case BITB_M: //C5
      temp8 = B_REG & MemRead8(PC_REG++);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case LDB_M: //C6
      B_REG = MemRead8(PC_REG++);
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case EORB_M: //C8
      B_REG = B_REG ^ MemRead8(PC_REG++);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case ADCB_M: //C9
      postbyte = MemRead8(PC_REG++);
      temp16 = B_REG + postbyte + CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      CC_H = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 2;
      break;

    case ORB_M: //CA
      B_REG = B_REG | MemRead8(PC_REG++);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      instance->CycleCounter += 2;
      break;

    case ADDB_M: //CB
      postbyte = MemRead8(PC_REG++);
      temp16 = B_REG + postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_H = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 2;
      break;

    case LDD_M: //CC
      D_REG = MemRead16(PC_REG);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 3;
      break;

    case LDU_M: //CE
      U_REG = MemRead16(PC_REG);
      CC_Z = ZTEST(U_REG);
      CC_N = NTEST16(U_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 3;
      break;

    case SUBB_D: //D0
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp16 = B_REG - postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      instance->CycleCounter += 4;
      break;

    case CMPB_D: //D1
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp8 = B_REG - postbyte;
      CC_C = temp8 > B_REG;
      CC_V = OTEST8(CC_C, postbyte, temp8, B_REG);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      instance->CycleCounter += 4;
      break;

    case SBCB_D: //D2
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp16 = B_REG - postbyte - CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 4;
      break;

    case ADDD_D: //D3
      temp16 = MemRead16((DP_REG | MemRead8(PC_REG++)));
      temp32 = D_REG + temp16;
      CC_C = (temp32 & 0x10000) >> 16;
      CC_V = OTEST16(CC_C, temp32, temp16, D_REG);
      D_REG = (temp32 & 0xFFFF);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      instance->CycleCounter += 6;
      break;

    case ANDB_D: //D4 
      B_REG = B_REG & MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case BITB_D: //D5
      temp8 = B_REG & MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case LDB_D: //D6
      B_REG = MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case	STB_D: //D7
      MemWrite8(B_REG, (DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case EORB_D: //D8	
      B_REG = B_REG ^ MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case ADCB_D: //D9
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp16 = B_REG + postbyte + CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      CC_H = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 4;
      break;

    case ORB_D: //DA
      B_REG = B_REG | MemRead8((DP_REG | MemRead8(PC_REG++)));
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case ADDB_D: //DB
      postbyte = MemRead8((DP_REG | MemRead8(PC_REG++)));
      temp16 = B_REG + postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_H = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 4;
      break;

    case LDD_D: //DC
      D_REG = MemRead16((DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case STD_D: //DD
      MemWrite16(D_REG, (DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case LDU_D: //DE
      U_REG = MemRead16((DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(U_REG);
      CC_N = NTEST16(U_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case STU_D: //DF
      MemWrite16(U_REG, (DP_REG | MemRead8(PC_REG++)));
      CC_Z = ZTEST(U_REG);
      CC_N = NTEST16(U_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case SUBB_X: //E0
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp16 = B_REG - postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      instance->CycleCounter += 4;
      break;

    case CMPB_X: //E1
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp8 = B_REG - postbyte;
      CC_C = temp8 > B_REG;
      CC_V = OTEST8(CC_C, postbyte, temp8, B_REG);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      instance->CycleCounter += 4;
      break;

    case SBCB_X: //E2
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp16 = B_REG - postbyte - CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 4;
      break;

    case ADDD_X: //E3 
      temp16 = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp32 = D_REG + temp16;
      CC_C = (temp32 & 0x10000) >> 16;
      CC_V = OTEST16(CC_C, temp32, temp16, D_REG);
      D_REG = (temp32 & 0xFFFF);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      instance->CycleCounter += 6;
      break;

    case ANDB_X: //E4
      B_REG = B_REG & MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case BITB_X: //E5 
      temp8 = B_REG & MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case LDB_X: //E6
      B_REG = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case STB_X: //E7
      MemWrite8(B_REG, MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case EORB_X: //E8
      temp8 = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      B_REG = B_REG ^ temp8;
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;
      break;

    case ADCB_X: //E9
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp16 = B_REG + postbyte + CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      CC_H = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 4;
      break;

    case ORB_X: //EA 
      B_REG = B_REG | MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      instance->CycleCounter += 4;

      break;

    case ADDB_X: //EB
      postbyte = MemRead8(MC6809_CalculateEA(MemRead8(PC_REG++)));
      temp16 = B_REG + postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_H = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      instance->CycleCounter += 4;
      break;

    case LDD_X: //EC
      temp16 = MC6809_CalculateEA(MemRead8(PC_REG++));
      D_REG = MemRead16(temp16);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case STD_X: //ED
      MemWrite16(D_REG, MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case LDU_X: //EE
      U_REG = MemRead16(MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(U_REG);
      CC_N = NTEST16(U_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case STU_X: //EF
      MemWrite16(U_REG, MC6809_CalculateEA(MemRead8(PC_REG++)));
      CC_Z = ZTEST(U_REG);
      CC_N = NTEST16(U_REG);
      CC_V = 0;
      instance->CycleCounter += 5;
      break;

    case SUBB_E: //F0
      postbyte = MemRead8(MemRead16(PC_REG));
      temp16 = B_REG - postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case CMPB_E: //F1
      postbyte = MemRead8(MemRead16(PC_REG));
      temp8 = B_REG - postbyte;
      CC_C = temp8 > B_REG;
      CC_V = OTEST8(CC_C, postbyte, temp8, B_REG);
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case SBCB_E: //F2
      postbyte = MemRead8(MemRead16(PC_REG));
      temp16 = B_REG - postbyte - CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case ADDD_E: //F3
      temp16 = MemRead16(MemRead16(PC_REG));
      temp32 = D_REG + temp16;
      CC_C = (temp32 & 0x10000) >> 16;
      CC_V = OTEST16(CC_C, temp32, temp16, D_REG);
      D_REG = (temp32 & 0xFFFF);
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      PC_REG += 2;
      instance->CycleCounter += 7;
      break;

    case ANDB_E:  //F4
      B_REG = B_REG & MemRead8(MemRead16(PC_REG));
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case BITB_E: //F5
      temp8 = B_REG & MemRead8(MemRead16(PC_REG));
      CC_N = NTEST8(temp8);
      CC_Z = ZTEST(temp8);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case LDB_E: //F6
      B_REG = MemRead8(MemRead16(PC_REG));
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case STB_E: //F7 
      MemWrite8(B_REG, MemRead16(PC_REG));
      CC_Z = ZTEST(B_REG);
      CC_N = NTEST8(B_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case EORB_E: //F8
      B_REG = B_REG ^ MemRead8(MemRead16(PC_REG));
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case ADCB_E: //F9
      postbyte = MemRead8(MemRead16(PC_REG));
      temp16 = B_REG + postbyte + CC_C;
      CC_C = (temp16 & 0x100) >> 8;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      CC_H = ((B_REG ^ temp16 ^ postbyte) & 0x10) >> 4;
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case ORB_E: //FA
      B_REG = B_REG | MemRead8(MemRead16(PC_REG));
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case ADDB_E: //FB
      postbyte = MemRead8(MemRead16(PC_REG));
      temp16 = B_REG + postbyte;
      CC_C = (temp16 & 0x100) >> 8;
      CC_H = ((B_REG ^ postbyte ^ temp16) & 0x10) >> 4;
      CC_V = OTEST8(CC_C, postbyte, temp16, B_REG);
      B_REG = (temp16 & 0xFF);
      CC_N = NTEST8(B_REG);
      CC_Z = ZTEST(B_REG);
      PC_REG += 2;
      instance->CycleCounter += 5;
      break;

    case LDD_E: //FC
      D_REG = MemRead16(MemRead16(PC_REG));
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 6;
      break;

    case STD_E: //FD
      MemWrite16(D_REG, MemRead16(PC_REG));
      CC_Z = ZTEST(D_REG);
      CC_N = NTEST16(D_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 6;
      break;

    case LDU_E: //FE
      U_REG = MemRead16(MemRead16(PC_REG));
      CC_Z = ZTEST(U_REG);
      CC_N = NTEST16(U_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 6;
      break;

    case STU_E: //FF
      MemWrite16(U_REG, MemRead16(PC_REG));
      CC_Z = ZTEST(U_REG);
      CC_N = NTEST16(U_REG);
      CC_V = 0;
      PC_REG += 2;
      instance->CycleCounter += 6;
      break;

    default:
      break;
    }
  }
}