#include "library/cassettedef.h"
#include "library/cassettestate.h"

//--CASSETTE--//

#include "SetSndOutMode.h"
#include "SyncFileBuffer.h"

void Motor(unsigned char state)
{
  CassetteState* cassetteState = GetCassetteState();

  cassetteState->MotorState = state;

  switch (cassetteState->MotorState)
  {
  case 0:
    SetSndOutMode(0);

    switch (cassetteState->TapeMode)
    {
    case STOP:
      break;

    case PLAY:
      cassetteState->Quiet = 30;
      cassetteState->TempIndex = 0;
      break;

    case REC:
      SyncFileBuffer();
      break;

    case EJECT:
      break;
    }
    break;	//MOTOROFF

  case 1:
    switch (cassetteState->TapeMode)
    {
    case STOP:
      SetSndOutMode(0);
      break;

    case PLAY:
      SetSndOutMode(2);
      break;

    case REC:
      SetSndOutMode(1);
      break;

    case EJECT:
      SetSndOutMode(0);
    }

    break;	//MOTORON	
  }
}
