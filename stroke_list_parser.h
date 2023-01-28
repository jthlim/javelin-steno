//---------------------------------------------------------------------------

#pragma once
#include "dictionary/user_dictionary.h"
#include "stroke.h"
#include <assert.h>

//---------------------------------------------------------------------------

struct StrokeListParser {
  StenoStroke strokes[StenoUserDictionary::MAX_STROKE_COUNT];
  size_t length;
  const char *failureOrEnd;

  bool Set(const char *p);
};

//---------------------------------------------------------------------------
