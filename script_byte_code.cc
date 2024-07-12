//---------------------------------------------------------------------------

#include "script_byte_code.h"
#include "crc.h"
#include "mem.h"
#include "str.h"

//---------------------------------------------------------------------------

struct StenoScriptHashTable {
  uint16_t size;
  uint16_t offsets[1];
};

const uint8_t *StenoScriptByteCodeData::FindStringOrReturnOriginal(
    const uint8_t *string) const {
  const uint8_t *base = (const uint8_t *)this;

  const StenoScriptHashTable *hashTable =
      (const StenoScriptHashTable *)(base + stringHashTableOffset);

  const size_t mask = hashTable->size - 1;
  const size_t length = Str::Length(string);
  size_t index = Crc32(string, length);

  for (;;) {
    const size_t textOffset = hashTable->offsets[index & mask];
    if (textOffset == 0) {
      return string;
    }
    const uint8_t *candidate = base + textOffset;
    if (Mem::Eq(string, candidate, length + 1)) {
      return candidate;
    }

    ++index;
  }

  return nullptr;
}

//---------------------------------------------------------------------------
