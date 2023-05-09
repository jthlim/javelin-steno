//---------------------------------------------------------------------------

#include "reverse_prefix_dictionary.h"
#include "../list.h"
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
  size_t characterIndex = 0;
  const uint8_t **left;
  const uint8_t **right;

  bool IsValid() const { return left < right; }
  void Narrow(uint8_t c);

  // Binary search for '^', then confirm suffix of '^}\0'.
  const uint8_t *FindPrefixStrokeData() const;
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
StenoReversePrefixDictionary::ReverseLookupContext::FindPrefixStrokeData()
    const {
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
      return midString + 3;
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
      handler.AddPrefix(wordStart + 1);
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
  struct PrefixTest {
    const uint8_t *lookup;
    const uint8_t *prefixStrokeData;
  };

  const uint8_t *lookup = (const uint8_t *)result.lookup;
  if (*lookup == '\0') {
    return;
  }
  context.Narrow(*lookup++);

  List<PrefixTest> prefixTests;

  while (*lookup) {
    if (!context.IsValid()) {
      break;
    }
    const uint8_t *prefixStrokeData = context.FindPrefixStrokeData();
    if (prefixStrokeData) {
      prefixTests.Add(PrefixTest{
          .lookup = lookup,
          .prefixStrokeData = prefixStrokeData,
      });
    }
    context.Narrow(*lookup++);
  }

  // Do reverse ordering to find longest matches first.
  for (size_t i = prefixTests.GetCount(); i > 0; --i) {
    const PrefixTest &test = prefixTests[i - 1];
    StenoReverseDictionaryLookup *suffixLookup =
        new StenoReverseDictionaryLookup(result.strokeThreshold - 1,
                                         (const char *)test.lookup);

    // Try the prefix lookups.
    ReverseLookup(*suffixLookup);

    bool hasResult = false;
    for (size_t i = 0; i < suffixLookup->resultCount; ++i) {
      const StenoReverseDictionaryResult &suffix = suffixLookup->results[i];

      const uint8_t *prefixStrokes = test.prefixStrokeData;
      while (*prefixStrokes != 0xff) {
        uint32_t offset = prefixStrokes[0] | (prefixStrokes[1] << 7) |
                          (prefixStrokes[2] << 14) + (prefixStrokes[3] << 21);
        const void *data = baseAddress + offset;
        prefixStrokes += 4;

        StenoReverseMapDictionaryLookup prefixLookup(data);
        if (dictionary->ReverseMapDictionaryLookup(prefixLookup)) {
          size_t combinedLength = prefixLookup.length + suffix.length;
          StenoStroke strokes[combinedLength];
          memcpy(strokes, prefixLookup.strokes,
                 prefixLookup.length * sizeof(StenoStroke));
          memcpy(strokes + prefixLookup.length, suffix.strokes,
                 suffix.length * sizeof(StenoStroke));

          if (!IsStrokeDefined(strokes, prefixLookup.length, combinedLength)) {
            result.AddResult(strokes, combinedLength, this);
            hasResult = true;
          }
        }
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
