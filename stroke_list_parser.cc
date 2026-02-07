//---------------------------------------------------------------------------

#include "stroke_list_parser.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

// Returns true if successfully parsed, false otherwise.
bool StrokeListParser::Parse(const char *p) {
  for (;;) {
    if (IsFull()) {
      failureOrEnd = p;
      return false;
    }
    const char *start = p;
    while (*p != ' ' && *p != '\0' && *p != '/') {
      ++p;
    }

    char *text = Str::DupN(start, p - start);
    StenoStroke &stroke = Add();
    stroke.Set(text);
    free(text);

    if (stroke.IsEmpty()) {
      failureOrEnd = start;
      return false;
    }
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
