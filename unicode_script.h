//---------------------------------------------------------------------------

#pragma once
#include "key.h"
#include "script.h"
#include "steno_key_code_emitter_context.h"

//---------------------------------------------------------------------------

class UnicodeScript : public Script {
private:
  using super = Script;

public:
  UnicodeScript();

  enum ScriptIndex {
    INIT,
    BEGIN,
    EMIT,
    END,
  };

  void SetScript(const ScriptByteCode *byteCode);
  void ExecuteInitScript() {
    if (IsValid()) {
      ExecuteScriptIndex(ScriptIndex::INIT);
    }
  }
  void ExecuteBeginScript() { ExecuteScriptIndex(ScriptIndex::BEGIN); }
  void ExecuteEmitScript(uint32_t unicode);
  void ExecuteEndScript() { ExecuteScriptIndex(ScriptIndex::END); }

  void SetContext(StenoKeyCodeEmitter::EmitterContext *c) { context = c; }

  static UnicodeScript instance;

  static const uint8_t EMPTY_SCRIPT[];

private:
  class Function;

  StenoKeyCodeEmitter::EmitterContext *context;

  static void (*const FUNCTION_TABLE[])(UnicodeScript &,
                                        const ScriptByteCode *);
};

//---------------------------------------------------------------------------
