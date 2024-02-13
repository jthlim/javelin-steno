//---------------------------------------------------------------------------

#pragma once
#include "bit_field.h"

//---------------------------------------------------------------------------

#if defined(JAVELIN_BUTTON_STATE_BIT_SIZE)
using ButtonState = BitField<JAVELIN_BUTTON_STATE_BIT_SIZE>;
#else
using ButtonState = BitField<128>;
#endif

struct TimedButtonState {
  uint32_t timestamp;
  ButtonState state;
};

//---------------------------------------------------------------------------
