//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>

#include "steno_key_code.h"
#include "steno_key_code_buffer.h"

//---------------------------------------------------------------------------

class StenoKeyCodeEmitter {
public:
  struct EmitterContext;

  // Processes previous and current keycode buffers and determines what
  // differences to emit.
  //
  // Returns true if the last stroke should be combined with previous for undo
  // purposes.
  bool Process(const StenoKeyCode *previous, size_t previousLength,
               const StenoKeyCode *value, size_t valueLength) const;

  bool Process(const StenoKeyCodeBuffer &previous,
               const StenoKeyCodeBuffer &next) const {
    return Process(previous.buffer, previous.count, next.buffer, next.count);
  }

  void Emit(const StenoKeyCode *value, size_t length) const;
  void Emit(const StenoKeyCodeBuffer &buffer) const {
    Emit(buffer.buffer, buffer.count);
  }
};

//---------------------------------------------------------------------------
