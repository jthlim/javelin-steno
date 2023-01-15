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

static void FreeString(StenoDictionaryLookupResult *p) {
  free((void *)p->context);
}

static void Nop(StenoDictionaryLookupResult *) {}

const StenoDictionaryLookupResult::StenoDictionaryLookupResultVtbl
    StenoDictionaryLookupResult::invalidVtbl = {
        .getTextMethod = nullptr,
        .destroyMethod = Nop,
};

const StenoDictionaryLookupResult::StenoDictionaryLookupResultVtbl
    StenoDictionaryLookupResult::staticVtbl = {
        .getTextMethod = ReturnContextAsString,
        .destroyMethod = Nop,
};

const StenoDictionaryLookupResult::StenoDictionaryLookupResultVtbl
    StenoDictionaryLookupResult::dynamicVtbl = {
        .getTextMethod = ReturnContextAsString,
        .destroyMethod = FreeString,
};

//---------------------------------------------------------------------------

void StenoReverseDictionaryLookup::AddResult(
    const StenoStroke *c, size_t length,
    const StenoDictionary *lookupProvider) {
  // Ignore if above or equal to the threshold
  if (length >= strokeThreshold) {
    return;
  }

  if (HasResult(c, length)) {
    return;
  }

  // Ignore if it'll overflow.
  if (resultCount + 1 >= sizeof(results) / sizeof(*results) ||
      strokesCount + length > STROKE_COUNT) {
    return;
  }

  StenoReverseDictionaryResult &result = results[resultCount++];
  result.length = length;
  result.strokes = strokes + strokesCount;
  result.lookupProvider = lookupProvider;

  memcpy(&strokes[strokesCount], c, sizeof(StenoStroke) * length);
  strokesCount += length;
}

bool StenoReverseDictionaryLookup::HasResult(const StenoStroke *c,
                                             size_t length) const {

  for (size_t i = 0; i < resultCount; ++i) {
    const StenoReverseDictionaryResult &result = results[i];
    if (result.length == length &&
        memcmp(result.strokes, c, length * sizeof(StenoStroke)) == 0) {
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------

const StenoDictionary *
StenoDictionary::GetLookupProvider(const StenoDictionaryLookup &lookup) const {
  StenoDictionaryLookupResult lookupResult = Lookup(lookup);
  bool result = lookupResult.IsValid();
  lookupResult.Destroy();
  return result ? this : nullptr;
}

void StenoDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {}

bool StenoDictionary::ReverseMapDictionaryLookup(
    StenoReverseMapDictionaryLookup &lookup) const {
  return false;
}

void StenoDictionary::PrintInfo(int depth) const {
  Console::Printf("%s%s\n", Spaces(depth), GetName());
}

//---------------------------------------------------------------------------
