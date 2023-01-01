//---------------------------------------------------------------------------

#pragma once
#include "script.h"

//---------------------------------------------------------------------------

using ButtonState = BitField<128>;

// Handles logical buttons -> key / steno presses.
class ButtonManager {
public:
  ButtonManager(const uint8_t *scriptByteCode);

  void Update(const ButtonState &newButtonState);

private:
  ButtonState buttonState;
  Script script;
};

//---------------------------------------------------------------------------
