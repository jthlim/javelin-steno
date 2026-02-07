//---------------------------------------------------------------------------

#pragma once
#include "container/static_list.h"
#include "dictionary/user_dictionary.h"
#include "stroke.h"

//---------------------------------------------------------------------------

// Parses a stroke list, e.g. "TEFT/-G" into StenoStroke array.
struct StrokeListParser
    : public StaticList<StenoStroke, StenoUserDictionary::MAX_STROKE_COUNT> {
  const char *failureOrEnd;

  bool Parse(const char *p);
};

//---------------------------------------------------------------------------
