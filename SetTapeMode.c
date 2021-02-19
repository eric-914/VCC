#include "library/cassettedef.h"
#include "library/cassettestate.h"

//--CASSETTE--//

#include "LoadTape.h"
#include "Motor.h"
#include "CloseTapeFile.h"
#include "UpdateTapeCounter.h"

void SetTapeMode(unsigned char mode)	//Handles button pressed from Dialog
{
  CassetteState* cassetteState = GetCassetteState();

  cassetteState->TapeMode = mode;

  switch (cassetteState->TapeMode)
  {
  case STOP:
    break;

  case PLAY:
    if (cassetteState->TapeHandle == NULL) {
      if (!LoadTape()) {
        cassetteState->TapeMode = STOP;
      }
      else {
        cassetteState->TapeMode = mode;
      }
    }

    if (cassetteState->MotorState) {
      Motor(1);
    }

    break;

  case REC:
    if (cassetteState->TapeHandle == NULL) {
      if (!LoadTape()) {
        cassetteState->TapeMode = STOP;
      }
      else {
        cassetteState->TapeMode = mode;
      }
    }
    break;

  case EJECT:
    CloseTapeFile();
    strcpy(cassetteState->TapeFileName, "EMPTY");

    break;
  }

  UpdateTapeCounter(cassetteState->TapeOffset, cassetteState->TapeMode);
}
