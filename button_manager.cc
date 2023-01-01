//---------------------------------------------------------------------------

#include "button_manager.h"

//---------------------------------------------------------------------------

ButtonManager::ButtonManager(const uint8_t *scriptByteCode)
    : script(scriptByteCode) {
  buttonState.ClearAll();
  script.ExecuteInitScript();
}

void ButtonManager::Update(const ButtonState &newButtonState) {
  if (newButtonState == buttonState) {
    return;
  }

  ButtonState changedButtons = buttonState ^ newButtonState;
  buttonState = newButtonState;
  for (size_t buttonIndex : changedButtons) {
    if (buttonState.IsSet(buttonIndex)) {
      script.HandlePress(buttonIndex);
    } else {
      script.HandleRelease(buttonIndex);
    }
  }
}

//---------------------------------------------------------------------------
