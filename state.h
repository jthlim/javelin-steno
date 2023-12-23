//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

enum class StenoCaseMode : uint8_t {
  NORMAL,
  LOWER,
  UPPER,
  TITLE,
  LOWER_ONCE,
  UPPER_ONCE,
  TITLE_ONCE,
  UNSPECIFIED,
};

//---------------------------------------------------------------------------

struct StenoState {
  StenoCaseMode caseMode;
  StenoCaseMode overrideCaseMode;
  bool joinNext : 1;
  bool isGlue : 1;
  bool isManualStateChange : 1;
  bool shouldCombineUndo : 1;
  bool isDefinitionStart : 1;
  uint8_t spaceCharacterLength;
  const char *spaceCharacter;

  static const StenoCaseMode NEXT_WORD_CASE_MODE[];
  static const StenoCaseMode NEXT_LETTER_CASE_MODE[];

  StenoCaseMode GetNextWordCaseMode() const {
    return NEXT_WORD_CASE_MODE[(int)caseMode];
  }

  void Reset();
};

//---------------------------------------------------------------------------

inline StenoCaseMode GetNextLetterCaseMode(StenoCaseMode caseMode) {
  return StenoState::NEXT_LETTER_CASE_MODE[(int)caseMode];
}

//---------------------------------------------------------------------------
