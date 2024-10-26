//---------------------------------------------------------------------------

#include "button_script_manager.h"
#include "clock.h"
#include "console.h"
#include "flash.h"
#include "timer_manager.h"

//---------------------------------------------------------------------------

#define CONSOLE_LOG_BUTTON_PRESSES 0
#define ENABLE_PRINT_SCRIPT_GLOBALS_CONSOLE_COMMAND 0

//---------------------------------------------------------------------------

JavelinStaticAllocate<ButtonScriptManager> ButtonScriptManager::container;

//---------------------------------------------------------------------------

ButtonScriptManager::ButtonScriptManager(const uint8_t *scriptByteCode)
    : script(scriptByteCode) {
  isScriptValid = script.IsValid();
  if (isScriptValid) {
    script.ExecuteInitScript(Clock::GetMilliseconds());
  }
}

void ButtonScriptManager::Update(const ButtonState &newButtonState,
                                 uint32_t scriptTime) {
  if (!isScriptValid) {
    return;
  }

  if (newButtonState == buttonState) {
    return;
  }

  const ButtonState pressedButtons = newButtonState & ~buttonState;
  const ButtonState releasedButtons = buttonState & ~newButtonState;

  buttonState = newButtonState;

  for (const size_t buttonIndex : releasedButtons) {
#if CONSOLE_LOG_BUTTON_PRESSES
    Console::Printf("Release %zu at %u ms, now: %u ms\n\n", buttonIndex,
                    scriptTime, Clock::GetMilliseconds());
#endif
    script.IncrementReleaseCount();
    script.HandleRelease(buttonIndex, scriptTime);
  }

  for (const size_t buttonIndex : pressedButtons) {
#if CONSOLE_LOG_BUTTON_PRESSES
    Console::Printf("Press %zu at %u ms, now: %u ms\n\n", buttonIndex,
                    scriptTime, Clock::GetMilliseconds());
#endif
    script.IncrementPressCount();
    script.HandlePress(buttonIndex, scriptTime);
  }
  SendButtonStateUpdate();
}

void ButtonScriptManager::ExecuteScript(ButtonScriptId scriptId) {
  if (Flash::IsUpdating()) {
    return;
  }
  ButtonScriptManager &instance = GetInstance();
  if (!instance.isScriptValid) {
    return;
  }

  instance.script.ExecuteScriptId(scriptId, Clock::GetMilliseconds());
}

void ButtonScriptManager::PressButton(size_t index, uint32_t scriptTime) {
  if (buttonState.IsSet(index)) {
    return;
  }
  buttonState.Set(index);
  script.HandlePress(index, scriptTime);
  SendButtonStateUpdate();
}

void ButtonScriptManager::ReleaseButton(size_t index, uint32_t scriptTime) {
  if (!buttonState.IsSet(index)) {
    return;
  }
  buttonState.Clear(index);
  script.HandleRelease(index, scriptTime);
  SendButtonStateUpdate();
}

void ButtonScriptManager::SendButtonStateUpdate(
    const ButtonState &state) const {
  if (isButtonStateUpdatesEnabled) {
    Console::Printf("EV {\"event\":\"button_state\",\"data\":\"%D\"}\n\n",
                    &state, sizeof(state));
  }
}

void ButtonScriptManager::SendButtonStateUpdate() const {
  if (allowButtonStateUpdates) {
    SendButtonStateUpdate(buttonState);
  }
}

void ButtonScriptManager::SetAllowButtonStateUpdates(bool value) {
  if (value == allowButtonStateUpdates) {
    return;
  }
  allowButtonStateUpdates = value;
  if (value) {
    SendButtonStateUpdate();
  } else {
    ButtonState state;
    state.ClearAll();
    SendButtonStateUpdate(state);
  }
}

//---------------------------------------------------------------------------

void ButtonScriptManager::Tick(uint32_t scriptTime) {
  if (Flash::IsUpdating()) {
    return;
  }
  if (!isScriptValid) {
    return;
  }
  script.ExecuteTickScript(scriptTime);
}

//---------------------------------------------------------------------------

void ButtonScriptManager::Reset() {
  script.Reset();
  TimerManager::instance.RemoveScriptTimers(Clock::GetMilliseconds());

  isScriptValid = script.IsValid();
  if (isScriptValid) {
    script.ExecuteInitScript(Clock::GetMilliseconds());
  }
}

//---------------------------------------------------------------------------

void ButtonScriptManager::EnableScriptEvents_Binding(void *context,
                                                     const char *commandLine) {
  ((ButtonScript *)context)->EnableScriptEvents();
  Console::SendOk();
}

void ButtonScriptManager::DisableScriptEvents_Binding(void *context,
                                                      const char *commandLine) {
  ((ButtonScript *)context)->DisableScriptEvents();
  Console::SendOk();
}

void ButtonScriptManager::EnableButtonStateUpdates_Binding(
    void *context, const char *commandLine) {
  ButtonScriptManager *manager = (ButtonScriptManager *)context;
  manager->isButtonStateUpdatesEnabled = true;
  Console::SendOk();
}

void ButtonScriptManager::DisableButtonStateUpdates_Binding(
    void *context, const char *commandLine) {
  ((ButtonScriptManager *)context)->isButtonStateUpdatesEnabled = false;
  Console::SendOk();
}

void ButtonScriptManager::PrintScriptGlobals_Binding(void *context,
                                                     const char *commandLine) {
  ((ButtonScriptManager *)context)->script.PrintScriptGlobals();
}

void ButtonScriptManager::AddConsoleCommands(Console &console) {
  console.RegisterCommand("enable_script_events", "Enables events from scripts",
                          EnableScriptEvents_Binding, &script);
  console.RegisterCommand("disable_script_events",
                          "Disables events from scripts",
                          DisableScriptEvents_Binding, &script);
  console.RegisterCommand("enable_button_state_updates",
                          "Enables button state updates",
                          EnableButtonStateUpdates_Binding, this);
  console.RegisterCommand("disable_button_state_updates",
                          "Disables button state updates",
                          DisableButtonStateUpdates_Binding, this);
#if ENABLE_PRINT_SCRIPT_GLOBALS_CONSOLE_COMMAND
  console.RegisterCommand("print_script_globals",
                          "Prints non-zero script globals",
                          PrintScriptGlobals_Binding, this);
#endif
}

//---------------------------------------------------------------------------
