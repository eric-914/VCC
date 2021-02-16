#include "MemWrite8.h"

void MemWrite16(unsigned short data, unsigned short addr)
{
  MemWrite8(data >> 8, addr);
  MemWrite8(data & 0xFF, addr + 1);
}
