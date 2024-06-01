//---------------------------------------------------------------------------

#pragma once
#include "mem.h"
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

enum class SegmentLookupType : uint8_t {
  UNKNOWN,
  DIRECT,
  AUTO_SUFFIX,
  STROKE,
};

//---------------------------------------------------------------------------

struct StenoState {
  StenoCaseMode caseMode;
  StenoCaseMode overrideCaseMode;
  SegmentLookupType lookupType : 2;
  bool joinNext : 1;
  bool isGlue : 1;
  bool isManualStateChange : 1;
  bool shouldCombineUndo : 1;
  uint8_t _reserved : 2;
  uint8_t spaceLength : 3;
  uint8_t spaceOffset : 5;

  static const StenoCaseMode NEXT_WORD_CASE_MODE[];
  static const StenoCaseMode NEXT_LETTER_CASE_MODE[];

  StenoCaseMode GetNextWordCaseMode() const {
    return NEXT_WORD_CASE_MODE[(int)caseMode];
  }

  void Reset();

  bool IsDefinitionStart() const {
    return lookupType != SegmentLookupType::UNKNOWN;
  }

  bool operator==(const StenoState &a) const {
    return memcmp(this, &a, sizeof(*this)) == 0;
  }

  void SetSpace(const char *space);

  void CopyTo(StenoState *destination, size_t length) const {
    for (size_t i = 0; i < length; ++i) {
      destination[i] = this[i];
    }
  }

  struct SpaceBuffer {
    static SpaceBuffer instance;

    uint8_t count;
    char data[32];
  };

  const char *GetSpace() const {
    return SpaceBuffer::instance.data + spaceOffset;
  }
};

static_assert(sizeof(StenoState) == 4, "Expect StenoState to fit in 4 bytes");

//---------------------------------------------------------------------------

inline StenoCaseMode GetNextLetterCaseMode(StenoCaseMode caseMode) {
  return StenoState::NEXT_LETTER_CASE_MODE[(int)caseMode];
}

//---------------------------------------------------------------------------
