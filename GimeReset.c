#include "library/graphicsstate.h"

#include "library/Config.h"
#include "library/Coco.h"
#include "library/Audio.h"

void GimeReset(void)
{
  ResetGraphicsState();

  MakeRGBPalette();
  MakeCMPpalette(GetPaletteType());

  CocoReset();
  ResetAudio();
}
