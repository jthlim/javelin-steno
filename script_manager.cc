//---------------------------------------------------------------------------

#include "script_manager.h"
#include "clock.h"
#include "console.h"
#include "script_byte_code.h"
#include "timer_manager.h"

//---------------------------------------------------------------------------

#define CONSOLE_LOG_BUTTON_PRESSES 0

//---------------------------------------------------------------------------

JavelinStaticAllocate<ScriptManager> ScriptManager::container;

//---------------------------------------------------------------------------

ScriptManager::ScriptManager(const uint8_t *scriptByteCode)
    : script(scriptByteCode) {
  isScriptValid = ((StenoScriptByteCodeData *)scriptByteCode)->IsValid();
  if (isScriptValid) {
    script.ExecuteInitScript(0);
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

  for (size_t buttonIndex : releasedButtons) {
#if CONSOLE_LOG_BUTTON_PRESSES
    Console::Printf("Release %zu at %u ms, now: %u ms\n\n", buttonIndex,
                    scriptTime, Clock::GetMilliseconds());
#endif
    script.HandleRelease(buttonIndex, scriptTime);
  }

  for (size_t buttonIndex : pressedButtons) {
#if CONSOLE_LOG_BUTTON_PRESSES
    Console::Printf("Press %zu at %u ms, now: %u ms\n\n", buttonIndex,
                    scriptTime, Clock::GetMilliseconds());
#endif
    script.HandlePress(buttonIndex, scriptTime);
  }
}

void ScriptManager::ExecuteScript(ScriptId scriptId) {
  ScriptManager &instance = GetInstance();
  if (instance.isScriptValid) {
    instance.script.ExecuteScriptId(scriptId, Clock::GetMilliseconds());
  }
}

void ScriptManager::PressButton(size_t index, uint32_t scriptTime) {
  if (buttonState.IsSet(index)) {
    return;
  }
  buttonState.Set(index);
  script.HandlePress(index, scriptTime);
}

void ScriptManager::ReleaseButton(size_t index, uint32_t scriptTime) {
  if (!buttonState.IsSet(index)) {
    return;
  }
  buttonState.Clear(index);
  script.HandleRelease(index, scriptTime);
}

//---------------------------------------------------------------------------

void ScriptManager::Tick(uint32_t scriptTime) {
  if (!isScriptValid) {
    return;
  }
  script.ExecuteTickScript(scriptTime);
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

void ScriptManager::AddConsoleCommands(Console &console) {
  console.RegisterCommand("enable_script_events", "Enables events from scripts",
                          EnableScriptEvents_Binding, &script);
  console.RegisterCommand("disable_script_events",
                          "Disables events from scripts",
                          DisableScriptEvents_Binding, &script);
}

//---------------------------------------------------------------------------
