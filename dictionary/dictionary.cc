//---------------------------------------------------------------------------

#include "dictionary.h"

#include "../console.h"
#include <stdlib.h>
#include <string.h>

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

void StenoReverseDictionaryLookup::AddResult(const StenoChord *c,
                                             size_t length) {
  // Ignore if above or equal to the threshold
  if (length >= strokeThreshold) {
    return;
  }

  // Ignore if it'll overflow.
  if (resultCount + 1 >= sizeof(resultLengths) ||
      chordsCount + length > CHORD_COUNT) {
    return;
  }

  resultLengths[resultCount++] = length;
  memcpy(&chords[chordsCount], c, sizeof(StenoChord) * length);
  chordsCount += length;
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

void StenoDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {}

bool StenoDictionary::ReverseMapDictionaryLookup(
    StenoReverseDictionaryLookup &result, const void *data) const {
  return false;
}

void StenoDictionary::PrintInfo(int depth) const {
  Console::Printf("%s%s\n", Spaces(depth), GetName());
}

//---------------------------------------------------------------------------
