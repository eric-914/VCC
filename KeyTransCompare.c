#include "library/di.version.h"

#include <dinput.h>

#include "keyboardstate.h"

#include "library/keyboarddef.h"

/**
  Key translation table compare function for sorting (with qsort)
*/
int KeyTransCompare(const void* e1, const void* e2)
{
  keytranslationentry_t* entry1 = (keytranslationentry_t*)e1;
  keytranslationentry_t* entry2 = (keytranslationentry_t*)e2;
  int result = 0;

  KeyboardState* keyboardState = GetKeyBoardState();

  // empty listing push to end
  if (entry1->ScanCode1 == 0 && entry1->ScanCode2 == 0 && entry2->ScanCode1 != 0)
  {
    return 1;
  }
  else {
    if (entry2->ScanCode1 == 0 && entry2->ScanCode2 == 0 && entry1->ScanCode1 != 0)
    {
      return -1;
    }
    else {
      // push shift/alt/control by themselves to the end
      if (entry1->ScanCode2 == 0 && (entry1->ScanCode1 == DIK_LSHIFT || entry1->ScanCode1 == DIK_LMENU || entry1->ScanCode1 == DIK_LCONTROL))
      {
        result = 1;
      }
      else {
        // push shift/alt/control by themselves to the end
        if (entry2->ScanCode2 == 0 && (entry2->ScanCode1 == DIK_LSHIFT || entry2->ScanCode1 == DIK_LMENU || entry2->ScanCode1 == DIK_LCONTROL))
        {
          result = -1;
        }
        else {
          // move double key combos in front of single ones
          if (entry1->ScanCode2 == 0 && entry2->ScanCode2 != 0)
          {
            result = 1;
          }
          else {
            // move double key combos in front of single ones
            if (entry2->ScanCode2 == 0 && entry1->ScanCode2 != 0)
            {
              result = -1;
            }
            else
            {
              result = entry1->ScanCode1 - entry2->ScanCode1;

              if (result == 0)
              {
                result = entry1->Row1 - entry2->Row1;
              }

              if (result == 0)
              {
                result = entry1->Col1 - entry2->Col1;
              }

              if (result == 0)
              {
                result = entry1->Row2 - entry2->Row2;
              }

              if (result == 0)
              {
                result = entry1->Col2 - entry2->Col2;
              }
            }
          }
        }
      }
    }
  }

  return result;
}
