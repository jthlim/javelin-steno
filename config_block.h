//---------------------------------------------------------------------------

#pragma once
#include "key_code.h"
#include "steno_key_code_emitter.h"
#include "steno_key_state.h"

//---------------------------------------------------------------------------

struct StenoConfigBlock {
  bool useFirstUp;
  bool useRepeat;
  bool useJeffModifiers;
  bool useJeffShowStroke;
  bool useJeffNumbers;
  bool useEmilySymbols;
  bool useJeffPhrasing;
  UnicodeMode unicodeMode;
  KeyboardLayout keyboardLayout;

  int8_t keyMap[(int)StenoKey::COUNT];
};

static_assert(sizeof(StenoConfigBlock) == 51,
              "Config block is of unexpected size");

//---------------------------------------------------------------------------
