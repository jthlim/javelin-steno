//---------------------------------------------------------------------------

#include "reverse_map_dictionary.h"
#include "../str.h"

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoReverseMapDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  return dictionary->Lookup(lookup);
}

void StenoReverseMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {

  dictionary->ReverseLookup(result);

  const uint8_t *left = textBlock + 1;
  const uint8_t *right = textBlock + textBlockLength;

  while (left < right) {
    const uint8_t *mid = left + (right - left) / 2;

    const uint8_t *wordStart = mid;
    while (wordStart[-1] != 0xff) {
      --wordStart;
    }

    int compare = Compare(result.lookup, wordStart);
    if (compare < 0) {
      right = wordStart;
    } else if (compare > 0) {
      while (*wordStart != 0xff) {
        ++wordStart;
      }
      left = wordStart + 1;
    } else {
      const uint8_t *p = wordStart;
      while (*p != 0) {
        ++p;
      }
      ++p;
      while (*p != 0xff) {
        uint32_t offset = p[0] | (p[1] << 7) | (p[2] << 14) + (p[3] << 21);
        ReverseMapDictionaryLookup(result, baseAddress + offset);
        p += 4;
      }

      return;
    }
  }
}

bool StenoReverseMapDictionary::ReverseMapDictionaryLookup(
    StenoReverseDictionaryLookup &result, const void *data) const {
  return dictionary->ReverseMapDictionaryLookup(result, data);
}

unsigned int StenoReverseMapDictionary::GetMaximumMatchLength() const {
  return dictionary->GetMaximumMatchLength();
}

const char *StenoReverseMapDictionary::GetName() const {
  return "#internal#reverse_map_dictionary";
}

void StenoReverseMapDictionary::PrintInfo(int depth) const {
  dictionary->PrintInfo(depth);
}

bool StenoReverseMapDictionary::PrintDictionary(bool hasData) const {
  return dictionary->PrintDictionary(hasData);
}

void StenoReverseMapDictionary::ListDictionaries() const {
  dictionary->ListDictionaries();
}

bool StenoReverseMapDictionary::EnableDictionary(const char *name) {
  return dictionary->EnableDictionary(name);
}

bool StenoReverseMapDictionary::DisableDictionary(const char *name) {
  return dictionary->DisableDictionary(name);
}

bool StenoReverseMapDictionary::ToggleDictionary(const char *name) {
  return dictionary->ToggleDictionary(name);
}

//---------------------------------------------------------------------------

// strcmp, with special consideration for {.}, {,} and {!}
int StenoReverseMapDictionary::Compare(const char *lookup,
                                       const uint8_t *textBlock) {
  const char *startLookup = lookup;
  const char *startTextBlock = (const char *)textBlock;

  // TODO.

  // for (;;) {
  //   int a = *(uint8_t *)lookup++;
  //   int b = *textBlock++;
  //   if (a == b) {
  //     if (a != '\0') {
  //       continue;
  //     }
  //     return 0;
  //   }

  //   if (b == '{') {
  //     int c = *textBlock++;
  //     switch (c) {
  //     case '.':
  //     case ',':
  //     case '!':
  //     case '?':
  //       if (a == c && *textBlock == '}') {
  //         ++textBlock;
  //         if (*lookup == ' ') {
  //           ++lookup;
  //         }
  //         if (*textBlock == ' ') {
  //           ++textBlock;
  //         }
  //         continue;
  //       }
  //     }
  //   }

  //   // In this situation, the comparison needs to go back to the start
  //   // for binary search to work properly.
  //   return strcmp(startLookup, startTextBlock);
  // }
  return strcmp(startLookup, startTextBlock);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// #include "../unit_test.h"

// TEST_BEGIN("Compare matches words with {,}") {
//   assert(StenoReverseMapDictionary::Compare("door,",
//                                             (const uint8_t *)"door{,}") ==
//                                             0);
// }
// TEST_END

//---------------------------------------------------------------------------
