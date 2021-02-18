#include "MemRead16.h"

unsigned int MemRead32(unsigned short addr)
{
  return MemRead16(addr) << 16 | MemRead16(addr + 2);
}
