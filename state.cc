//---------------------------------------------------------------------------

#include "state.h"
#include "str.h"

//---------------------------------------------------------------------------

const StenoCaseMode StenoState::NEXT_WORD_CASE_MODE[] = {
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::LOWER,       //
    StenoCaseMode::UPPER,       //
    StenoCaseMode::TITLE,       //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::NORMAL,      //
    StenoCaseMode::UNSPECIFIED, //
};

const StenoCaseMode StenoState::NEXT_LETTER_CASE_MODE[] = {
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
    .count = 1,
    .data = {' '},
};

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
      .spaceOffset = 0,
  };
}

void StenoState::SetSpace(const char *space) {
  const size_t length = Str::Length(space);
  if (length >= 16) {
    spaceOffset = 0;
    spaceLength = 0;
    return;
  }

  // See if it already exists in the buffer.
  SpaceBuffer &buffer = SpaceBuffer::instance;
  for (size_t i = 0; i + length <= buffer.count; ++i) {
    if (Mem::Eq(buffer.data + i, space, length)) {
      spaceLength = (uint32_t)length;
      spaceOffset = (uint32_t)i;
      return;
    }
  }

  // Add it if possible otherwise.

  if (buffer.count + length >= sizeof(buffer.data)) {
    spaceOffset = 0;
    spaceLength = 0;
    return;
  }

  spaceOffset = buffer.count;
  spaceLength = (uint32_t)length;
  memcpy(buffer.data + buffer.count, space, length);
  buffer.count += length;
}

//---------------------------------------------------------------------------
