//---------------------------------------------------------------------------

#pragma once
#include "key_code.h"
#include "state.h"
#include "unicode.h"

//---------------------------------------------------------------------------

// Represents a value in the steno emit buffer.
class StenoKeyCode {
public:
  StenoKeyCode() = default;

  StenoKeyCode(uint32_t unicode, StenoCaseMode outputCaseMode,
               StenoCaseMode selectedCaseMode = StenoCaseMode::NORMAL)
      : unicode(unicode), outputCaseMode(uint8_t(outputCaseMode)),
        selectedCaseMode(uint8_t(selectedCaseMode)) {}

  static StenoKeyCode CreateRawKeyCodePress(KeyCode keyCode) {
    StenoKeyCode result;
    result.value = keyCode.value | 0xc0000000; // Set isRawKeyCode
    return result;
  }

  static StenoKeyCode CreateRawKeyCodeRelease(KeyCode keyCode) {
    StenoKeyCode result;
    result.value = keyCode.value | 0x80000000; // Set isRawKeyCode
    return result;
  }

  bool IsRawKeyCode() const { return isRawKeyCode; }
  bool IsWhitespace() const {
    return !isRawKeyCode && Unicode::IsWhitespace(unicode);
  }

  bool IsLetter() const { return !isRawKeyCode && Unicode::IsLetter(unicode); }

  bool IsAsciiDigit() const {
    return !isRawKeyCode && '0' <= unicode && unicode <= '9';
  }

  bool IsUnicode(uint32_t value) { return !isRawKeyCode && unicode == value; }

  bool IsPress() const { return isPress; }
  KeyCode::Value GetRawKeyCode() const { return rawKeyCode; }

  uint32_t GetUnicode() const { return isRawKeyCode ? 0 : unicode; }
  StenoCaseMode GetOutputCaseMode() const {
    return isRawKeyCode ? StenoCaseMode::NORMAL : StenoCaseMode(outputCaseMode);
  }

  void SetCase(StenoCaseMode newCaseMode) {
    if (!isRawKeyCode) {
      outputCaseMode = uint8_t(newCaseMode);
      selectedCaseMode = uint8_t(newCaseMode);
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

  // Applies output case mode on unicode and returns the result.
  uint32_t ResolveOutputUnicode() const;
  uint32_t ResolveSelectedUnicode() const;

  bool operator==(const StenoKeyCode &other) const {
    return value == other.value;
  }

private:
  union {
    struct {
      KeyCode::Value rawKeyCode : 8;
      uint32_t reserved : 22;
      bool isPress : 1;
      bool isRawKeyCode : 1;
    };
    struct {
      uint32_t unicode : 24;

      // StenoCaseMode type.
      // outputCaseMode is used to display.
      // selectedCaseMode is used for reverse lookups.
      uint8_t outputCaseMode : 4;
      uint8_t selectedCaseMode : 4;
    };
    uint32_t value;
  };

  static uint32_t ResolveUnicode(uint32_t unicode, StenoCaseMode mode);
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

const uint32_t MODIFIER_SHIFT_FLAG =
    MODIFIER_L_SHIFT_FLAG | MODIFIER_R_SHIFT_FLAG;

//---------------------------------------------------------------------------
