//---------------------------------------------------------------------------

#include "dictionary.h"

#include "../console.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

const char StenoDictionary::SPACES[SPACES_COUNT + 1] = "                ";

static const char *ReturnContextAsString(const StenoDictionaryLookupResult *p) {
  return (const char *)p->context;
}

static void NoOp(StenoDictionaryLookupResult *) {}

static void FreeString(StenoDictionaryLookupResult *p) {
  free((void *)p->context);
}

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoDictionaryLookupResult::CreateStaticString(const char *p) {
  StenoDictionaryLookupResult result = {
      .GetTextMethod = ReturnContextAsString,
      .DestroyMethod = NoOp,
      .context = p,
  };
  return result;
}

// string will be free() when the Lookup is destroyed.
StenoDictionaryLookupResult
StenoDictionaryLookupResult::CreateDynamicString(const char *p) {
  StenoDictionaryLookupResult result = {
      .GetTextMethod = ReturnContextAsString,
      .DestroyMethod = FreeString,
      .context = p,
  };
  return result;
}

//---------------------------------------------------------------------------

void StenoDictionary::PrintInfo(int depth) const {
  Console::Printf("%s%s\n", Spaces(depth), GetName());
}

//---------------------------------------------------------------------------
