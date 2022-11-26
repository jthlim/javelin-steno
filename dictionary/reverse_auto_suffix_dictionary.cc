//---------------------------------------------------------------------------

#include "reverse_auto_suffix_dictionary.h"
#include "../orthography.h"
#include "../pattern.h"
#include "../str.h"

//---------------------------------------------------------------------------

const Pattern *StenoReverseAutoSuffixDictionary::CreateReversePatterns(
    const StenoOrthography &orthography) {
  Pattern *patterns =
      (Pattern *)malloc(sizeof(Pattern) * orthography.reverseAutoSuffixCount);
  for (size_t i = 0; i < orthography.reverseAutoSuffixCount; ++i) {
    patterns[i] =
        Pattern::Compile(orthography.reverseAutoSuffixes[i].testPattern);
  }
  return patterns;
}

//---------------------------------------------------------------------------

StenoReverseAutoSuffixDictionary::StenoReverseAutoSuffixDictionary(
    StenoDictionary *dictionary, const StenoCompiledOrthography &orthography)
    : dictionary(dictionary), orthography(orthography),
      reversePatterns(CreateReversePatterns(orthography.data)) {}

StenoDictionaryLookupResult StenoReverseAutoSuffixDictionary::Lookup(
    const StenoDictionaryLookup &lookup) const {
  return dictionary->Lookup(lookup);
}

const StenoDictionary *StenoReverseAutoSuffixDictionary::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {
  return dictionary->GetLookupProvider(lookup);
}

void StenoReverseAutoSuffixDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {

  dictionary->ReverseLookup(result);

  for (size_t i = 0; i < orthography.data.reverseAutoSuffixCount; ++i) {
    ProcessReverseAutoSuffix(result, orthography.data.reverseAutoSuffixes[i],
                             reversePatterns[i]);
  }
}

bool StenoReverseAutoSuffixDictionary::ReverseMapDictionaryLookup(
    StenoReverseDictionaryLookup &result, const void *data) const {
  return dictionary->ReverseMapDictionaryLookup(result, data);
}

unsigned int StenoReverseAutoSuffixDictionary::GetMaximumMatchLength() const {
  return dictionary->GetMaximumMatchLength();
}

const char *StenoReverseAutoSuffixDictionary::GetName() const {
  return "#internal#reverse_auto_suffix_dictionary";
}

void StenoReverseAutoSuffixDictionary::PrintInfo(int depth) const {
  dictionary->PrintInfo(depth);
}

bool StenoReverseAutoSuffixDictionary::PrintDictionary(bool hasData) const {
  return dictionary->PrintDictionary(hasData);
}

void StenoReverseAutoSuffixDictionary::ListDictionaries() const {
  dictionary->ListDictionaries();
}

bool StenoReverseAutoSuffixDictionary::EnableDictionary(const char *name) {
  return dictionary->EnableDictionary(name);
}

bool StenoReverseAutoSuffixDictionary::DisableDictionary(const char *name) {
  return dictionary->DisableDictionary(name);
}

bool StenoReverseAutoSuffixDictionary::ToggleDictionary(const char *name) {
  return dictionary->ToggleDictionary(name);
}

//---------------------------------------------------------------------------

void StenoReverseAutoSuffixDictionary::ProcessReverseAutoSuffix(
    StenoReverseDictionaryLookup &result,
    const StenoOrthographyReverseAutoSuffix &reverseAutoSuffix,
    const Pattern &reversePattern) const {
  // 1. Generate word without suffix
  // 2. Test addition of the auto-suffix matches the lookup.
  // 3. Lookup the un-suffixed word,
  // 4. Add in the auto suffix, and verify that it produces invalid lookup.

  // 1. To generate the word without suffix, run the regex on up to the last
  // 8 letters of the lookup.
  size_t lookupOffset = result.lookupLength > 8 ? result.lookupLength - 8 : 0;
  PatternMatch match = reversePattern.Match(result.lookup + lookupOffset);
  if (!match.match) {
    return;
  }

  // 2. Create the without suffix version, add the suffix, and verify it
  // matches.
  char *replacedSuffix = match.Replace(reverseAutoSuffix.replacement);

  char *withoutSuffix = (char *)malloc(result.lookupLength + 8);
  memcpy(withoutSuffix, result.lookup, lookupOffset);
  strcpy(withoutSuffix + lookupOffset, replacedSuffix);
  free(replacedSuffix);

  char *suffix = Str::DupN(reverseAutoSuffix.autoSuffix->text + 3,
                           strlen(reverseAutoSuffix.autoSuffix->text) - 4);
  char *withSuffix = orthography.AddSuffix(withoutSuffix, suffix);
  free(suffix);

  bool isSuffixEqual = Str::Eq(withSuffix, result.lookup);
  free(withSuffix);
  if (!isSuffixEqual) {
    free(withoutSuffix);
    return;
  }

  // 3. Lookup the un-suffixed word
  StenoReverseDictionaryLookup resultWithoutSuffix(result.strokeThreshold,
                                                   withoutSuffix);
  dictionary->ReverseLookup(resultWithoutSuffix);
  free(withoutSuffix);

  // 4. Verify that lookup up with suffix produces an invalid lookup.
  StenoChord *chords = resultWithoutSuffix.chords;
  for (size_t i = 0; i < resultWithoutSuffix.resultCount; ++i) {
    size_t length = resultWithoutSuffix.resultLengths[i];

    bool hasAdded = false;

    if ((chords[length - 1] & reverseAutoSuffix.suppressMask).IsEmpty()) {
      chords[length - 1] |= reverseAutoSuffix.autoSuffix->chord;

      if (!HasValidLookup(chords, length)) {
        // Even if it produced an invalid lookup, there's a chance that
        // another auto-suffix might take precedence. Check for that.
        for (size_t i = 0; i < orthography.data.autoSuffixCount; ++i) {
          const StenoOrthographyAutoSuffix *autoSuffix =
              &orthography.data.autoSuffixes[i];
          if (reverseAutoSuffix.autoSuffix == autoSuffix) {
            result.AddResult(chords, length, this);
            hasAdded = true;
          }

          if ((chords[length - 1] & autoSuffix->chord).IsNotEmpty()) {
            chords[length - 1] &= ~autoSuffix->chord;
            const StenoDictionary *lookupProvider =
                dictionary->GetLookupProvider(chords, length);
            if (lookupProvider) {
              break;
            }
            chords[length - 1] |= autoSuffix->chord;
          }
        }
      }
      chords[length - 1] &= ~reverseAutoSuffix.autoSuffix->chord;
    }

    if (!hasAdded) {
      StenoChord chordsWithSuffixStroke[length + 1];
      memcpy(chordsWithSuffixStroke, chords, length * sizeof(StenoChord));
      chordsWithSuffixStroke[length] = reverseAutoSuffix.autoSuffix->chord;

      if (dictionary->GetLookupProvider(chordsWithSuffixStroke, length + 1) ==
          nullptr) {
        result.AddResult(chordsWithSuffixStroke, length + 1, this);
      }
    }

    chords += length;
  }
}

bool StenoReverseAutoSuffixDictionary::HasValidLookup(const StenoChord *chords,
                                                      size_t length) const {
  for (size_t chordLength = 1; chordLength <= length; ++chordLength) {
    const StenoDictionary *lookupProvider = dictionary->GetLookupProvider(
        chords + length - chordLength, chordLength);
    if (lookupProvider) {
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------
