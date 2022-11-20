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

void StenoReverseDictionaryLookup::AddResult(
    const StenoChord *c, size_t length, const StenoDictionary *lookupProvider) {
  // Ignore if above or equal to the threshold
  if (length >= strokeThreshold) {
    return;
  }

  if (HasResult(c, length, lookupProvider)) {
    return;
  }

  // Ignore if it'll overflow.
  if (resultCount + 1 >= sizeof(resultLengths) ||
      chordsCount + length > CHORD_COUNT) {
    return;
  }

  resultLengths[resultCount] = length;
  lookupProviders[resultCount] = lookupProvider;
  ++resultCount;
  memcpy(&chords[chordsCount], c, sizeof(StenoChord) * length);
  chordsCount += length;
}

bool StenoReverseDictionaryLookup::HasResult(
    const StenoChord *c, size_t length,
    const StenoDictionary *lookupProvider) const {

  const StenoChord *currentChord = chords;
  for (size_t i = 0; i < resultCount; ++i) {
    size_t currentChordLength = resultLengths[i];
    if (currentChordLength == length && lookupProvider == lookupProviders[i] &&
        memcmp(currentChord, c, length * sizeof(StenoChord)) == 0) {
      return true;
    }
    currentChord += currentChordLength;
  }

  return false;
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
    StenoReverseDictionaryLookup &result, const void *data) const {
  return false;
}

void StenoDictionary::PrintInfo(int depth) const {
  Console::Printf("%s%s\n", Spaces(depth), GetName());
}

//---------------------------------------------------------------------------
