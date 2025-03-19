//---------------------------------------------------------------------------

#pragma once
#include "key.h"

//---------------------------------------------------------------------------

struct StenoConfigBlock {
  uint8_t hidCompatibilityMode;
  bool useSpaceAfter;
  bool useFirstUp;
  bool useRepeat;
  bool useJeffModifiers;

  bool _reserved[27];

  int8_t keyMap[(int)StenoKey::COUNT];
};

static_assert(sizeof(StenoConfigBlock) == 96,
              "Config block is of unexpected size");

//---------------------------------------------------------------------------
