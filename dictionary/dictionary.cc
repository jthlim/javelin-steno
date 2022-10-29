//---------------------------------------------------------------------------

#include "dictionary.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

static const char *ReturnContextAsString(const StenoDictionaryLookup *p) {
  return (const char *)p->context;
}

static void NoOp(StenoDictionaryLookup *) {}

static void FreeString(StenoDictionaryLookup *p) { free((void *)p->context); }

//---------------------------------------------------------------------------

StenoDictionaryLookup StenoDictionaryLookup::CreateStaticString(const char *p) {
  StenoDictionaryLookup result = {
      .GetTextMethod = ReturnContextAsString,
      .DestroyMethod = NoOp,
      .context = p,
  };
  return result;
}

// string will be free() when the Lookup is destroyed.
StenoDictionaryLookup
StenoDictionaryLookup::CreateDynamicString(const char *p) {
  StenoDictionaryLookup result = {
      .GetTextMethod = ReturnContextAsString,
      .DestroyMethod = FreeString,
      .context = p,
  };
  return result;
}

//---------------------------------------------------------------------------
