//---------------------------------------------------------------------------

#include "button_manager.h"
#include "clock.h"
#include "console.h"

//---------------------------------------------------------------------------

#define CONSOLE_LOG_BUTTON_PRESSES 0

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
#if CONSOLE_LOG_BUTTON_PRESSES
      Console::Printf("Press %zu at %u ms\n\n", buttonIndex,
                      Clock::GetCurrentTime());
#endif
      script.HandlePress(buttonIndex);
    } else {
#if CONSOLE_LOG_BUTTON_PRESSES
      Console::Printf("Release %zu at %u ms\n\n", buttonIndex,
                      Clock::GetCurrentTime());
#endif
      script.HandleRelease(buttonIndex);
    }
  }
}

//---------------------------------------------------------------------------
