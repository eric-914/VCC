#include "library/mc6821state.h"

void SetCart(unsigned char cart)
{
  MC6821State* mc6821State = GetMC6821State();

  mc6821State->CartInserted = cart;
}
