//---------------------------------------------------------------------------

#pragma once
#include "script.h"
#include "static_allocate.h"

//---------------------------------------------------------------------------

using ButtonState = BitField<128>;

// Handles logical buttons -> key / steno presses.
class ButtonManager {
public:
  static void Initialize(const uint8_t *scriptByteCode) {
    new (container) ButtonManager(scriptByteCode);
  }

  void Update(const ButtonState &newButtonState);
  void Tick();

  void PressButton(size_t index);
  void ReleaseButton(size_t index);

  static ButtonManager &GetInstance() { return container.value; }

private:
  ButtonManager(const uint8_t *scriptByteCode);

  ButtonState buttonState;
  Script script;

  static JavelinStaticAllocate<ButtonManager> container;
};

//---------------------------------------------------------------------------
