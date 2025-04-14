//---------------------------------------------------------------------------

#include "button_script_manager.h"
#include "base64.h"
#include "clock.h"
#include "console.h"
#include "flash.h"
#include "str.h"
#include "timer_manager.h"
#include "unicode.h"

//---------------------------------------------------------------------------

#define CONSOLE_LOG_BUTTON_PRESSES 0
#define PROFILE_BUTTON_ACTIVITY 0

//-------------------------------------------------------------------- th-------

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
  if (!isScriptValid) [[unlikely]] {
    return;
  }

  if (newButtonState == buttonState) {
    return;
  }

  const ButtonState pressedButtons = newButtonState & ~buttonState;
  const ButtonState releasedButtons = buttonState & ~newButtonState;

  buttonState = newButtonState;

  for (const size_t buttonIndex : releasedButtons) {
#if PROFILE_BUTTON_ACTIVITY
    const uint32_t start = Clock::GetMicroseconds();
#endif
#if CONSOLE_LOG_BUTTON_PRESSES
    Console::Printf("Release %zu at %u ms, now: %u ms\n\n", buttonIndex,
                    scriptTime, Clock::GetMilliseconds());
#endif
    script.IncrementReleaseCount();
    script.HandleRelease(buttonIndex, scriptTime);

#if PROFILE_BUTTON_ACTIVITY
    const uint32_t end = Clock::GetMicroseconds();
    Console::Printf("Release %zu: %u us\n\n", buttonIndex, end - start);
#endif
  }

  for (const size_t buttonIndex : pressedButtons) {
#if PROFILE_BUTTON_ACTIVITY
    const uint32_t start = Clock::GetMicroseconds();
#endif
#if CONSOLE_LOG_BUTTON_PRESSES
    Console::Printf("Press %zu at %u ms, now: %u ms\n\n", buttonIndex,
                    scriptTime, Clock::GetMilliseconds());
#endif
    script.IncrementPressCount();
    script.HandlePress(buttonIndex, scriptTime);

#if PROFILE_BUTTON_ACTIVITY
    const uint32_t end = Clock::GetMicroseconds();
    Console::Printf("Press %zu: %u us\n\n", buttonIndex, end - start);
#endif
  }
  SendButtonStateUpdate();
}

void ButtonScriptManager::ExecuteByteCode(const ScriptByteCode *byteCode) {
  if (Flash::IsUpdating()) [[unlikely]] {
    return;
  }
  script.ExecuteByteCode(byteCode);
}

[[gnu::noinline]] void
ButtonScriptManager::ExecuteScript(ButtonScriptId scriptId) {
  if (Flash::IsUpdating()) [[unlikely]] {
    return;
  }
  ButtonScriptManager &instance = GetInstance();
  if (!instance.isScriptValid) [[unlikely]] {
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
  if (Flash::IsUpdating()) [[unlikely]] {
    return;
  }
  if (!isScriptValid) [[unlikely]] {
    return;
  }
  script.ExecuteTickScript(scriptTime);
}

//---------------------------------------------------------------------------

void ButtonScriptManager::Reset() {
  script.Reset();
  TimerManager::instance.RemoveScriptTimers(Clock::GetMilliseconds());

  isScriptValid = script.IsValid();
  if (!isScriptValid) [[unlikely]] {
    return;
  }

  script.SetReinit(true);
  script.ExecuteInitScript(Clock::GetMilliseconds());
  script.SetReinit(false);
}

//---------------------------------------------------------------------------

void ButtonScriptManager::CallScript(ButtonScriptId scriptId, const char *p) {
  while (*p) {
    if (Unicode::IsWhitespace(*p)) {
      ++p;
      continue;
    }

    int parameter;
    const char *next = Str::ParseInteger(&parameter, p);
    if (!next) {
      Console::Printf("ERR Unable to parse parameter %s\n\n", p);
      return;
    }
    script.Push(parameter);
    p = next;
  }

  Console::SendOk();

  ExecuteScript(scriptId);
}

void ButtonScriptManager::CallScript_Binding(void *context,
                                             const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR No scriptId specified\n\n");
    return;
  }

  ButtonScriptId scriptId;
  p = Str::ParseInteger((int *)&scriptId, p + 1, false);
  if (!p) {
    Console::Printf("ERR Unable to parse scriptId\n\n");
    return;
  }
  if (scriptId >= ButtonScriptId::COUNT) {
    Console::Printf("ERR Invalid scriptId\n\n");
    return;
  }

  ButtonScriptManager *manager = (ButtonScriptManager *)context;
  intptr_t *stackTop = manager->script.GetStackTop();
  manager->CallScript(scriptId, p);
  manager->script.SetStackTop(stackTop);
}

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

void ButtonScriptManager::SetScriptGlobal_Binding(void *context,
                                                  const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing globalIndex\n\n");
    return;
  }

  int globalIndex;
  p = Str::ParseInteger(&globalIndex, p + 1, false);
  if (!p) {
    Console::Printf("ERR Unable to parse globalIndex\n\n");
    return;
  }
  if (*p == '\0') {
    Console::Printf("ERR Missing value\n\n");
    return;
  }

  int value;
  p = Str::ParseInteger(&value, p + 1, true);
  if (!p) {
    Console::Printf("ERR Unable to parse value\n\n");
    return;
  }

  ((ButtonScriptManager *)context)->script.SetGlobal(globalIndex, value);
  Console::SendOk();
}

void ButtonScriptManager::RunScript_Binding(void *context,
                                            const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing script byte code\n\n");
    return;
  }

  static uint8_t buffer[256];

  ButtonScriptManager *manager = (ButtonScriptManager *)context;
  ScriptByteCode *byteCode = (ScriptByteCode *)buffer;
  manager->CancelAllScriptsForByteCode(byteCode, 256);

  Base64::Decode(buffer, (const uint8_t *)p + 1);
  if (!byteCode->IsValid()) {
    Console::Printf("ERR Invalid byte code\n\n");
    return;
  }

  Console::SendOk();

  manager->ExecuteByteCode(byteCode);
}

void ButtonScriptManager::AddConsoleCommands(Console &console) {
  console.RegisterCommand("call_script",
                          "Call scripts registered with setScript",
                          CallScript_Binding, this);
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
  console.RegisterCommand("print_script_globals",
                          "Prints non-zero script globals",
                          PrintScriptGlobals_Binding, this);
  console.RegisterCommand("set_script_global",
                          "Set script global index to a value",
                          SetScriptGlobal_Binding, this);
  console.RegisterCommand("run_script", "Runs a script", RunScript_Binding,
                          this);
}

//---------------------------------------------------------------------------
