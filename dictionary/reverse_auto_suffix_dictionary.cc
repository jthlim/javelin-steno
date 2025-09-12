//---------------------------------------------------------------------------

#include "reverse_auto_suffix_dictionary.h"
#include "../orthography.h"
#include "../pattern.h"
#include "../str.h"

//---------------------------------------------------------------------------

StenoReverseAutoSuffixDictionary::StenoReverseAutoSuffixDictionary(
    StenoDictionary *dictionary, const StenoCompiledOrthography &orthography)
    : StenoWrappedDictionary(dictionary), orthography(orthography) {
  for (const StenoOrthographyReverseAutoSuffix &reverseAutoSuffix :
       orthography.data.reverseAutoSuffixes) {
    AddTest(reverseAutoSuffix);
  }

  for (const AutoSuffixTest &test : tests) {
    mergedQuickReject.Merge(test.reversePattern.GetQuickReject());
  }
}

void StenoReverseAutoSuffixDictionary::AddTest(
    const StenoOrthographyReverseAutoSuffix &reverseAutoSuffix) {
  for (AutoSuffixTest &test : tests) {
    if (test.testPattern == reverseAutoSuffix.testPattern &&
        test.replacement == reverseAutoSuffix.replacement &&
        test.text == reverseAutoSuffix.autoSuffix->text &&
        test.replacements.IsNotFull()) {
      test.replacements.Add(&reverseAutoSuffix);
      return;
    }
  }

  AutoSuffixTest test;
  test.testPattern = reverseAutoSuffix.testPattern;
  test.replacement = reverseAutoSuffix.replacement;
  test.text = reverseAutoSuffix.autoSuffix->text;
  test.textLength = Str::Length(test.text);
  test.reversePattern = Pattern::Compile(reverseAutoSuffix.testPattern);
  test.replacements.Add(&reverseAutoSuffix);
  tests.Add(test);
}

void StenoReverseAutoSuffixDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  super::ReverseLookup(lookup);

  if (lookup.definitionLength < 2) {
    return;
  }

  const char *quickRejectText =
      lookup.definitionLength > 8
          ? lookup.definition + lookup.definitionLength - 8
          : lookup.definition;
  const PatternQuickReject textReject(quickRejectText);

  if (!mergedQuickReject.IsPossibleMergeMatch(textReject)) {
    return;
  }

  for (const AutoSuffixTest &test : tests) {
    if (test.reversePattern.IsPossibleMatch(textReject)) {
      ProcessReverseAutoSuffix(lookup, test);
    }
  }
}

const char *StenoReverseAutoSuffixDictionary::GetName() const {
  return "#internal#reverse_auto_suffix_dictionary";
}

//---------------------------------------------------------------------------

void StenoReverseAutoSuffixDictionary::ProcessReverseAutoSuffix(
    StenoReverseDictionaryLookup &lookup, const AutoSuffixTest &test) const {
  // 1. Generate word without suffix
  // 2. Test addition of the auto-suffix matches the lookup.
  // 3. Lookup the un-suffixed word,
  // 4. Add in the auto suffix, and verify that it produces invalid lookup.

  // 1. To generate the word without suffix, run the regex on up to the last
  // 8 letters of the definition.
  const PatternMatch match = test.reversePattern.MatchBypassingQuickReject(
      lookup.definition, lookup.definitionLength);
  if (!match.match) {
    return;
  }

  // 2. Create the without suffix version, add the suffix, and verify it
  // matches.
  char *withoutSuffix = match.Replace(test.replacement);
  char *suffix = Str::DupN(test.text + 3, test.textLength - 4);
  char *withSuffix = orthography.AddSuffixToPhrase(withoutSuffix, suffix);
  free(suffix);

  const bool isSuffixEqual = Str::Eq(withSuffix, lookup.definition);
  free(withSuffix);
  if (!isSuffixEqual) {
    free(withoutSuffix);
    return;
  }

  // 3. Lookup the un-suffixed word
  StenoReverseDictionaryLookup lookupWithoutSuffix(
      withoutSuffix, lookup.ignoreStrokeThreshold);
  super::ReverseLookup(lookupWithoutSuffix);
  free(withoutSuffix);

  // 4. Verify that lookup up with suffix produces an invalid lookup.
  for (const StenoOrthographyReverseAutoSuffix *reverseAutoSuffix :
       test.replacements) {
    for (const StenoReverseDictionaryResult &entry :
         lookupWithoutSuffix.results) {
      const size_t length = entry.length;
      StenoStroke *strokes = entry.strokes;

      bool hasAdded = false;

      const StenoStroke previousStroke = strokes[length - 1];
      if ((previousStroke & reverseAutoSuffix->suppressMask).IsEmpty()) {
        strokes[length - 1] =
            previousStroke | reverseAutoSuffix->autoSuffix->stroke;

        if (lookup.HasResult(strokes, length)) {
          hasAdded = true;
        } else if (CanAutoSuffixLookup(strokes, length)) {
          // Even if it produced an invalid lookup, there's a chance that
          // another auto-suffix might take precedence. Check for that.
          StenoStroke testedStrokes(0);
          for (const StenoOrthographyAutoSuffix &autoSuffix :
               orthography.data.autoSuffixes) {
            if ((autoSuffix.stroke & ~testedStrokes).IsEmpty()) {
              continue;
            }
            testedStrokes |= autoSuffix.stroke;

            if (autoSuffix.stroke == reverseAutoSuffix->autoSuffix->stroke) {
              lookup.AddResult(strokes, length, this);
              hasAdded = true;
              break;
            }

            const StenoStroke previousAutoSuffixedStroke = strokes[length - 1];
            if ((previousAutoSuffixedStroke & autoSuffix.stroke).IsEmpty()) {
              continue;
            }

            strokes[length - 1] =
                previousAutoSuffixedStroke & ~autoSuffix.stroke;
            const bool hasValidLookup = HasValidLookup(strokes, length);
            strokes[length - 1] = previousAutoSuffixedStroke;
            if (hasValidLookup) {
              break;
            }
          }
        }

        strokes[length - 1] = previousStroke;
      }

      if (!hasAdded && length + 1 < lookup.ignoreStrokeThreshold) {
        StenoStroke strokesWithSuffixStroke[length + 1];
        strokes->CopyTo(strokesWithSuffixStroke, length);
        strokesWithSuffixStroke[length] = reverseAutoSuffix->autoSuffix->stroke;

        if (!super::HasOutline(strokesWithSuffixStroke, length + 1)) {
          lookup.AddResult(strokesWithSuffixStroke, length + 1, this);
        }
      }
    }
  }
}

bool StenoReverseAutoSuffixDictionary::HasValidLookup(
    const StenoStroke *strokes, size_t length) const {
  size_t i = length;
  for (;;) {
    if (super::HasOutline(strokes, i)) {
      length -= i;
      if (length == 0) {
        return true;
      }
      strokes += i;
      i = length;
      continue;
    } else if (--i == 0) {
      return false;
    }
  }
}

//---------------------------------------------------------------------------
