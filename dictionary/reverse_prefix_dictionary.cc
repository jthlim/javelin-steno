//---------------------------------------------------------------------------

#include "reverse_prefix_dictionary.h"
#include "../list.h"
#include "dictionary.h"
#include <assert.h>

//---------------------------------------------------------------------------

class StenoReversePrefixDictionary::TextBlockHandler {
public:
  virtual void AddPrefix(const uint8_t *prefix) = 0;
};

class CountTextBlockHandler
    : public StenoReversePrefixDictionary::TextBlockHandler {
public:
  virtual void AddPrefix(const uint8_t *prefix) { ++counter; }

  size_t counter = 0;
};

class PopulateTextBlockHandler
    : public StenoReversePrefixDictionary::TextBlockHandler {
public:
  PopulateTextBlockHandler(const uint8_t **prefixes) : prefixes(prefixes) {}

  virtual void AddPrefix(const uint8_t *prefix) { *prefixes++ = prefix; }

  const uint8_t **prefixes;
};

//---------------------------------------------------------------------------

struct StenoReversePrefixDictionary::ReverseLookupContext {
  size_t characterIndex = 1;
  const uint8_t **left;
  const uint8_t **right;

  bool IsValid() const { return left < right; }
  void Narrow(uint8_t c);

  // Binary search for '^', then confirm suffix of '^}\0'.
  // This is because ascii wise, ^ lies between upper case and lower case
  // letters.
  const uint8_t *FindPrefixLookup() const;
};

void StenoReversePrefixDictionary::ReverseLookupContext::Narrow(uint8_t c) {

  // Update left
  const uint8_t **l = left;
  const uint8_t **r = right;
  while (l < r) {
    const uint8_t **mid = l + ((r - l) >> 1);
    uint8_t cm = (*mid)[characterIndex];
    if (cm < c) {
      l = mid + 1;
    } else {
      r = mid;
    }
  }
  left = l;

  // Update right, using new l.
  r = right;
  while (l < r) {
    const uint8_t **mid = l + ((r - l) >> 1);
    uint8_t cm = (*mid)[characterIndex];
    if (cm <= c) {
      l = mid + 1;
    } else {
      r = mid;
    }
  }
  right = r;

  ++characterIndex;
}

const uint8_t *
StenoReversePrefixDictionary::ReverseLookupContext::FindPrefixLookup() const {
  const uint8_t **l = left;
  const uint8_t **r = right;

  while (l < r) {
    const uint8_t **mid = l + ((r - l) >> 1);
    const uint8_t *midString = (*mid) + characterIndex;
    uint8_t c = midString[0];
    if (c < '^') {
      l = mid + 1;
    } else if (c > '^') {
      r = mid;
    } else {
      // Equal!
      assert(midString[1] == '}' && midString[2] == '\0');
      return *mid;
    }
  }
  return nullptr;
}

//---------------------------------------------------------------------------

StenoReversePrefixDictionary::StenoReversePrefixDictionary(
    StenoDictionary *dictionary, const uint8_t *baseAddress,
    const uint8_t *textBlock, size_t textBlockLength)
    : StenoWrappedDictionary(dictionary), baseAddress(baseAddress) {

  CountTextBlockHandler counter;
  ProcessTextBlock(textBlock, textBlockLength, counter);

  prefixCount = counter.counter;
  prefixes = new const uint8_t *[prefixCount];

  PopulateTextBlockHandler populate(prefixes);
  ProcessTextBlock(textBlock, textBlockLength, populate);
}

void StenoReversePrefixDictionary::ProcessTextBlock(const uint8_t *textBlock,
                                                    size_t textBlockLength,
                                                    TextBlockHandler &handler) {
  // Binary search to find the start of all commands.
  const uint8_t *left = textBlock + 1;
  const uint8_t *right = textBlock + textBlockLength;

  while (left < right) {
#if JAVELIN_PLATFORM_PICO_SDK || JAVELIN_PLATFORM_NRF5_SDK
    // Optimization when top bit of pointer cannot be set.
    const uint8_t *mid = (const uint8_t *)((size_t(left) + size_t(right)) / 2);
#else
    const uint8_t *mid = left + size_t(right - left) / 2;
#endif

    const uint8_t *wordStart = mid;
    while (wordStart[-1] != 0xff) {
      --wordStart;
    }

    if (*wordStart >= '{') {
      right = wordStart;
    } else {
      while (*wordStart != 0xff) {
        ++wordStart;
      }
      left = wordStart + 1;
    }
  }

  // Iterate text block and find all prefixes.
  const uint8_t *p = right;

  while (*p == '{') {
    const uint8_t *wordStart = p;
    int braceCounter = 0;
    int caretCounter = 0;

    // Search for '\0'
    while (*p) {
      if (*p == '{' || *p == '}') {
        ++braceCounter;
      }
      if (*p == '^') {
        ++caretCounter;
      }
      ++p;
    }
    ++p;

    // Word end.
    if (braceCounter == 2 && caretCounter == 1 && *wordStart == '{' &&
        p[-2] == '}' && p[-3] == '^' && p - wordStart > 4) {
      // Since the textblock is already sorted, entries added here are
      // sorted too.
      handler.AddPrefix(wordStart);
    }

    // Search for end marker
    while (*p != 0xff) {
      ++p;
    }
    ++p;
  }
}

void StenoReversePrefixDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  dictionary->ReverseLookup(result);
  if (result.strokeThreshold > 2) {
    ReverseLookupContext context;
    context.left = prefixes;
    context.right = prefixes + prefixCount;
    AddPrefixReverseLookup(context, result);
  }
}

void StenoReversePrefixDictionary::AddPrefixReverseLookup(
    ReverseLookupContext &context, StenoReverseDictionaryLookup &result) const {

  // Character by character lookup
  const uint8_t *lookup = (const uint8_t *)result.lookup;
  if (*lookup == '\0') {
    return;
  }
  context.Narrow(*lookup++);

  // Build a list of suffixes to prefixes to use and suffixes to test
  struct Test {
    const uint8_t *prefix;
    const uint8_t *suffix;
  };
  List<Test> tests;
  while (*lookup) {
    if (!context.IsValid()) {
      break;
    }
    const uint8_t *prefix = context.FindPrefixLookup();
    if (prefix) {
      tests.Add(Test{
          .prefix = prefix,
          .suffix = lookup,
      });
    }
    context.Narrow(*lookup++);
  }

  // Do reverse ordering to find longest matches first.
  for (size_t i = tests.GetCount(); i > 0; --i) {
    const Test &test = tests[i - 1];
    StenoReverseDictionaryLookup *suffixLookup =
        new StenoReverseDictionaryLookup(result.strokeThreshold - 1,
                                         (const char *)test.suffix);

    ReverseLookup(*suffixLookup);

    bool hasResult = false;
    if (suffixLookup->HasResults()) {
      // Suffix lookup succeeded.
      StenoReverseDictionaryLookup *prefixLookup =
          new StenoReverseDictionaryLookup(
              result.strokeThreshold - suffixLookup->GetMinimumStrokeCount(),
              (const char *)test.prefix);

      // Add map lookup hints.
      size_t prefixLength = test.suffix - (const uint8_t *)result.lookup;

      // Given format '{prefix^}\0', skip 4 extra bytes to get to prefix strokes
      const uint8_t *prefixStrokes = test.prefix + prefixLength + 4;
      while (*prefixStrokes != 0xff) {
        uint32_t offset = prefixStrokes[0] | (prefixStrokes[1] << 7) |
                          (prefixStrokes[2] << 14) + (prefixStrokes[3] << 21);
        const void *data = baseAddress + offset;
        prefixStrokes += 4;

        prefixLookup->mapDataLookup[prefixLookup->mapDataLookupCount++] = data;
        if (prefixLookup->mapDataLookupCount >=
            StenoReverseDictionaryLookup::MAX_MAP_DATA_LOOKUP_COUNT) {
          break;
        }
      }

      dictionary->ReverseLookup(*prefixLookup);

      if (prefixLookup->HasResults()) {
        for (size_t p = 0; p < prefixLookup->resultCount; p++) {
          StenoReverseDictionaryResult &prefix = prefixLookup->results[p];
          for (size_t s = 0; s < suffixLookup->resultCount; ++s) {
            StenoReverseDictionaryResult &suffix = suffixLookup->results[s];
            size_t combinedLength = prefix.length + suffix.length;
            StenoStroke strokes[combinedLength];
            memcpy(strokes, prefix.strokes,
                   prefix.length * sizeof(StenoStroke));
            memcpy(strokes + prefix.length, suffix.strokes,
                   suffix.length * sizeof(StenoStroke));

            if (!IsStrokeDefined(strokes, prefix.length, combinedLength)) {
              result.AddResult(strokes, combinedLength, this);
              hasResult = true;
            }
          }
        }
      }
      delete prefixLookup;
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
    if (dictionary->GetLookupProvider(strokes, i)) {
      return true;
    }
  }
  return false;
}

const char *StenoReversePrefixDictionary::GetName() const {
  return "#internal#reverse_prefix_dictionary";
}

//---------------------------------------------------------------------------
