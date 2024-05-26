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
  caseMode = StenoCaseMode::NORMAL;
  overrideCaseMode = StenoCaseMode::NORMAL;
  joinNext = false;
  isGlue = false;
  isManualStateChange = false;
  isDefinitionStart = 1;
  lookupType = SegmentLookupType::DIRECT;
  spaceOffset = 0;
  spaceLength = 1;
}

void StenoState::SetSpace(const char *space) {
  size_t length = Str::Length(space);
  if (length >= 8) {
    spaceOffset = 0;
    spaceLength = 0;
    return;
  }

  // See if it already exists in the buffer.
  SpaceBuffer &buffer = SpaceBuffer::instance;
  for (size_t i = 0; i + length <= buffer.count; ++i) {
    if (Mem::Eq(buffer.data + i, space, length)) {
      spaceLength = length;
      spaceOffset = i;
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
  spaceLength = length;
  memcpy(buffer.data + buffer.count, space, length);
  buffer.count += length;
}

//---------------------------------------------------------------------------
