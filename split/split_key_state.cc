//---------------------------------------------------------------------------

#include "split_key_state.h"
#include "../clock.h"

//---------------------------------------------------------------------------

size_t SplitKeyState::Set(uint32_t timestamp) {
  this->timestamp = timestamp;
  format = Format::NONE;

  return HEADER_SIZE;
}

size_t SplitKeyState::Set(uint32_t timestamp, const ButtonState &state) {
  this->timestamp = timestamp;

  if (state.PopCount() < sizeof(ButtonState)) {
    format = Format::INDEXES;
    size_t count = 0;
    for (const size_t index : state) {
      data[count++] = index;
    }
    return HEADER_SIZE + count;
  }

  format = Format::BITFIELD;
  memcpy(data, &state, sizeof(ButtonState));

  return sizeof(SplitKeyState);
}

uint32_t SplitKeyState::GetTimestamp() const {
  const uint32_t now = Clock::GetMilliseconds();
  const uint32_t recoveredTimestamp = (now & 0xffffc000) | timestamp;

  // Adjust for when the received timestamp was for the previous time quantum.
  // This cannot use 0 as a reference due to potential clock drift between
  // sides.
  return int32_t(recoveredTimestamp - now) > 0x2000
             ? recoveredTimestamp - 0x4000
             : recoveredTimestamp;
}

ButtonState SplitKeyState::GetButtonState(size_t length) const {
  ButtonState result;
  switch (format) {
  case Format::BITFIELD:
    memcpy(&result, data, sizeof(ButtonState));
    break;

  case Format::INDEXES:
  default:
    result.ClearAll();

    for (size_t i = 0; i < length - HEADER_SIZE; ++i) {
      result.Set(data[i]);
    }

    break;
  }
  return result;
}

//---------------------------------------------------------------------------
