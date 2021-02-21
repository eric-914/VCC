#include "library/defines.h"

#include "library/Cassette.h"
#include "library/CoCo.h"

#include "CassIn.h"
#include "CassOut.h"

unsigned char SetSndOutMode(unsigned char mode)  //0 = Speaker 1= Cassette Out 2=Cassette In
{
  CoCoState* coco = GetCoCoState();

  static unsigned char lastMode = 0;
  static unsigned short primarySoundRate = coco->SoundRate;

  switch (mode)
  {
  case 0:
    if (lastMode == 1) {	//Send the last bits to be encoded
      FlushCassetteBuffer(coco->CassBuffer, coco->AudioIndex); /* Cassette.cpp */
    }

    coco->AudioEvent = AudioOut;

    SetAudioRate(primarySoundRate);

    break;

  case 1:
    coco->AudioEvent = CassOut;

    primarySoundRate = coco->SoundRate;

    SetAudioRate(TAPEAUDIORATE);

    break;

  case 2:
    coco->AudioEvent = CassIn;

    primarySoundRate = coco->SoundRate;

    SetAudioRate(TAPEAUDIORATE);

    break;

  default:	//QUERY
    return(coco->SoundOutputMode);
    break;
  }

  if (mode != lastMode)
  {
    coco->AudioIndex = 0;	//Reset Buffer on true mode switch
    lastMode = mode;
  }

  coco->SoundOutputMode = mode;

  return(coco->SoundOutputMode);
}
