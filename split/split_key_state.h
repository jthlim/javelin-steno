//---------------------------------------------------------------------------

#pragma once
#include "../button_state.h"

//---------------------------------------------------------------------------

struct SplitKeyState {
  // Returns transmission size required in bytes;
  size_t Set(uint32_t timestamp);
  size_t Set(uint32_t timestamp, const ButtonState &state);

  bool HasButtonState() const { return format != Format::NONE; }
  uint32_t GetTimestamp() const;
  ButtonState GetButtonState(size_t length) const;

private:
  enum class Format : uint8_t {
    NONE,
    INDEXES,
    BITFIELD,
  };

  uint16_t timestamp : 14;
  Format format : 2;
  uint8_t data[sizeof(ButtonState)];

  static constexpr size_t HEADER_SIZE = 2;
};

//---------------------------------------------------------------------------
