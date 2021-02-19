#include "mc6821state.h"
#include "PakInterfaceAccessors.h"

extern "C" {
  __declspec(dllexport) unsigned int __cdecl GetDACSample(void)
  {
    static unsigned int retVal = 0;
    static unsigned short sampleLeft = 0, sampleRight = 0, pakSample = 0;
    static unsigned short outLeft = 0, outRight = 0;
    static unsigned short lastLeft = 0, lastRight = 0;

    MC6821State* mc6821State = GetMC6821State();

    pakSample = PakAudioSample();
    sampleLeft = (pakSample >> 8) + mc6821State->Asample + mc6821State->Ssample;
    sampleRight = (pakSample & 0xFF) + mc6821State->Asample + mc6821State->Ssample; //9 Bits each
    sampleLeft = sampleLeft << 6;	//Conver to 16 bit values
    sampleRight = sampleRight << 6;	//For Max volume

    if (sampleLeft == lastLeft)	//Simulate a slow high pass filter
    {
      if (outLeft) {
        outLeft--;
      }
    }
    else
    {
      outLeft = sampleLeft;
      lastLeft = sampleLeft;
    }

    if (sampleRight == lastRight)
    {
      if (outRight) {
        outRight--;
      }
    }
    else
    {
      outRight = sampleRight;
      lastRight = sampleRight;
    }

    retVal = (outLeft << 16) + (outRight);

    return(retVal);
  }
}