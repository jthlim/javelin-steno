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
  void Tick();

  void PressButton(size_t index);
  void ReleaseButton(size_t index);

private:
  ButtonState buttonState;
  Script script;
};

//---------------------------------------------------------------------------
