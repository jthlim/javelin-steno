//---------------------------------------------------------------------------

#pragma once
#include "script.h"
#include "static_allocate.h"

//---------------------------------------------------------------------------

#if defined(JAVELIN_BUTTON_STATE_BIT_SIZE)
using ButtonState = BitField<JAVELIN_BUTTON_STATE_BIT_SIZE>;
#else
using ButtonState = BitField<128>;
#endif

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
