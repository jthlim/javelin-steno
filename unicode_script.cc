//---------------------------------------------------------------------------

#include "unicode_script.h"
#include "clock.h"
#include "hal/connection.h"
#include "script_byte_code.h"

//---------------------------------------------------------------------------

UnicodeScript UnicodeScript::instance;

[[gnu::aligned(4)]] const uint8_t UnicodeScript::EMPTY_SCRIPT[] = {
    0x4a, 0x53, 0x53, 0x34, // Magic number: 'JSS4'
    0x00, 0x00,             // String hash table offset
    0x00, 0x00, 0x00, 0x00, // Script offsets (4 x uint16_t)
    0x00, 0x00, 0x00, 0x00,
};

//---------------------------------------------------------------------------

UnicodeScript::UnicodeScript()
    : super(EMPTY_SCRIPT,
            (void (*const *)(Script &, const ScriptByteCode *))FUNCTION_TABLE) {
}

void UnicodeScript::SetScript(const ScriptByteCode *byteCode) {
  super::SetScript(byteCode);
  super::Reset();
  ExecuteInitScript();
}

void UnicodeScript::ExecuteEmitScript(uint32_t unicode) {
  const uintptr_t v = unicode;
  Script::ExecuteScriptIndex(ScriptIndex::EMIT, (intptr_t *)&v, 1);
}

//---------------------------------------------------------------------------

class UnicodeScript::Function {
public:
  static void PressKey(UnicodeScript &script, const ScriptByteCode *byteCode) {
    const uint32_t key = (uint32_t)script.Pop();
    script.context->PressKey(key);
  }

  static void ReleaseKey(UnicodeScript &script,
                         const ScriptByteCode *byteCode) {
    const uint32_t key = (uint32_t)script.Pop();
    script.context->ReleaseKey(key);
  }

  static void TapKey(UnicodeScript &script, const ScriptByteCode *byteCode) {
    const uint32_t key = (uint32_t)script.Pop();
    script.context->TapKey(key);
  }

  static void EmitKeyCode(UnicodeScript &script,
                          const ScriptByteCode *byteCode) {
    const uint32_t keyCode = (uint32_t)script.Pop();
    script.context->EmitKeyCode(keyCode);
  }

  static void Flush(UnicodeScript &script, const ScriptByteCode *byteCode) {
    script.context->Flush();
  }

  static void GetLedStatus(UnicodeScript &script,
                           const ScriptByteCode *byteCode) {
    const int index = (int)script.Pop();
    script.Push(Connection::GetActiveKeyboardLedStatus().GetLedStatus(index));
  }

  static void ReleaseModifiers(UnicodeScript &script,
                               const ScriptByteCode *byteCode) {
    script.context->ReleaseModifiers();
  }

  static void Sleep(UnicodeScript &script, const ScriptByteCode *byteCode) {
    const uint32_t duration = (uint32_t)script.Pop();
    Clock::Sleep(duration);
  }
};

//---------------------------------------------------------------------------

constexpr void (*UnicodeScript::FUNCTION_TABLE[])(UnicodeScript &,
                                                  const ScriptByteCode *) = {
    &Function::PressKey,         //
    &Function::ReleaseKey,       //
    &Function::TapKey,           //
    &Function::EmitKeyCode,      //
    &Function::Flush,            //
    &Function::GetLedStatus,     //
    &Function::ReleaseModifiers, //
    &Function::Sleep,            //
};

//---------------------------------------------------------------------------
