#include "library/cassettedef.h"
#include "library/cassettestate.h"

//--CASSETTE--//

void CastoWav(unsigned char* buffer, unsigned int bytesToConvert, unsigned long* bytesConverted)
{
  unsigned char byte = 0;
  char mask = 0;

  CassetteState* cassetteState = GetCassetteState();

  if (cassetteState->Quiet > 0)
  {
    cassetteState->Quiet--;

    memset(buffer, 0, bytesToConvert);

    return;
  }

  if ((cassetteState->TapeOffset > cassetteState->TotalSize) || (cassetteState->TotalSize == 0))	//End of tape return nothing
  {
    memset(buffer, 0, bytesToConvert);

    cassetteState->TapeMode = STOP;	//Stop at end of tape

    return;
  }

  while ((cassetteState->TempIndex < bytesToConvert) && (cassetteState->TapeOffset <= cassetteState->TotalSize))
  {
    byte = cassetteState->CasBuffer[(cassetteState->TapeOffset++) % cassetteState->TotalSize];

    for (mask = 0; mask <= 7; mask++)
    {
      if ((byte & (1 << mask)) == 0)
      {
        memcpy(&(cassetteState->TempBuffer[cassetteState->TempIndex]), cassetteState->Zero, 40);

        cassetteState->TempIndex += 40;
      }
      else
      {
        memcpy(&(cassetteState->TempBuffer[cassetteState->TempIndex]), cassetteState->One, 21);

        cassetteState->TempIndex += 21;
      }
    }
  }

  if (cassetteState->TempIndex >= bytesToConvert)
  {
    memcpy(buffer, cassetteState->TempBuffer, bytesToConvert); //Fill the return Buffer
    memcpy(cassetteState->TempBuffer, &(cassetteState->TempBuffer[bytesToConvert]), cassetteState->TempIndex - bytesToConvert);	//Slide the overage to the front

    cassetteState->TempIndex -= bytesToConvert; //Point to the Next free byte in the tempbuffer
  }
  else	//We ran out of source bytes
  {
    memcpy(buffer, cassetteState->TempBuffer, cassetteState->TempIndex);						//Partial Fill of return buffer;
    memset(&buffer[cassetteState->TempIndex], 0, bytesToConvert - cassetteState->TempIndex);		//and silence for the rest

    cassetteState->TempIndex = 0;
  }
}
