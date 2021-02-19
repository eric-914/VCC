#include "GetMuxState.h"
#include "DACState.h"
#include "JoystickAccessors.h"

#include "library/joystickstate.h"
#include "library/keyboardstate.h"

/*
  Get CoCo 'scan' code

  Only called from MC6821.c to read the keyboard/joystick state

  should be a push instead of a pull?
*/
extern "C" {
  unsigned char vccKeyboardGetScan(unsigned char column)
  {
    unsigned char temp;
    unsigned char mask = 1;
    unsigned char ret_val = 0;

    KeyboardState* keyboardState = GetKeyBoardState();
    JoystickState* joystickState = GetJoystickState();

    temp = ~column; //Get column

    for (unsigned char x = 0; x < 8; x++)
    {
      if ((temp & mask)) // Found an active column scan
      {
        ret_val |= keyboardState->RolloverTable[x];
      }

      mask = (mask << 1);
    }

    ret_val = 127 - ret_val;

    //Collect CA2 and CB2 from the PIA (1of4 Multiplexer)
    joystickState->StickValue = get_pot_value(GetMuxState());

    if (joystickState->StickValue != 0)		//OS9 joyin routine needs this (koronis rift works now)
    {
      if (joystickState->StickValue >= DACState())		// Set bit of stick >= DAC output $FF20 Bits 7-2
      {
        ret_val |= 0x80;
      }
    }

    if (joystickState->LeftButton1Status == 1)
    {
      //Left Joystick Button 1 Down?
      ret_val = ret_val & 0xFD;
    }

    if (joystickState->RightButton1Status == 1)
    {
      //Right Joystick Button 1 Down?
      ret_val = ret_val & 0xFE;
    }

    if (joystickState->LeftButton2Status == 1)
    {
      //Left Joystick Button 2 Down?
      ret_val = ret_val & 0xF7;
    }

    if (joystickState->RightButton2Status == 1)
    {
      //Right Joystick Button 2 Down?
      ret_val = ret_val & 0xFB;
    }

#if 0 // no noticible change when this is disabled
    // TODO: move to MC6821/GIME
    {
      /** another keyboard IRQ flag - this should really be in the GIME code*/
      static unsigned char IrqFlag = 0;
      if ((ret_val & 0x7F) != 0x7F)
      {
        if ((IrqFlag == 0) & GimeGetKeyboardInterruptState())
        {
          GimeAssertKeyboardInterrupt();
          IrqFlag = 1;
        }
      }
      else
      {
        IrqFlag = 0;
      }
    }
#endif

    return (ret_val);
  }
}