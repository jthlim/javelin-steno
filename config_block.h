//---------------------------------------------------------------------------

#pragma once
#include "key.h"
#include "steno_key_code_emitter.h"
#include "steno_key_state.h"

//---------------------------------------------------------------------------

struct StenoConfigBlock {
  bool useFirstUp;
  bool useRepeat;
  bool useJeffModifiers;
  bool _deprecatedUseJeffShowStroke;
  bool _deprecatedUseJeffNumbers;
  bool _deprecatedUseEmilySymbols;
  bool _deprecatedUseJeffPhrasing;
  UnicodeMode unicodeMode;
  KeyboardLayout keyboardLayout;
  bool hidCompatibilityMode;
  bool useSpaceAfter;
  bool allowButtonStateUpdates;

  bool _reserved[20];

  int8_t keyMap[(int)StenoKey::COUNT];
};

static_assert(sizeof(StenoConfigBlock) == 74,
              "Config block is of unexpected size");

//---------------------------------------------------------------------------
