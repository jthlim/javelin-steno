//---------------------------------------------------------------------------

#pragma once
#include "key.h"

//---------------------------------------------------------------------------

struct StenoConfigBlock {
  bool useFirstUp;
  bool useRepeat;
  bool useJeffModifiers;
  bool _deprecatedUseJeffShowStroke;
  bool _deprecatedUseJeffNumbers;
  bool _deprecatedUseEmilySymbols;
  bool _deprecatedUseJeffPhrasing;
  uint8_t _deprecatedUnicodeMode;
  uint8_t _deprecatedKeyboardLayout;
  bool hidCompatibilityMode;
  bool useSpaceAfter;
  bool allowButtonStateUpdates;

  bool _reserved[20];

  int8_t keyMap[(int)StenoKey::COUNT];
};

static_assert(sizeof(StenoConfigBlock) == 74,
              "Config block is of unexpected size");

//---------------------------------------------------------------------------
