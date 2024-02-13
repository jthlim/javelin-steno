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
    : StenoWrappedDictionary(dictionary), orthography(orthography),
      reversePatterns(CreateReversePatterns(orthography.data)) {}

void StenoReverseAutoSuffixDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  dictionary->ReverseLookup(result);

  for (size_t i = 0; i < orthography.data.reverseAutoSuffixCount; ++i) {
    ProcessReverseAutoSuffix(result, orthography.data.reverseAutoSuffixes[i],
                             reversePatterns[i]);
  }
}

const char *StenoReverseAutoSuffixDictionary::GetName() const {
  return "#internal#reverse_auto_suffix_dictionary";
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
  const PatternMatch match = reversePattern.Match(result.lookup + lookupOffset);
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
  for (size_t i = 0; i < resultWithoutSuffix.resultCount; ++i) {
    const StenoReverseDictionaryResult &lookup = resultWithoutSuffix.results[i];

    size_t length = lookup.length;
    StenoStroke *strokes = lookup.strokes;

    bool hasAdded = false;

    if ((strokes[length - 1] & reverseAutoSuffix.suppressMask).IsEmpty()) {
      strokes[length - 1] |= reverseAutoSuffix.autoSuffix->stroke;

      if (result.HasResult(strokes, length)) {
        hasAdded = true;
      } else if (CanAutoSuffixLookup(strokes, length)) {
        // Even if it produced an invalid lookup, there's a chance that
        // another auto-suffix might take precedence. Check for that.
        for (size_t i = 0; i < orthography.data.autoSuffixCount; ++i) {
          const StenoOrthographyAutoSuffix *autoSuffix =
              &orthography.data.autoSuffixes[i];
          if (reverseAutoSuffix.autoSuffix == autoSuffix) {
            result.AddResult(strokes, length, this);
            hasAdded = true;
          }

          if ((strokes[length - 1] & autoSuffix->stroke).IsNotEmpty()) {
            strokes[length - 1] &= ~autoSuffix->stroke;
            if (dictionary->HasOutline(strokes, length)) {
              break;
            }
            strokes[length - 1] |= autoSuffix->stroke;
          }
        }
      }
      strokes[length - 1] &= ~reverseAutoSuffix.autoSuffix->stroke;
    }

    if (!hasAdded) {
      StenoStroke strokesWithSuffixStroke[length + 1];
      memcpy(strokesWithSuffixStroke, strokes, length * sizeof(StenoStroke));
      strokesWithSuffixStroke[length] = reverseAutoSuffix.autoSuffix->stroke;

      if (!dictionary->HasOutline(strokesWithSuffixStroke, length + 1)) {
        result.AddResult(strokesWithSuffixStroke, length + 1, this);
      }
    }
  }
}

bool StenoReverseAutoSuffixDictionary::CanAutoSuffixLookup(
    const StenoStroke *strokes, size_t length) const {
  // For autosuffix to be used, there must be no valid shorter lookup,
  // or there must be no valid suffix lookup
  if (dictionary->HasOutline(strokes, length)) {
    return false;
  }
  return !HasPrefixLookup(strokes, length) ||
         (length > 1 && !HasSuffixLookup(strokes, length));
}

// Returns true if there's any valid prefix lookup.
bool StenoReverseAutoSuffixDictionary::HasPrefixLookup(
    const StenoStroke *strokes, size_t length) const {
  for (size_t strokeLength = 1; strokeLength < length; ++strokeLength) {
    if (dictionary->HasOutline(strokes, strokeLength)) {
      return true;
    }
  }

  return false;
}

// Returns true if the last stroke lookup is valid.
bool StenoReverseAutoSuffixDictionary::HasSuffixLookup(
    const StenoStroke *strokes, size_t length) const {
  return dictionary->HasOutline(strokes + length - 1, 1);
}

//---------------------------------------------------------------------------
