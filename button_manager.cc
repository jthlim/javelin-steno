//---------------------------------------------------------------------------

#include "button_manager.h"
#include "clock.h"
#include "console.h"
#include "script_byte_code.h"

//---------------------------------------------------------------------------

#define CONSOLE_LOG_BUTTON_PRESSES 0

//---------------------------------------------------------------------------

JavelinStaticAllocate<ButtonManager> ButtonManager::container;

//---------------------------------------------------------------------------

ButtonManager::ButtonManager(const uint8_t *scriptByteCode)
    : script(scriptByteCode) {
  isScriptValid = *(uint32_t *)scriptByteCode == SCRIPT_MAGIC;
  if (isScriptValid) {
    script.ExecuteInitScript();
  }
}

void ButtonManager::Update(const ButtonState &newButtonState) {
  if (!isScriptValid) {
    return;
  }

  if (newButtonState == buttonState) {
    return;
  }

  const ButtonState changedButtons = buttonState ^ newButtonState;
  const ButtonState pressedButtons = changedButtons & newButtonState;
  const ButtonState releasedButtons = changedButtons ^ pressedButtons;

  buttonState = newButtonState;

  for (size_t buttonIndex : releasedButtons) {
#if CONSOLE_LOG_BUTTON_PRESSES
    Console::Printf("Release %zu at %u ms\n\n", buttonIndex,
                    Clock::GetMilliseconds());
#endif
    script.HandleRelease(buttonIndex);
  }

  for (size_t buttonIndex : pressedButtons) {
#if CONSOLE_LOG_BUTTON_PRESSES
    Console::Printf("Press %zu at %u ms\n\n", buttonIndex,
                    Clock::GetMilliseconds());
#endif
    script.HandlePress(buttonIndex);
  }
}

void ButtonManager::PressButton(size_t index) {
  if (buttonState.IsSet(index)) {
    return;
  }
  buttonState.Set(index);
  script.HandlePress(index);
}

void ButtonManager::ReleaseButton(size_t index) {
  if (!buttonState.IsSet(index)) {
    return;
  }
  buttonState.Clear(index);
  script.HandleRelease(index);
}

//---------------------------------------------------------------------------

void ButtonManager::Tick() {
  if (isScriptValid) {
    script.ExecuteTickScript();
  }
}

//---------------------------------------------------------------------------
