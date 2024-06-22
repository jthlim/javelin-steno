//---------------------------------------------------------------------------

#include "reverse_suffix_dictionary.h"
#include "../list.h"
#include "../orthography.h"
#include "dictionary.h"
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

  const uint8_t *GetMapDataLookup() const { return suffix + 3; };
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
    const StenoCompiledOrthography &orthography,
    const StenoDictionary *prefixDictionary,
    const SizedList<const uint8_t *> suffixes,
    const List<const uint8_t *> &ignoreSuffixes)
    : StenoWrappedDictionary(dictionary), baseAddress(baseAddress),
      suffixes(CreateSuffixList(suffixes, ignoreSuffixes)),
      orthography(orthography), prefixDictionary(prefixDictionary) {}

SizedList<StenoReverseSuffixDictionary::Suffix>
StenoReverseSuffixDictionary::CreateSuffixList(
    const SizedList<const uint8_t *> suffixes,
    const List<const uint8_t *> &ignoreSuffixes) {
  SizedList<Suffix> filteredList =
      SizedList<Suffix>::CreateWithCapacity(suffixes.GetCount());

  for (const uint8_t *suffix : suffixes) {
    if (!ignoreSuffixes.Contains(suffix)) {
      filteredList.Add(Suffix{.suffix = suffix});
    }
  }

  return filteredList;
}

void StenoReverseSuffixDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  dictionary->ReverseLookup(lookup);
  if (lookup.definitionLength > 1 && lookup.strokeThreshold > 2 &&
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
    char *withoutSuffix = Str::DupN(lookup.definition, prefixLength);

    const size_t suffixLength = definitionEnd - test.prefixEnd;
    char *orthographySuffix =
        test.suffix->CreateOrthographySuffix(suffixLength);
    char *withSuffix = orthography.AddSuffix(withoutSuffix, orthographySuffix);
    free(orthographySuffix);

    const bool isSuffixEqual = Str::Eq(withSuffix, lookup.definition);
    free(withSuffix);
    if (!isSuffixEqual) {
      free(withoutSuffix);
      continue;
    }

    StenoReverseDictionaryLookup *prefixLookup =
        new StenoReverseDictionaryLookup(lookup.strokeThreshold - 1,
                                         withoutSuffix);

    prefixDictionary->ReverseLookup(*prefixLookup);
    free(withoutSuffix);

    if (prefixLookup->HasResults()) {
      // Prefix lookup succeeded, get the suffixes.
      const size_t minimumPrefixStrokeCount =
          prefixLookup->GetMinimumStrokeCount();
      if (lookup.strokeThreshold > minimumPrefixStrokeCount + 1) {
        StenoReverseDictionaryLookup *suffixLookup =
            new StenoReverseDictionaryLookup(
                lookup.strokeThreshold - minimumPrefixStrokeCount,
                (const char *)test.suffix->GetText(suffixLength));

        // Add map lookup hints.
        suffixLookup->AddMapDataLookup(test.suffix->GetMapDataLookup(),
                                       baseAddress);

        dictionary->ReverseLookup(*suffixLookup);

        if (suffixLookup->HasResults()) {
          for (const StenoReverseDictionaryResult &prefix :
               prefixLookup->results) {
            for (const StenoReverseDictionaryResult &suffix :
                 suffixLookup->results) {
              const size_t combinedLength = prefix.length + suffix.length;
              if (combinedLength >= lookup.strokeThreshold) {
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
}

bool StenoReverseSuffixDictionary::IsStrokeDefined(
    const StenoStroke *strokes, size_t prefixStrokeCount,
    size_t combinedStrokeCount) const {
  for (size_t i = prefixStrokeCount + 1; i <= combinedStrokeCount; ++i) {
    if (dictionary->HasOutline(strokes, i)) {
      return true;
    }
  }
  return false;
}

const char *StenoReverseSuffixDictionary::GetName() const {
  return "#internal#reverse_suffix_dictionary";
}

//---------------------------------------------------------------------------
