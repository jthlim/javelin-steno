//---------------------------------------------------------------------------

#include "state.h"
#include "str.h"

//---------------------------------------------------------------------------

constexpr StenoCaseMode StenoState::NEXT_WORD_CASE_MODE[] = {
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::LOWER,       //
    StenoCaseMode::UPPER,       //
    StenoCaseMode::TITLE,       //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::UNSPECIFIED, //
};

constexpr StenoCaseMode StenoState::NEXT_LETTER_CASE_MODE[] = {
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::LOWER,       //
    StenoCaseMode::UPPER,       //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::LOWER_ONCE,  //
    StenoCaseMode::UPPER_ONCE,  //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::UNSPECIFIED, //
};

//---------------------------------------------------------------------------

StenoState::SpaceBuffer StenoState::SpaceBuffer::instance = {
    .data = {" "},
};

size_t StenoState::SpaceBuffer::GetOrCreateIndex(const char *space) {
  for (size_t i = 0; i < MAXIMUM_COUNT; ++i) {
    const char *s = data[i];
    if (s == nullptr) {
      data[i] = Str::Dup(space);
      return i;
    }
    if (Str::Eq(space, data[i])) {
      return i;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------

void StenoState::Reset() {
  *this = StenoState{
      .caseMode = StenoCaseMode::NORMAL,
      .overrideCaseMode = StenoCaseMode::NORMAL,
      .lookupType = SegmentLookupType::UNKNOWN,
      .joinNext = false,
      .isGlue = false,
      .isManualStateChange = false,
      .shouldCombineUndo = false,
      .spaceLength = 1,
      .spaceIndex = 0,
  };
}

void StenoState::SetSpace(const char *space) {
  spaceIndex = (uint32_t) SpaceBuffer::instance.GetOrCreateIndex(space);
  spaceLength = (uint32_t) Str::Length(GetSpace());
}

//---------------------------------------------------------------------------
