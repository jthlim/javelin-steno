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
      : unicode(unicode), outputCaseMode(outputCaseMode),
        selectedCaseMode(selectedCaseMode) {}

  static StenoKeyCode CreateRawKeyCodePress(KeyCode keyCode) {
    StenoKeyCode result;
    result.value = keyCode.value | 0x00c00000; // Set isRawKeyCode and isPress.
    return result;
  }

  static StenoKeyCode CreateRawKeyCodeRelease(KeyCode keyCode) {
    StenoKeyCode result;
    result.value = keyCode.value | 0x00800000; // Set isRawKeyCode
    return result;
  }

  bool IsRawKeyCode() const { return isRawKeyCode; }
  bool IsWhitespace() const {
    // return !isRawKeyCode && Unicode::IsWhitespace(unicode);
    return Unicode::IsWhitespace(unicode);
  }

  bool IsLetter() const {
    // return !isRawKeyCode && Unicode::IsLetter(unicode);
    return Unicode::IsLetter(unicode);
  }

  bool IsAsciiDigit() const {
    // return !isRawKeyCode && Unicode::IsAsciiDigit(unicode);
    return Unicode::IsAsciiDigit(unicode);
  }

  bool IsUnicode(uint32_t value) const {
    // return !isRawKeyCode && unicode == value;
    return unicode == value;
  }

  bool IsPress() const { return isPress; }
  KeyCode::Value GetRawKeyCode() const { return (KeyCode::Value)rawKeyCode; }

  uint32_t GetUnicode() const { return isRawKeyCode ? 0 : unicode; }
  StenoCaseMode GetOutputCaseMode() const {
    // In the case of isRawKeyCode, the associated outputCaseMode bits will
    // be StenoCaseMode::NORMAL.
    // return isRawKeyCode ? StenoCaseMode::NORMAL : outputCaseMode;
    return outputCaseMode;
  }

  void SetCase(StenoCaseMode newCaseMode) {
    if (!isRawKeyCode) {
      outputCaseMode = newCaseMode;
      selectedCaseMode = newCaseMode;
    }
  }

  inline StenoKeyCode WithCase(StenoCaseMode caseMode) const {
    //    return isRawKeyCode ? *this : StenoKeyCode(unicode, caseMode);
    return StenoKeyCode(unicode, caseMode);
  }

  StenoKeyCode ToUpper() const { return WithCase(StenoCaseMode::UPPER); }

  StenoKeyCode ToTitle() const { return WithCase(StenoCaseMode::TITLE); }

  StenoKeyCode ToTitleOnce() const {
    return WithCase(StenoCaseMode::TITLE_ONCE);
  }

  StenoKeyCode ToLower() const { return WithCase(StenoCaseMode::LOWER); }

  // Applies output case mode on unicode and returns the result.
  uint32_t ResolveOutputUnicode() const;
  uint32_t ResolveSelectedUnicode() const;

  bool HasSameOutput(const StenoKeyCode &other) const {
    return value == other.value ||
           ResolveOutputUnicode() == other.ResolveOutputUnicode();
  }

private:
  union {
    struct {
      uint32_t rawKeyCode : 22; // This is KeyCode::Value
      bool isPress : 1;
      bool isRawKeyCode : 1;
      uint32_t reserved : 8;
    };
    struct {
      uint32_t unicode : 24; // This will have the top bit set if it is a
                             // raw key code.

      // StenoCaseMode type.
      // outputCaseMode is used to display.
      // selectedCaseMode is used for reverse lookups.
      StenoCaseMode outputCaseMode : 4;
      StenoCaseMode selectedCaseMode : 4;
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
