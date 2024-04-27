//---------------------------------------------------------------------------

#include "stroke_list_parser.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

// Returns true if successfully parsed, false otherwise.
bool StrokeListParser::Parse(const char *p) {
  length = 0;

  for (;;) {
    if (length >= StenoUserDictionary::MAX_STROKE_COUNT) {
      failureOrEnd = p;
      return false;
    }
    const char *start = p;
    while (*p != ' ' && *p != '\0' && *p != '/') {
      ++p;
    }

    char *text = Str::DupN(start, p - start);
    strokes[length].Set(text);
    free(text);

    if (strokes[length].IsEmpty()) {
      failureOrEnd = start;
      return false;
    }
    ++length;
    const int last = *p;
    if (last != '\0') {
      ++p;
    }
    if (last != '/') {
      failureOrEnd = p;
      return true;
    }
  }
}
//---------------------------------------------------------------------------
