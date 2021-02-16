#include "library/graphicsstate.h"

#include "SetGimeBorderColor.h"
#include "ConfigAccessors.h"

void SetPaletteType() {
  int borderColor = GetGraphicsState()->CC3BorderColor;
  SetGimeBorderColor(0);
  MakeCMPpalette(GetPaletteType());
  SetGimeBorderColor(borderColor);
}
