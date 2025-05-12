//---------------------------------------------------------------------------

#pragma once
#include "dictionary/user_dictionary.h"
#include "stroke.h"

//---------------------------------------------------------------------------

// Parses a stroke list, e.g. "TEFT/-G" into StenoStroke array.
struct StrokeListParser {
  StenoStroke strokes[StenoUserDictionary::MAX_STROKE_COUNT];
  size_t length;
  const char *failureOrEnd;

  bool Parse(const char *p);
};

//---------------------------------------------------------------------------
