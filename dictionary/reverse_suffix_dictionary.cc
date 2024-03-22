//---------------------------------------------------------------------------

#include "reverse_suffix_dictionary.h"
#include "../list.h"
#include "../orthography.h"
#include "dictionary.h"
#include "map_data_lookup.h"
#include <assert.h>

//---------------------------------------------------------------------------

struct StenoReverseSuffixDictionary::Suffix {
  const uint8_t *text;
  // suffix points to the last letter of the suffix.
  const uint8_t *suffix;

  char *CreateOrthographySuffix() const {
    return Str::DupN(text + 2, suffix - text - 1);
  }
  const uint8_t *GetMapDataLookup() const { return suffix + 3; };
};

class StenoReverseSuffixDictionary::TextBlockHandler {
public:
  virtual void AddSuffix(const uint8_t *text, const uint8_t *suffix) = 0;
};

class StenoReverseSuffixDictionary::CountTextBlockHandler
    : public TextBlockHandler {
public:
  virtual void AddSuffix(const uint8_t *text, const uint8_t *suffix) {
    ++counter;
  }

  size_t counter = 0;
};

class StenoReverseSuffixDictionary::PopulateTextBlockHandler
    : public TextBlockHandler {
public:
  PopulateTextBlockHandler(StenoReverseSuffixDictionary::Suffix *suffixes)
      : suffixes(suffixes) {}

  virtual void AddSuffix(const uint8_t *text, const uint8_t *suffix) {
    suffixes->text = text;
    suffixes->suffix = suffix;
    ++suffixes;
  }

  StenoReverseSuffixDictionary::Suffix *suffixes;
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
    uint8_t cm = mid->suffix[characterIndex];
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
    const Suffix *mid = l + ((r - l) >> 1);
    uint8_t cm = mid->suffix[characterIndex];
    if (cm <= c) {
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
    uint8_t c = midString[0];
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
    const uint8_t *textBlock, size_t textBlockLength,
    const StenoCompiledOrthography &orthography,
    const StenoDictionary *prefixDictionary)
    : StenoWrappedDictionary(dictionary), baseAddress(baseAddress),
      orthography(orthography), prefixDictionary(prefixDictionary) {

  CountTextBlockHandler counter;
  ProcessTextBlock(textBlock, textBlockLength, counter);

  suffixCount = counter.counter;
  Suffix *suffixes = (Suffix *)malloc(suffixCount * sizeof(Suffix));
  this->suffixes = suffixes;

  PopulateTextBlockHandler populate(suffixes);
  ProcessTextBlock(textBlock, textBlockLength, populate);

  qsort(suffixes, suffixCount, sizeof(Suffix),
        [](const void *a, const void *b) -> int {
          const uint8_t *sa = ((const Suffix *)a)->suffix;
          const uint8_t *sb = ((const Suffix *)b)->suffix;
          for (;;) {
            if (*sa == '{') {
              return -1;
            }
            if (*sb == '{') {
              return 1;
            }
            int v = *sa - *sb;
            if (v != 0) {
              return v;
            }
            --sa;
            --sb;
          }
        });
}

void StenoReverseSuffixDictionary::ProcessTextBlock(const uint8_t *textBlock,
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

    uint32_t value = (wordStart[0] << 8) | wordStart[1];

    if (value >= ('{' * 256 | '^')) {
      right = wordStart;
    } else {
      while (*mid != 0xff) {
        ++mid;
      }
      left = mid + 1;
    }
  }

  // Iterate text block and find all suffixes.
  const uint8_t *p = right;

  while (p[0] == '{' && p[1] == '^') {
    const uint8_t *wordStart = p;
    int braceCounter = 0;
    int caretCounter = 0;
    bool hasSpace = false;

    // Search for '\0'
    while (*p) {
      if (*p == '{' || *p == '}') {
        ++braceCounter;
      }
      if (*p == '^') {
        ++caretCounter;
      }
      if (*p == ' ') {
        hasSpace = true;
        while (*p) {
          ++p;
        }
        break;
      }
      ++p;
    }
    ++p;

    // Word end.
    if (!hasSpace && braceCounter == 2 && caretCounter == 1 &&
        wordStart[0] == '{' && wordStart[1] == '^' && p[-2] == '}' &&
        p - wordStart > 4) {
      if (!orthography.IsAutoSuffix((const char *)wordStart)) {
        handler.AddSuffix(wordStart, p - 3);
      }
    }

    // Search for end marker
    MapDataLookup data(p);
    while (data.HasData()) {
      ++data;
    }
    p = data.GetPointer() + 1;
  }
}

void StenoReverseSuffixDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  dictionary->ReverseLookup(result);
  if (result.lookupLength > 1 && result.strokeThreshold > 2 &&
      !Str::Contains(result.lookup, ' ')) {
    ReverseLookupContext context;
    context.left = suffixes;
    context.right = suffixes + suffixCount;
    AddSuffixReverseLookup(context, result);
  }
}

void StenoReverseSuffixDictionary::AddSuffixReverseLookup(
    ReverseLookupContext &context, StenoReverseDictionaryLookup &result) const {
  size_t length = Str::Length(result.lookup);
  if (length <= 2) {
    return;
  }
  const uint8_t *lookup = (const uint8_t *)result.lookup + length;
  context.Narrow(*--lookup);

  // Build a list of suffixes to test.
  struct Test {
    const uint8_t *prefixEnd;
    const Suffix *suffix;
  };
  List<Test> tests;
  while (lookup > (const uint8_t *)result.lookup + 2) {
    if (!context.IsValid()) {
      break;
    }
    const Suffix *suffix = context.FindSuffixLookup();
    if (suffix) {
      tests.Add(Test{
          .prefixEnd = lookup,
          .suffix = suffix,
      });
    }
    context.Narrow(*--lookup);
  }

  for (size_t i = 0; i < tests.GetCount(); ++i) {
    const Test &test = tests[i];

    // Create the without suffix version, add the suffix, and verify it matches.
    size_t prefixLength = test.prefixEnd - (const uint8_t *)result.lookup;
    char *withoutSuffix = Str::DupN(result.lookup, prefixLength);

    char *orthographySuffix = test.suffix->CreateOrthographySuffix();
    char *withSuffix = orthography.AddSuffix(withoutSuffix, orthographySuffix);
    free(orthographySuffix);

    bool isSuffixEqual = Str::Eq(withSuffix, result.lookup);
    free(withSuffix);
    if (!isSuffixEqual) {
      free(withoutSuffix);
      continue;
    }

    StenoReverseDictionaryLookup *prefixLookup =
        new StenoReverseDictionaryLookup(result.strokeThreshold - 1,
                                         withoutSuffix);

    prefixDictionary->ReverseLookup(*prefixLookup);
    free(withoutSuffix);

    bool hasResult = false;
    if (prefixLookup->HasResults()) {
      // Prefix lookup succeeded, get the suffixes.
      StenoReverseDictionaryLookup *suffixLookup =
          new StenoReverseDictionaryLookup(
              result.strokeThreshold - prefixLookup->GetMinimumStrokeCount(),
              (const char *)test.suffix->text);

      // Add map lookup hints.
      suffixLookup->AddMapDataLookup(test.suffix->GetMapDataLookup(),
                                     baseAddress);

      dictionary->ReverseLookup(*suffixLookup);

      if (suffixLookup->HasResults()) {
        for (const StenoReverseDictionaryResult &prefix :
             prefixLookup->results) {
          for (const StenoReverseDictionaryResult &suffix :
               suffixLookup->results) {
            size_t combinedLength = prefix.length + suffix.length;
            if (combinedLength >= result.strokeThreshold) {
              continue;
            }
            StenoStroke strokes[combinedLength];
            memcpy(strokes, prefix.strokes,
                   prefix.length * sizeof(StenoStroke));
            memcpy(strokes + prefix.length, suffix.strokes,
                   suffix.length * sizeof(StenoStroke));

            if (!IsStrokeDefined(strokes, prefix.length, combinedLength)) {
              result.AddResult(strokes, combinedLength, this);
            }
          }
        }
      }
      delete suffixLookup;
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
