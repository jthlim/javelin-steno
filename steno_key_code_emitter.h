//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>

#include "steno_key_code.h"
#include "steno_key_code_buffer.h"

//---------------------------------------------------------------------------

enum class UnicodeMode : uint8_t {
  NONE,
  MACOS_US,
  MACOS_UNICODE_HEX,
  WINDOWS_ALT,
  WINDOWS_HEX,
  LINUX_IBUS,
  COUNT,
};

const char *UnicodeModeName(UnicodeMode mode);

//---------------------------------------------------------------------------

class StenoKeyCodeEmitter {
public:
  struct EmitterContext;

  bool Process(const StenoKeyCode *previous, size_t previousLength,
               const StenoKeyCode *value, size_t valueLength) const;

  bool Process(const StenoKeyCodeBuffer &previous,
               const StenoKeyCodeBuffer &next) const {
    return Process(previous.buffer, previous.count, next.buffer, next.count);
  }

  static const char *GetUnicodeModeName() {
    return UnicodeModeName(emitterMode);
  }

  static bool SetUnicodeMode(const char *name);

  static void SetUnicodeMode(UnicodeMode newMode) { emitterMode = newMode; }

  static const char *const UNICODE_EMITTER_NAMES[];

private:
  static UnicodeMode emitterMode;
};

//---------------------------------------------------------------------------
