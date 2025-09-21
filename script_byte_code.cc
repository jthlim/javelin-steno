//---------------------------------------------------------------------------

#include "script_byte_code.h"
#include "mem.h"
#include "str.h"

//---------------------------------------------------------------------------

const char *
ScriptByteCode::FindStringOrReturnOriginal(const char *string) const {
  const StenoScriptHashTable *hashTable = GetHashTable();

  const size_t mask = hashTable->size - 1;
  const size_t length = Str::Length(string);
  size_t index = Crc32::Hash(string, length);

  for (;;) {
    const size_t textOffset = hashTable->offsets[index & mask];
    if (textOffset == 0) {
      return string;
    }

    const char *base = (const char *)this;
    const char *candidate = base + textOffset;
    if (Mem::Eq(string, candidate, length + 1)) {
      return candidate;
    }

    ++index;
  }

  return nullptr;
}

//---------------------------------------------------------------------------
