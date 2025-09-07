//---------------------------------------------------------------------------

#include "reverse_suffix_dictionary.h"
#include "../container/list.h"
#include "../orthography.h"
#include "dictionary.h"
#include "javelin/pattern.h"
#include "map_data_lookup.h"
#include <assert.h>

//---------------------------------------------------------------------------

struct StenoReverseSuffixDictionary::Suffix {
  // suffix points to the last letter of the suffix.
  //
  // Suffixes have form "{^suffix}\0<MapData>"
  const uint8_t *suffix;

  const uint8_t *GetText(size_t suffixLength) const {
    // -1 is for the preceding "{"
    return suffix - suffixLength - 1;
  }

  char *CreateOrthographySuffix(size_t suffixLength) const {
    const uint8_t *text = GetText(suffixLength);
    return Str::DupN(text + 2, suffix - text - 1);
  }

  const uint8_t *GetMapLookupData() const { return suffix + 3; };
};

struct StenoReverseSuffixDictionary::ReverseSuffix {
  Pattern testPattern;
  const char *replacement;
};

//---------------------------------------------------------------------------

struct StenoReverseSuffixDictionary::ReverseLookupContext {
  size_t characterIndex = 0;
  const Suffix *left;
  const Suffix *right;

  bool IsValid() const { return left < right; }
  void Narrow(uint8_t c);

  // Binary search for '^', then confirm suffix of '^}\0'.
  // This is because ascii wise, ^ lies between upper case and lower case
  // letters.
  const Suffix *FindSuffixLookup() const;
};

void StenoReverseSuffixDictionary::ReverseLookupContext::Narrow(uint8_t c) {
  // Update left
  const Suffix *l = left;
  const Suffix *r = right;
  while (l < r) {
    const Suffix *mid = l + ((r - l) >> 1);
    if (mid->suffix[characterIndex] < c) {
      l = mid + 1;
    } else {
      r = mid;
    }
  }
  left = l;

  // Update right, using new l.
  r = right;
  while (l < r) {
    const Suffix *mid = l + ((r - l) >> 1);
    if (mid->suffix[characterIndex] <= c) {
      l = mid + 1;
    } else {
      r = mid;
    }
  }
  right = r;

  --characterIndex;
}

const StenoReverseSuffixDictionary::Suffix *
StenoReverseSuffixDictionary::ReverseLookupContext::FindSuffixLookup() const {
  const Suffix *l = left;
  const Suffix *r = right;

  while (l < r) {
    const Suffix *mid = l + ((r - l) >> 1);
    const uint8_t *midString = mid->suffix + characterIndex;
    const uint8_t c = midString[0];
    if (c < '^') {
      l = mid + 1;
    } else if (c > '^') {
      r = mid;
    } else {
      // Equal!
      assert(midString[-1] == '{');
      return mid;
    }
  }
  return nullptr;
}

//---------------------------------------------------------------------------

StenoReverseSuffixDictionary::StenoReverseSuffixDictionary(
    StenoDictionary *dictionary, const uint8_t *baseAddress,
    const SizedList<StenoOrthographyRule> &reverseSuffixes,
    const StenoCompiledOrthography &orthography,
    const StenoDictionary *prefixDictionary,
    const SizedList<const uint8_t *> suffixes,
    const List<const uint8_t *> &ignoreSuffixes)
    : StenoWrappedDictionary(dictionary), baseAddress(baseAddress),
      reversePatterns(CreateReversePatterns(reverseSuffixes)),
      suffixes(CreateSuffixList(suffixes, ignoreSuffixes)),
      orthography(orthography), prefixDictionary(prefixDictionary) {
  for (const ReverseSuffix &suffix : reversePatterns) {
    mergedQuickReject.Merge(suffix.testPattern.GetQuickReject());
  }
}

const SizedList<StenoReverseSuffixDictionary::ReverseSuffix>
StenoReverseSuffixDictionary::CreateReversePatterns(
    const SizedList<StenoOrthographyRule> &patterns) {
  ReverseSuffix *result =
      (ReverseSuffix *)malloc(sizeof(ReverseSuffix) * patterns.GetCount());
  for (size_t i = 0; i < patterns.GetCount(); ++i) {
    result[i].testPattern = Pattern::Compile(patterns[i].testPattern);
    result[i].replacement = patterns[i].replacement;
  }
  return SizedList<ReverseSuffix>{
      .count = patterns.GetCount(),
      .data = result,
  };
}

SizedList<StenoReverseSuffixDictionary::Suffix>
StenoReverseSuffixDictionary::CreateSuffixList(
    const SizedList<const uint8_t *> suffixes,
    const List<const uint8_t *> &ignoreSuffixes) {
  SizedList<Suffix> filteredSuffixes =
      SizedList<Suffix>::CreateWithCapacity(suffixes.GetCount());

  const uint8_t *ignoreSuffix = nullptr;
  size_t ignoreSuffixIndex = 0;
  if (ignoreSuffixes.IsNotEmpty()) {
    ignoreSuffix = ignoreSuffixes[ignoreSuffixIndex++];
  }

  for (const uint8_t *suffix : suffixes) {
    if (suffix != ignoreSuffix) [[likely]] {
      filteredSuffixes.Add(Suffix{.suffix = suffix});
    } else if (ignoreSuffixIndex < ignoreSuffixes.GetCount()) {
      ignoreSuffix = ignoreSuffixes[ignoreSuffixIndex++];
    } else {
      ignoreSuffix = nullptr;
    }
  }

  return filteredSuffixes;
}

void StenoReverseSuffixDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  super::ReverseLookup(lookup);
  if (lookup.definitionLength > 1 && lookup.ignoreStrokeThreshold > 2 &&
      !Str::Contains(lookup.definition, ' ')) {
    ReverseLookupContext context;
    context.left = begin(suffixes);
    context.right = end(suffixes);
    AddSuffixReverseLookup(context, lookup);
  }
}

void StenoReverseSuffixDictionary::AddSuffixReverseLookup(
    ReverseLookupContext &context, StenoReverseDictionaryLookup &lookup) const {
  const size_t length = Str::Length(lookup.definition);
  if (length <= 2) {
    return;
  }
  const uint8_t *definitionEnd = (const uint8_t *)lookup.definition + length;
  const uint8_t *definition = definitionEnd;
  context.Narrow(*--definition);

  // Build a list of suffixes to test.
  struct Test {
    const uint8_t *prefixEnd;
    const Suffix *suffix;
  };

  List<Test> tests;
  while (definition > (const uint8_t *)lookup.definition + 1) {
    if (!context.IsValid()) {
      break;
    }

    const Suffix *suffix = context.FindSuffixLookup();
    if (suffix) {
      tests.Add(Test{
          .prefixEnd = definition,
          .suffix = suffix,
      });
    }
    context.Narrow(*--definition);
  }

  for (const Test &test : tests) {
    // Create the without suffix version, add the suffix, and verify it matches.
    const size_t prefixLength =
        test.prefixEnd - (const uint8_t *)lookup.definition;
    const size_t suffixLength = definitionEnd - test.prefixEnd;
    char *withoutSuffix = Str::DupN(lookup.definition, prefixLength);
    AddSuffixReverseLookup(context, lookup, withoutSuffix, test.suffix,
                           suffixLength);

    const PatternQuickReject textReject(withoutSuffix);

    if (mergedQuickReject.IsPossibleMergeMatch(textReject)) {
      for (const ReverseSuffix &revereSuffix : reversePatterns) {
        if (!revereSuffix.testPattern.IsPossibleMatch(textReject)) {
          continue;
        }
        const PatternMatch match =
            revereSuffix.testPattern.Match(withoutSuffix);
        if (!match.match) {
          continue;
        }
        char *possiblePrefix = match.Replace(revereSuffix.replacement);
        AddSuffixReverseLookup(context, lookup, possiblePrefix, test.suffix,
                               suffixLength);
        free(possiblePrefix);
      }
    }

    free(withoutSuffix);
  }
}

void StenoReverseSuffixDictionary::AddSuffixReverseLookup(
    ReverseLookupContext &context, StenoReverseDictionaryLookup &lookup,
    const char *withoutSuffix, const Suffix *suffix,
    size_t suffixLength) const {
  char *orthographySuffix = suffix->CreateOrthographySuffix(suffixLength);
  char *withSuffix = orthography.AddSuffix(withoutSuffix, orthographySuffix);
  free(orthographySuffix);

  const bool isWordEqual = Str::Eq(withSuffix, lookup.definition);
  free(withSuffix);
  if (!isWordEqual) {
    return;
  }

  StenoReverseDictionaryLookup *prefixLookup = new StenoReverseDictionaryLookup(
      withoutSuffix, lookup.ignoreStrokeThreshold - 1);

  prefixDictionary->ReverseLookup(*prefixLookup);

  if (prefixLookup->HasResults()) {
    // Prefix lookup succeeded, get the suffixes.
    const size_t minimumPrefixStrokeCount =
        prefixLookup->GetMinimumStrokeCount();
    if (lookup.ignoreStrokeThreshold > minimumPrefixStrokeCount + 1) {
      StenoReverseDictionaryLookup *suffixLookup =
          new StenoReverseDictionaryLookup(
              (const char *)suffix->GetText(suffixLength),
              lookup.ignoreStrokeThreshold - minimumPrefixStrokeCount);

      // Add map lookup hints.
      suffixLookup->mapLookupData.Add(suffix->GetMapLookupData(), baseAddress);

      super::ReverseLookup(*suffixLookup);

      if (suffixLookup->HasResults()) {
        for (const StenoReverseDictionaryResult &prefix :
             prefixLookup->results) {
          for (const StenoReverseDictionaryResult &suffix :
               suffixLookup->results) {
            const size_t combinedLength = prefix.length + suffix.length;
            if (combinedLength >= lookup.ignoreStrokeThreshold) {
              continue;
            }
            StenoStroke strokes[combinedLength];
            prefix.strokes->CopyTo(strokes, prefix.length);
            suffix.strokes->CopyTo(strokes + prefix.length, suffix.length);

            if (!IsStrokeDefined(strokes, prefix.length, combinedLength)) {
              lookup.AddResult(strokes, combinedLength, this);
            }
          }
        }
      }
      delete suffixLookup;
    }
  }

  delete prefixLookup;
}

bool StenoReverseSuffixDictionary::IsStrokeDefined(
    const StenoStroke *strokes, size_t prefixStrokeCount,
    size_t combinedStrokeCount) const {
  for (size_t i = prefixStrokeCount + 1; i <= combinedStrokeCount; ++i) {
    if (super::HasOutline(strokes, i)) {
      return true;
    }
  }
  return false;
}

const char *StenoReverseSuffixDictionary::GetName() const {
  return "#internal#reverse_suffix_dictionary";
}

//---------------------------------------------------------------------------
