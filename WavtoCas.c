#include "library/cassettedef.h"
#include "library/cassettestate.h"

//--CASSETTE--//

void WavtoCas(unsigned char* waveBuffer, unsigned int length)
{
  unsigned char bit = 0, sample = 0;
  unsigned int index = 0, width = 0;

  CassetteState* cassetteState = GetCassetteState();

  for (index = 0; index < length; index++) {
    sample = waveBuffer[index];

    if ((cassetteState->LastSample <= 0x80) && (sample > 0x80))	//Low to High transition
    {
      width = index - cassetteState->LastTrans;

      if ((width < 10) || (width > 50))	//Invalid Sample Skip it
      {
        cassetteState->LastSample = 0;
        cassetteState->LastTrans = index;
        cassetteState->Mask = 0;
        cassetteState->Byte = 0;
      }
      else
      {
        bit = 1;

        if (width > 30) {
          bit = 0;
        }

        cassetteState->Byte = cassetteState->Byte | (bit << cassetteState->Mask);
        cassetteState->Mask++;
        cassetteState->Mask &= 7;

        if (cassetteState->Mask == 0)
        {
          cassetteState->CasBuffer[cassetteState->TapeOffset++] = cassetteState->Byte;
          cassetteState->Byte = 0;

          if (cassetteState->TapeOffset >= WRITEBUFFERSIZE) {	//Don't blow past the end of the buffer
            cassetteState->TapeMode = STOP;
          }
        }
      }

      cassetteState->LastTrans = index;
    }

    cassetteState->LastSample = sample;
  }

  cassetteState->LastTrans -= length;

  if (cassetteState->TapeOffset > cassetteState->TotalSize) {
    cassetteState->TotalSize = cassetteState->TapeOffset;
  }
}
