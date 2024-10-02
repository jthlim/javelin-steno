//---------------------------------------------------------------------------

#include "script_byte_code.h"
#include "mem.h"
#include "str.h"

//---------------------------------------------------------------------------

const uint8_t *
ScriptByteCode::FindStringOrReturnOriginal(const uint8_t *string) const {
  const StenoScriptHashTable *hashTable = GetHashTable();

  const size_t mask = hashTable->size - 1;
  const size_t length = Str::Length(string);
  size_t index = Crc32(string, length);

  for (;;) {
    const size_t textOffset = hashTable->offsets[index & mask];
    if (textOffset == 0) {
      return string;
    }

    const uint8_t *base = (const uint8_t *)this;
    const uint8_t *candidate = base + textOffset;
    if (Mem::Eq(string, candidate, length + 1)) {
      return candidate;
    }

    ++index;
  }

  return nullptr;
}

//---------------------------------------------------------------------------