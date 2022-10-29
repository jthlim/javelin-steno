//---------------------------------------------------------------------------

#pragma once
#include "state.h"
#include <stdint.h>

//---------------------------------------------------------------------------

uint32_t ToUpper(uint32_t c);
uint32_t ToLower(uint32_t c);
bool IsAsciiDigit(uint32_t c);
bool IsLetter(uint32_t c);
bool IsWhitespace(uint32_t c);
bool IsWordCharacter(uint32_t c);

//---------------------------------------------------------------------------

// Represents a value in the steno emit buffer.
class StenoKeyCode {
public:
  StenoKeyCode() = default;

  StenoKeyCode(uint32_t unicode, StenoCaseMode caseMode)
      : unicode(unicode), caseMode(caseMode) {}

  static StenoKeyCode CreateRawKeyCodePress(uint32_t v) {
    StenoKeyCode result;
    result.value = v | 0xc0000000; // Set isRawKeyCode
    return result;
  }

  static StenoKeyCode CreateRawKeyCodeRelease(uint32_t v) {
    StenoKeyCode result;
    result.value = v | 0x80000000; // Set isRawKeyCode
    return result;
  }

  bool IsRawKeyCode() const { return isRawKeyCode; }
  bool IsWhitespace() const { return !isRawKeyCode && ::IsWhitespace(unicode); }

  bool IsLetter() const { return !isRawKeyCode && ::IsLetter(unicode); }

  bool IsPress() const { return isPress; }
  uint32_t GetRawKeyCode() { return rawKeyCode; }

  uint32_t GetUnicode() const { return isRawKeyCode ? 0 : unicode; }
  StenoCaseMode GetCaseMode() const {
    return isRawKeyCode ? StenoCaseMode::NORMAL : caseMode;
  }

  void SetCase(StenoCaseMode newCaseMode) {
    if (!isRawKeyCode) {
      caseMode = newCaseMode;
    }
  }

  StenoKeyCode WithCase(StenoCaseMode caseMode) const {
    return isRawKeyCode ? *this : StenoKeyCode(unicode, caseMode);
  }

  StenoKeyCode ToUpper() const {
    return isRawKeyCode ? *this : StenoKeyCode(unicode, StenoCaseMode::UPPER);
  }

  StenoKeyCode ToTitle() const {
    return isRawKeyCode ? *this : StenoKeyCode(unicode, StenoCaseMode::TITLE);
  }

  StenoKeyCode ToTitleOnce() const {
    return isRawKeyCode ? *this
                        : StenoKeyCode(unicode, StenoCaseMode::TITLE_ONCE);
  }

  StenoKeyCode ToLower() const {
    return isRawKeyCode ? *this : StenoKeyCode(unicode, StenoCaseMode::LOWER);
  }

  // Applies case mode on unicode and returns the result.
  uint32_t ResolveUnicode() const;

  bool operator==(const StenoKeyCode &other) const {
    return value == other.value;
  }

private:
  union {
    struct {
      uint32_t rawKeyCode : 8;
      uint32_t reserved : 22;
      bool isPress : 1;
      bool isRawKeyCode : 1;
    };
    struct {
      uint32_t unicode : 24;
      StenoCaseMode caseMode : 8;
    };
    uint32_t value;
  };
};

static_assert(sizeof(StenoKeyCode) == 4);

//---------------------------------------------------------------------------

const uint32_t MODIFIER_MASK = 0xff00;
const uint32_t MODIFIER_BIT_SHIFT = 8;

const uint32_t MODIFIER_L_CTRL_FLAG = 0x100;
const uint32_t MODIFIER_L_SHIFT_FLAG = 0x200;
const uint32_t MODIFIER_L_ALT_FLAG = 0x400;
const uint32_t MODIFIER_L_SUPER_FLAG = 0x800;
const uint32_t MODIFIER_R_CTRL_FLAG = 0x1000;
const uint32_t MODIFIER_R_SHIFT_FLAG = 0x2000;
const uint32_t MODIFIER_R_ALT_FLAG = 0x4000;
const uint32_t MODIFIER_R_SUPER_FLAG = 0x8000;

//---------------------------------------------------------------------------
