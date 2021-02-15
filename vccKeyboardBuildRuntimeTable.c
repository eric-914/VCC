#include "library/di.version.h"

#include <windows.h>
#include <assert.h>
#include <dinput.h>

#include "keyboardstate.h"

#include "KeyTransCompare.h"

#include "library/keyboarddef.h"
#include "library/keyboardlayout.h"

/*
  Rebuilds the run-time keyboard translation lookup table based on the
  current keyboard layout.

  The entries are sorted.  Any SHIFT + [char] entries need to be placed first
*/
extern "C" {
  void vccKeyboardBuildRuntimeTable(keyboardlayout_e keyBoardLayout)
  {
    int index1 = 0;
    int index2 = 0;
    keytranslationentry_t* keyLayoutTable = NULL;
    keytranslationentry_t	keyTransEntry;

    KeyboardState* keyboardState = GetKeyBoardState();

    assert(keyBoardLayout >= 0 && keyBoardLayout < kKBLayoutCount);

    switch (keyBoardLayout)
    {
    case kKBLayoutCoCo:
      keyLayoutTable = GetKeyTranslationsCoCo();
      break;

    case kKBLayoutNatural:
      keyLayoutTable = GetKeyTranslationsNatural();
      break;

    case kKBLayoutCompact:
      keyLayoutTable = GetKeyTranslationsCompact();
      break;

    case kKBLayoutCustom:
      keyLayoutTable = GetKeyTranslationsCustom();
      break;

    default:
      assert(0 && "unknown keyboard layout");
      break;
    }

    //XTRACE("Building run-time key table for layout # : %d - %s\n", keyBoardLayout, k_keyboardLayoutNames[keyBoardLayout]);

    // copy the selected keyboard layout to the run-time table
    memset(keyboardState->KeyTransTable, 0, sizeof(keyboardState->KeyTransTable));
    index2 = 0;

    for (index1 = 0; ; index1++)
    {
      memcpy(&keyTransEntry, &keyLayoutTable[index1], sizeof(keytranslationentry_t));

      //
      // Change entries to what the code expects
      //
      // Make sure ScanCode1 is never 0
      // If the key combo uses SHIFT, put it in ScanCode1
      // Completely clear unused entries (ScanCode1+2 are 0)
      //

      //
      // swaps ScanCode1 with ScanCode2 if ScanCode2 == DIK_LSHIFT
      //
      if (keyTransEntry.ScanCode2 == DIK_LSHIFT)
      {
        keyTransEntry.ScanCode2 = keyTransEntry.ScanCode1;
        keyTransEntry.ScanCode1 = DIK_LSHIFT;
      }

      //
      // swaps ScanCode1 with ScanCode2 if ScanCode1 is zero
      //
      if ((keyTransEntry.ScanCode1 == 0) && (keyTransEntry.ScanCode2 != 0))
      {
        keyTransEntry.ScanCode1 = keyTransEntry.ScanCode2;
        keyTransEntry.ScanCode2 = 0;
      }

      // check for terminating entry
      if (keyTransEntry.ScanCode1 == 0 && keyTransEntry.ScanCode2 == 0)
      {
        break;
      }

      memcpy(&(keyboardState->KeyTransTable[index2++]), &keyTransEntry, sizeof(keytranslationentry_t));

      assert(index2 <= KBTABLE_ENTRY_COUNT && "keyboard layout table is longer than we can handle");
    }

    //
    // Sort the key translation table
    //
    // Since the table is searched from beginning to end each
    // time a key is pressed, we want them to be in the correct 
    // order.
    //
    qsort(keyboardState->KeyTransTable, KBTABLE_ENTRY_COUNT, sizeof(keytranslationentry_t), KeyTransCompare);

#ifdef _DEBUG
    //
    // Debug dump the table
    //
    for (index1 = 0; index1 < KBTABLE_ENTRY_COUNT; index1++)
    {
      // check for null entry
      if (keyboardState->KeyTransTable[index1].ScanCode1 == 0 && keyboardState->KeyTransTable[index1].ScanCode2 == 0)
      {
        // done
        break;
      }

      //XTRACE("Key: %3d - 0x%02X (%3d) 0x%02X (%3d) - %2d %2d  %2d %2d\n",
      //  Index1,
      //  KeyTransTable[Index1].ScanCode1,
      //  KeyTransTable[Index1].ScanCode1,
      //  KeyTransTable[Index1].ScanCode2,
      //  KeyTransTable[Index1].ScanCode2,
      //  KeyTransTable[Index1].Row1,
      //  KeyTransTable[Index1].Col1,
      //  KeyTransTable[Index1].Row2,
      //  KeyTransTable[Index1].Col2
      //);
    }
#endif
  }
}
