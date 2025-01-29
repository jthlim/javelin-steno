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
  HISTORY_MODIFIED,
};

//---------------------------------------------------------------------------

struct StenoState {
  StenoCaseMode caseMode;
  StenoCaseMode overrideCaseMode : 3;
  SegmentLookupType lookupType : 3;
  bool joinNext : 1;
  bool isGlue : 1;
  bool isManualStateChange : 1;
  bool shouldCombineUndo : 1;
  bool isHistoryExtending : 1;       // e.g. fingerspelling
  bool isSpace : 1;                  // {^ ^}
  bool requestsHistoryExtending : 1; // e.g. =set_value, =retro_transform
  bool isSuffix : 1;
  bool isNonAffixCommand : 1;
  bool _unused23 : 1;
  uint32_t spaceLength : 4;
  uint32_t spaceIndex : 4;

  static const StenoCaseMode NEXT_WORD_CASE_MODE[];
  static const StenoCaseMode NEXT_LETTER_CASE_MODE[];

  StenoCaseMode GetNextWordCaseMode() const {
    return NEXT_WORD_CASE_MODE[(int)caseMode];
  }

  void Reset();

  bool IsDefinitionStart() const {
    return lookupType != SegmentLookupType::UNKNOWN;
  }

  void SetSpace(const char *space);

  void CopyTo(StenoState *destination, size_t length) const {
    for (size_t i = 0; i < length; ++i) {
      destination[i] = this[i];
    }
  }

  struct SpaceBuffer {
    static SpaceBuffer instance;

    static const size_t MAXIMUM_COUNT = 32;
    const char *data[MAXIMUM_COUNT];

    size_t GetOrCreateIndex(const char *space);
  };

  bool ShouldStopProcessingLookupType(SegmentLookupType type) const {
    return lookupType != SegmentLookupType::UNKNOWN && lookupType != type;
  }

  const char *GetSpace() const {
    return SpaceBuffer::instance.data[spaceIndex];
  }

  StenoState GetPersistentState() const {
    StenoState result = *this;
    result.shouldCombineUndo = false;
    result.isManualStateChange = false;
    result.isNonAffixCommand = false;
    result.lookupType = SegmentLookupType::UNKNOWN;
    return result;
  }

  const char *GetLookupTypeName() const {
    static constexpr const char *NAMES[] = {
        "UNKNOWN", "DIRECT", "AUTO_SUFFIX", "STROKE", "HISTORY_MODIFIED",
    };
    return NAMES[(int)lookupType];
  }
};

static_assert(sizeof(StenoState) == 4, "Expect StenoState to fit in 4 bytes");

//---------------------------------------------------------------------------

inline StenoCaseMode GetNextLetterCaseMode(StenoCaseMode caseMode) {
  return StenoState::NEXT_LETTER_CASE_MODE[(int)caseMode];
}

//---------------------------------------------------------------------------
