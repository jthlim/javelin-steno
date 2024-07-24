//---------------------------------------------------------------------------

#include "script_manager.h"
#include "clock.h"
#include "console.h"
#include "flash.h"
#include "script_byte_code.h"
#include "timer_manager.h"

//---------------------------------------------------------------------------

#define CONSOLE_LOG_BUTTON_PRESSES 0
#define ENABLE_BUTTON_STATE_UPDATES 0

//---------------------------------------------------------------------------

JavelinStaticAllocate<ScriptManager> ScriptManager::container;

//---------------------------------------------------------------------------

ScriptManager::ScriptManager(const uint8_t *scriptByteCode)
    : script(scriptByteCode) {
  isScriptValid = script.IsValid();
  if (isScriptValid) {
    script.ExecuteInitScript(Clock::GetMilliseconds());
  }
}

void ScriptManager::Update(const ButtonState &newButtonState,
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

void ScriptManager::ExecuteScript(ScriptId scriptId) {
  if (Flash::IsUpdating()) {
    return;
  }
  ScriptManager &instance = GetInstance();
  if (!instance.isScriptValid) {
    return;
  }

  instance.script.ExecuteScriptId(scriptId, Clock::GetMilliseconds());
}

void ScriptManager::PressButton(size_t index, uint32_t scriptTime) {
  if (buttonState.IsSet(index)) {
    return;
  }
  buttonState.Set(index);
  script.HandlePress(index, scriptTime);
  SendButtonStateUpdate();
}

void ScriptManager::ReleaseButton(size_t index, uint32_t scriptTime) {
  if (!buttonState.IsSet(index)) {
    return;
  }
  buttonState.Clear(index);
  script.HandleRelease(index, scriptTime);
  SendButtonStateUpdate();
}

void ScriptManager::SendButtonStateUpdate() const {
#if ENABLE_BUTTON_STATE_UPDATES
  if (isButtonStateUpdatesEnabled) {
    Console::Printf("EV {\"event\":\"button_state\",\"data\":\"%D\"}\n\n",
                    &buttonState, sizeof(buttonState));
  }
#endif
}

//---------------------------------------------------------------------------

void ScriptManager::Tick(uint32_t scriptTime) {
  if (Flash::IsUpdating()) {
    return;
  }
  ScriptManager &instance = GetInstance();
  if (!instance.isScriptValid) {
    return;
  }
  script.ExecuteTickScript(scriptTime);
}

//---------------------------------------------------------------------------

void ScriptManager::Reset() {
  script.Reset();
  TimerManager::instance.RemoveScriptTimers(Clock::GetMilliseconds());

  isScriptValid = script.IsValid();
  if (isScriptValid) {
    script.ExecuteInitScript(Clock::GetMilliseconds());
  }
}

//---------------------------------------------------------------------------

void ScriptManager::EnableScriptEvents_Binding(void *context,
                                               const char *commandLine) {
  ((Script *)context)->EnableScriptEvents();
  Console::SendOk();
}

void ScriptManager::DisableScriptEvents_Binding(void *context,
                                                const char *commandLine) {
  ((Script *)context)->DisableScriptEvents();
  Console::SendOk();
}

void ScriptManager::EnableButtonStateUpdates_Binding(void *context,
                                                     const char *commandLine) {
  ((ScriptManager *)context)->isButtonStateUpdatesEnabled = true;
  Console::SendOk();
}

void ScriptManager::DisableButtonStateUpdates_Binding(void *context,
                                                      const char *commandLine) {
  ((ScriptManager *)context)->isButtonStateUpdatesEnabled = false;
  Console::SendOk();
}

void ScriptManager::AddConsoleCommands(Console &console) {
  console.RegisterCommand("enable_script_events", "Enables events from scripts",
                          EnableScriptEvents_Binding, &script);
  console.RegisterCommand("disable_script_events",
                          "Disables events from scripts",
                          DisableScriptEvents_Binding, &script);
#if ENABLE_BUTTON_STATE_UPDATES
  console.RegisterCommand("enable_button_state_updates",
                          "Enables button state updates",
                          EnableButtonStateUpdates_Binding, this);
  console.RegisterCommand("disable_button_state_updates",
                          "Disables button state updates",
                          DisableButtonStateUpdates_Binding, this);
#endif
}

//---------------------------------------------------------------------------
