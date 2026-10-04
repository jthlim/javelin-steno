//---------------------------------------------------------------------------

#pragma once
#include "steno_key_code_emitter_context.h"
#include "writer.h"

//---------------------------------------------------------------------------

class StenoKeyCodeEmitter::HidWriter final : public IWriter {
public:
  HidWriter() {}
  ~HidWriter() { context.ReleaseModifiers(context.modifiers); }

  virtual void Write(const char *data, size_t length);

  StenoKeyCodeEmitter::EmitterContext context;
};

//---------------------------------------------------------------------------
