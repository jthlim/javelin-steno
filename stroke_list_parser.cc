//---------------------------------------------------------------------------

#include "stroke_list_parser.h"

//---------------------------------------------------------------------------

// Returns true if successfully parsed, ERR otherwise.
bool StrokeListParser::Set(const char *p) {
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
    int last = *p;
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
