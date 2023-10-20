//---------------------------------------------------------------------------

#include "button_manager.h"
#include "clock.h"
#include "console.h"
#include "script_byte_code.h"
#include "timer_manager.h"

//---------------------------------------------------------------------------

#define CONSOLE_LOG_BUTTON_PRESSES 0

//---------------------------------------------------------------------------

JavelinStaticAllocate<ButtonManager> ButtonManager::container;

//---------------------------------------------------------------------------

ButtonManager::ButtonManager(const uint8_t *scriptByteCode)
    : script(scriptByteCode) {
  isScriptValid = ((StenoScriptByteCodeData *)scriptByteCode)->IsValid();
  if (isScriptValid) {
    script.ExecuteInitScript(0);
  }
}

void ButtonManager::Update(const ButtonState &newButtonState,
                           uint32_t scriptTime) {
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
    Console::Printf("Release %zu at %u ms\n\n", buttonIndex, scriptTime);
#endif
    script.HandleRelease(buttonIndex, scriptTime);
  }

  for (size_t buttonIndex : pressedButtons) {
#if CONSOLE_LOG_BUTTON_PRESSES
    Console::Printf("Press %zu at %u ms\n\n", buttonIndex, scriptTime);
#endif
    script.HandlePress(buttonIndex, scriptTime);
  }
}

void ButtonManager::ExecuteScript(ScriptId scriptId) {
  ButtonManager &instance = GetInstance();
  if (instance.isScriptValid) {
    instance.script.ExecuteScriptId(scriptId, Clock::GetMilliseconds());
  }
}

void ButtonManager::PressButton(size_t index, uint32_t scriptTime) {
  if (buttonState.IsSet(index)) {
    return;
  }
  buttonState.Set(index);
  script.HandlePress(index, scriptTime);
}

void ButtonManager::ReleaseButton(size_t index, uint32_t scriptTime) {
  if (!buttonState.IsSet(index)) {
    return;
  }
  buttonState.Clear(index);
  script.HandleRelease(index, scriptTime);
}

//---------------------------------------------------------------------------

void ButtonManager::Tick(uint32_t scriptTime) {
  if (!isScriptValid) {
    return;
  }
  script.ExecuteTickScript(scriptTime);
}

//---------------------------------------------------------------------------

void ButtonManager::EnableScriptEvents_Binding(void *context,
                                               const char *commandLine) {
  ((Script *)context)->EnableScriptEvents();
  Console::SendOk();
}

void ButtonManager::DisableScriptEvents_Binding(void *context,
                                                const char *commandLine) {
  ((Script *)context)->DisableScriptEvents();
  Console::SendOk();
}

void ButtonManager::AddConsoleCommands(Console &console) {
  console.RegisterCommand("enable_script_events", "Enables events from scripts",
                          EnableScriptEvents_Binding, &script);
  console.RegisterCommand("disable_script_events",
                          "Disables events from scripts",
                          DisableScriptEvents_Binding, &script);
}

//---------------------------------------------------------------------------
