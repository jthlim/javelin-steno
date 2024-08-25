//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>

#include "steno_key_code.h"
#include "steno_key_code_buffer.h"

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
};

//---------------------------------------------------------------------------
