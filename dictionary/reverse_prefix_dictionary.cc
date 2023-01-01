//---------------------------------------------------------------------------

#include "reverse_prefix_dictionary.h"
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
  const uint8_t *FindPrefixChordData() const;
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
StenoReversePrefixDictionary::ReverseLookupContext::FindPrefixChordData()
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
  // Iterate text block and find all prefixes.
  const uint8_t *p = textBlock;
  const uint8_t *pEnd = textBlock + textBlockLength;

  // Initial guard
  assert(*p == 0xff);
  ++p;

  while (p < pEnd) {
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
    const uint8_t *prefixChordData;
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
    const uint8_t *prefixChordData = context.FindPrefixChordData();
    if (prefixChordData) {
      prefixTests.Add(PrefixTest{
          .lookup = lookup,
          .prefixChordData = prefixChordData,
      });
    }
    context.Narrow(*lookup++);
  }

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

      const uint8_t *prefixChords = test.prefixChordData;
      while (*prefixChords != 0xff) {
        uint32_t offset = prefixChords[0] | (prefixChords[1] << 7) |
                          (prefixChords[2] << 14) + (prefixChords[3] << 21);
        const void *data = baseAddress + offset;
        prefixChords += 4;

        StenoReverseMapDictionaryLookup prefixLookup(data);
        if (dictionary->ReverseMapDictionaryLookup(prefixLookup)) {
          StenoChord chords[prefixLookup.length + suffix.length];
          memcpy(chords, prefixLookup.chords,
                 prefixLookup.length * sizeof(StenoChord));
          memcpy(chords + prefixLookup.length, suffix.chords,
                 suffix.length * sizeof(StenoChord));

          result.AddResult(chords, prefixLookup.length + suffix.length, this);
          hasResult = true;
        }
      }
    }
    delete suffixLookup;

    if (hasResult) {
      return;
    }
  }
}

const char *StenoReversePrefixDictionary::GetName() const {
  return "#internal#reverse_prefix_dictionary";
}

//---------------------------------------------------------------------------
