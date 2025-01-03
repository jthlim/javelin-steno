//---------------------------------------------------------------------------

#include "reverse_prefix_dictionary.h"
#include "../list.h"
#include "dictionary.h"
#include "map_data_lookup.h"
#include <assert.h>

//---------------------------------------------------------------------------

struct StenoReversePrefixDictionary::Prefix {
  // Prefixes have form "{prefix^}\0<MapData>"
  const uint8_t *text;

  const char *GetText() const { return (const char *)text; }

  const uint8_t *GetMapLookupData(size_t prefixLength) const {
    return text + prefixLength + 4;
  }
};

//---------------------------------------------------------------------------

struct StenoReversePrefixDictionary::ReverseLookupContext {
  size_t characterIndex = 1;
  const Prefix *left;
  const Prefix *right;

  bool IsValid() const { return left < right; }
  void Narrow(uint8_t c);

  // Binary search for '^', then confirm suffix of '^}\0'.
  // This is because ascii wise, ^ lies between upper case and lower case
  // letters.
  const Prefix *FindPrefixLookup() const;
};

void StenoReversePrefixDictionary::ReverseLookupContext::Narrow(uint8_t c) {

  // Update left
  const Prefix *l = left;
  const Prefix *r = right;
  while (l < r) {
    const Prefix *mid = l + ((r - l) >> 1);
    if (mid->text[characterIndex] < c) {
      l = mid + 1;
    } else {
      r = mid;
    }
  }
  left = l;

  // Update right, using new l.
  r = right;
  while (l < r) {
    const Prefix *mid = l + ((r - l) >> 1);
    if (mid->text[characterIndex] <= c) {
      l = mid + 1;
    } else {
      r = mid;
    }
  }
  right = r;

  ++characterIndex;
}

const StenoReversePrefixDictionary::Prefix *
StenoReversePrefixDictionary::ReverseLookupContext::FindPrefixLookup() const {
  const Prefix *l = left;
  const Prefix *r = right;

  while (l < r) {
    const Prefix *mid = l + ((r - l) >> 1);
    const uint8_t *midString = mid->text + characterIndex;
    const uint8_t c = midString[0];
    if (c < '^') {
      l = mid + 1;
    } else if (c > '^') {
      r = mid;
    } else {
      // Equal!
      assert(midString[1] == '}' && midString[2] == '\0');
      return mid;
    }
  }
  return nullptr;
}

//---------------------------------------------------------------------------

StenoReversePrefixDictionary::StenoReversePrefixDictionary(
    StenoDictionary *dictionary, const uint8_t *baseAddress,
    const SizedList<const uint8_t *> prefixes)
    : StenoWrappedDictionary(dictionary), baseAddress(baseAddress),
      prefixes(prefixes.Copy().Cast<Prefix>()) {}

void StenoReversePrefixDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  super::ReverseLookup(lookup);
  if (lookup.definitionLength > 1 && lookup.strokeThreshold > 2 &&
      lookup.prefixLookupDepth < MAXIMUM_REVERSE_PREFIX_DEPTH &&
      !Str::Contains(lookup.definition, ' ')) {
    ReverseLookupContext context;
    context.left = begin(prefixes);
    context.right = end(prefixes);
    AddPrefixReverseLookup(context, lookup);
  }
}

void StenoReversePrefixDictionary::AddPrefixReverseLookup(
    ReverseLookupContext &context, StenoReverseDictionaryLookup &lookup) const {

  // Character by character lookup
  const uint8_t *definition = (const uint8_t *)lookup.definition;
  if (*definition == '\0') {
    return;
  }
  context.Narrow(*definition++);

  // Build a list of suffixes to prefixes to use and suffixes to test
  struct Test {
    const Prefix *prefix;
    const char *suffix;
  };

  List<Test> tests;
  while (*definition) {
    if (!context.IsValid()) {
      break;
    }

    const Prefix *prefix = context.FindPrefixLookup();
    if (prefix) {
      tests.Add(Test{
          .prefix = prefix,
          .suffix = (const char *)definition,
      });
    }
    context.Narrow(*definition++);
  }

  // Do reverse ordering to find longest matches first.
  for (const Test &test : tests.Reverse()) {
    // In theory, this should use:
    //   result.strokeThreshold - minimumStrokesForPrefix
    // Use "1" as an quick approximation.
    StenoReverseDictionaryLookup *suffixLookup =
        new StenoReverseDictionaryLookup(test.suffix,
                                         lookup.strokeThreshold - 1);

    suffixLookup->prefixLookupDepth = lookup.prefixLookupDepth + 1;
    ReverseLookup(*suffixLookup);

    bool hasResult = false;
    if (suffixLookup->HasResults()) {
      // Suffix lookup succeeded.
      const size_t minimumSuffixStrokeCount =
          suffixLookup->GetMinimumStrokeCount();
      if (lookup.strokeThreshold > minimumSuffixStrokeCount + 1) {
        StenoReverseDictionaryLookup *prefixLookup =
            new StenoReverseDictionaryLookup(test.prefix->GetText(),
                                             lookup.strokeThreshold -
                                                 minimumSuffixStrokeCount);

        // Add map lookup hints.
        const size_t prefixLength = test.suffix - lookup.definition;
        prefixLookup->AddMapLookupData(
            test.prefix->GetMapLookupData(prefixLength), baseAddress);

        super::ReverseLookup(*prefixLookup);

        if (prefixLookup->HasResults()) {
          // Prioritize shorter results if they can't all fit in.
          if (suffixLookup->results.GetCount() *
                  prefixLookup->results.GetCount() >
              lookup.results.GetRemainingCapacity()) {
            suffixLookup->SortResults();
          }

          for (const StenoReverseDictionaryResult &suffix :
               suffixLookup->results) {
            for (const StenoReverseDictionaryResult &prefix :
                 prefixLookup->results) {
              const size_t combinedLength = prefix.length + suffix.length;
              if (combinedLength >= lookup.strokeThreshold) {
                continue;
              }
              StenoStroke strokes[combinedLength];
              prefix.strokes->CopyTo(strokes, prefix.length);
              suffix.strokes->CopyTo(strokes + prefix.length, suffix.length);

              if (!IsStrokeDefined(strokes, prefix.length, combinedLength)) {
                lookup.AddResult(strokes, combinedLength, this);
                hasResult = true;
              }
            }
          }
        }
        delete prefixLookup;
      }
    }

    delete suffixLookup;

    if (hasResult) {
      return;
    }
  }
}

bool StenoReversePrefixDictionary::IsStrokeDefined(
    const StenoStroke *strokes, size_t prefixStrokeCount,
    size_t combinedStrokeCount) const {
  for (size_t i = prefixStrokeCount + 1; i <= combinedStrokeCount; ++i) {
    if (super::HasOutline(strokes, i)) {
      return true;
    }
  }
  return false;
}

const char *StenoReversePrefixDictionary::GetName() const {
  return "#internal#reverse_prefix_dictionary";
}

//---------------------------------------------------------------------------
