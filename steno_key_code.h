//---------------------------------------------------------------------------

#pragma once
#include "key_code.h"
#include "state.h"
#include "unicode.h"

//---------------------------------------------------------------------------

// Represents a value in the steno emit buffer.
//
// There are two types of StenoKeyCodes - Unicode with case, and raw key codes.
// The representation of these are carefully designed to avoid specific checks
// against isRawKeyCode in many of the functions, as the raw code flags overlap
// with the unicode bitfield, and will just do the right thing.
//
// The functions that work in either mode have a comment: // *
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

  bool IsRawKeyCode() const { return isRawKeyCode; } // *

  // Only for raw key codes.
  bool IsPress() const { return isPress; }

  // Only for raw key codes.
  KeyCode::Value GetRawKeyCode() const { return (KeyCode::Value)rawKeyCode; }

  bool IsWhitespace() const { return Unicode::IsWhitespace(unicode); } // *
  bool IsLetter() const { return Unicode::IsLetter(unicode); }         // *
  bool IsAsciiDigit() const { return Unicode::IsAsciiDigit(unicode); } // *
  bool IsUnicode(uint32_t value) const { return unicode == value; }    // *
  bool IsSpace() const { return unicode == ' '; }                      // *

  uint32_t GetUnicode() const { return isRawKeyCode ? 0 : unicode; }

  // Returns the output case mode for unicode, and NORMAL for raw key codes.
  StenoCaseMode GetOutputCaseMode() const { return outputCaseMode; } // *

  void SetCase(StenoCaseMode newCaseMode) { // *
    if (!isRawKeyCode) {
      outputCaseMode = newCaseMode;
      selectedCaseMode = newCaseMode;
    }
  }

  inline StenoKeyCode WithCase(StenoCaseMode caseMode) const { // *
    return StenoKeyCode(unicode, caseMode);
  }

  StenoKeyCode ToUpper() const { return WithCase(StenoCaseMode::UPPER); } // *
  StenoKeyCode ToLower() const { return WithCase(StenoCaseMode::LOWER); } // *
  StenoKeyCode ToTitle() const { return WithCase(StenoCaseMode::TITLE); } // *
  StenoKeyCode ToTitleOnce() const {                                      // *
    return WithCase(StenoCaseMode::TITLE_ONCE);
  }

  // Applies output case mode on unicode and returns the result.
  uint32_t ResolveOutputUnicode() const {
    return ResolveUnicode(unicode, outputCaseMode);
  }
  uint32_t ResolveSelectedUnicode() const {
    return ResolveUnicode(unicode, selectedCaseMode);
  }

  bool HasSameOutput(const StenoKeyCode &other) const { // *
    return value == other.value ||
           ResolveOutputUnicode() == other.ResolveOutputUnicode();
  }

  bool operator==(const StenoKeyCode &other) const {
    return value == other.value;
  }

private:
  union {
    struct {
      // This is KeyCode::Value, but using a using a uint32_t to use more than
      // 8 bits.
      uint32_t rawKeyCode : 22;
      bool isPress : 1;
      bool isRawKeyCode : 1;
      uint32_t reserved : 8;
    };
    struct {
      // Unicode only needs 21 bits.
      // The upper bits intentionally overlap with the flags for raw key codes
      // to simplify the implementation of the above methods.
      uint32_t unicode : 24;

      // Used for output case.
      StenoCaseMode outputCaseMode : 4;

      // Used for reverse lookups.
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
