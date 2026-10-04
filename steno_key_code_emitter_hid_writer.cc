//---------------------------------------------------------------------------

#include "steno_key_code_emitter_hid_writer.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

void StenoKeyCodeEmitter::HidWriter::Write(const char *data, size_t length) {
  Utf8Pointer utf8(data);
  const char *end = data + length;

  while (utf8 < end) {
    const uint32_t unicode = *utf8++;

    const StenoKeyCode stenoKeyCode(unicode, StenoCaseMode::NORMAL);
    context.ProcessStenoKeyCode(stenoKeyCode);
  }
}

//---------------------------------------------------------------------------
