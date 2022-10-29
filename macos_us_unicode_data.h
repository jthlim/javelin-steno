//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

// Unicode mapping when keyboard is set to "US".
struct MacOsUsUnicodeData {
  static const uint16_t *GetSequenceForUnicode(uint32_t unicode);
};

//---------------------------------------------------------------------------
