#include "library/graphicsstate.h"

#include "library/ConfigAccessors.h"
#include "CocoReset.h"
#include "ResetAudio.h"

void GimeReset(void)
{
  ResetGraphicsState();

  MakeRGBPalette();
  MakeCMPpalette(GetPaletteType());

  CocoReset();
  ResetAudio();
}
