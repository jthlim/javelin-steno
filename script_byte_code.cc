//---------------------------------------------------------------------------

#include "script_byte_code.h"
#include "console.h"
#include "crc.h"
#include "hal/external_flash.h"
#include "mem.h"
#include "script_manager.h"
#include "str.h"

#include JAVELIN_BOARD_CONFIG

//---------------------------------------------------------------------------

const uint8_t *StenoScriptByteCodeData::FindStringOrReturnOriginal(
    const uint8_t *string) const {
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

void JavelinLayoutData::GetJavelinLayoutParameter() {
#if JAVELIN_USE_LAYOUT_STORAGE
  const ExternalFlashSentry sentry;
  uint32_t scriptCrc = ScriptManager::GetInstance().Crc();
  if (scriptCrc != JAVELIN_LAYOUT_STORAGE_ADDRESS->expectedScriptCrc) {
    Console::Printf("ERR No matching layout\n\n");
    return;
  }

  Console::Printf("\"%D\"\n\n", JAVELIN_LAYOUT_STORAGE_ADDRESS->data,
                  JAVELIN_LAYOUT_STORAGE_ADDRESS->length);
#else
  Console::Printf("ERR No layout storage\n\n");
#endif
}

//---------------------------------------------------------------------------