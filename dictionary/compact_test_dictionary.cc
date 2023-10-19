// *** Autogenerated file ***

// This is build using the following dictionaries
// * test.json

#include "test_dictionary.h"
#include "dictionary_definition.h"

const uint8_t textBlock[47] = {
  0x00, 0x7b, 0x3a, 0x61, 0x64, 0x64, 0x5f, 0x74, 0x72, 0x61, 0x6e, 0x73, 0x6c, 0x61, 0x74, 0x69,
  0x6f, 0x6e, 0x7d, 0x00, 0x7b, 0x5e, 0x7e, 0x7c, 0x0a, 0x5e, 0x7d, 0x00, 0x7b, 0x5e, 0x69, 0x6e,
  0x67, 0x7d, 0x00, 0x74, 0x65, 0x73, 0x74, 0x65, 0x64, 0x00, 0x74, 0x65, 0x73, 0x74, 0x00,
};
const size_t hashMapSize1 = 128;
const uint8_t data1[24] = {
  0x1c, 0x00, 0x00, 0x00, 0x00, 0x04, 0x2a, 0x00, 0x00, 0x04, 0x28, 0x08, 0x01, 0x00, 0x00, 0x0c,
  0x90, 0x08, 0x14, 0x00, 0x00, 0x80, 0x40, 0x00,
};
const StenoCompactHashMapEntryBlock offsets1[] = {
  { {0x09000000, 0x00000000, 0x10000000, 0x00000080}, 0 },
};

const size_t hashMapSize2 = 128;
const uint8_t data2[9] = {
  0x23, 0x00, 0x00, 0x04, 0x28, 0x08, 0x00, 0x00, 0x20,
};
const StenoCompactHashMapEntryBlock offsets2[] = {
  { {0x00000000, 0x00000080, 0x00000000, 0x00000000}, 0 },
};


const StenoMapDictionaryStrokesDefinition strokes[] = {
  {.hashMapSize = hashMapSize1, .data = data1, .offsets = offsets1},
  {.hashMapSize = hashMapSize2, .data = data2, .offsets = offsets2},
};

constexpr StenoDictionaryDefinition TestDictionary::definition = {
  true,
  2,
  StenoDictionaryType::COMPACT_MAP,
  0,
  "main.json",
  textBlock,
  strokes,
};