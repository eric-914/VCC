#include <assert.h>

#include "keyboardstate.h"

void vccKeyboardUpdateRolloverTable()
{
  unsigned char	lockOut = 0;

  KeyboardState* keyboardState = GetKeyBoardState();

  // clear the rollover table
  for (int index = 0; index < 8; index++)
  {
    keyboardState->RolloverTable[index] = 0;
  }

  // set rollover table based on ScanTable key status
  for (int index = 0; index < KBTABLE_ENTRY_COUNT; index++)
  {
    // stop at last entry
    if ((keyboardState->KeyTransTable[index].ScanCode1 == 0) && (keyboardState->KeyTransTable[index].ScanCode2 == 0))
    {
      break;
    }

    if (lockOut != keyboardState->KeyTransTable[index].ScanCode1)
    {
      // Single input key 
      if ((keyboardState->KeyTransTable[index].ScanCode1 != 0) && (keyboardState->KeyTransTable[index].ScanCode2 == 0))
      {
        // check if key pressed
        if (keyboardState->ScanTable[keyboardState->KeyTransTable[index].ScanCode1] == KEY_DOWN)
        {
          int col = keyboardState->KeyTransTable[index].Col1;

          assert(col >= 0 && col < 8);

          keyboardState->RolloverTable[col] |= keyboardState->KeyTransTable[index].Row1;

          col = keyboardState->KeyTransTable[index].Col2;

          assert(col >= 0 && col < 8);

          keyboardState->RolloverTable[col] |= keyboardState->KeyTransTable[index].Row2;
        }
      }

      // Double Input Key
      if ((keyboardState->KeyTransTable[index].ScanCode1 != 0) && (keyboardState->KeyTransTable[index].ScanCode2 != 0))
      {
        // check if both keys pressed
        if ((keyboardState->ScanTable[keyboardState->KeyTransTable[index].ScanCode1] == KEY_DOWN) && (keyboardState->ScanTable[keyboardState->KeyTransTable[index].ScanCode2] == KEY_DOWN))
        {
          int col = keyboardState->KeyTransTable[index].Col1;

          assert(col >= 0 && col < 8);

          keyboardState->RolloverTable[col] |= keyboardState->KeyTransTable[index].Row1;

          col = keyboardState->KeyTransTable[index].Col2;

          assert(col >= 0 && col < 8);

          keyboardState->RolloverTable[col] |= keyboardState->KeyTransTable[index].Row2;

          // always SHIFT
          lockOut = keyboardState->KeyTransTable[index].ScanCode1;

          break;
        }
      }
    }
  }
}
