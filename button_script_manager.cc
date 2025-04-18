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

constexpr intptr_t TIMER_ID = -('B' * 256 + 'S');

//--------------------------------------------------------------------

JavelinStaticAllocate<ButtonScriptManager> ButtonScriptManager::container;

//---------------------------------------------------------------------------

ButtonState PendingComboButtons::CreateButtonState(size_t count) const {
  ButtonState state;
  state.ClearAll();
  for (size_t i = 0; i < count; ++i) {
    state.Set((*this)[i].buttonIndex);
  }
  return state;
}

void Combo::Set(bool isOrdered, int comboTimeOut, const uint8_t *buttonList,
                const ScriptByteCode *byteCode, size_t pressScriptOffset,
                size_t releaseScriptOffset) {
  this->isOrdered = isOrdered;
  this->isPressed = false;
  this->isReleased = true;
  this->comboTimeOut = comboTimeOut;
  this->byteCode = byteCode;
  this->pressScriptOffset = pressScriptOffset;
  this->releaseScriptOffset = releaseScriptOffset;
  this->buttonList = buttonList;
  buttonState.ClearAll();
  while (*buttonList != 0xff) {
    buttonState.Set(*buttonList++);
  }
}

ComboMatch Combo::Match(const PendingComboButtons &buttons, size_t buttonCount,
                        const ButtonState &state) const {
  if ((state & ~this->buttonState).IsAnySet()) {
    return ComboMatch::NO_MATCH;
  }

  if (state != this->buttonState) {
    if (!isOrdered) {
      return ComboMatch::PARTIAL_MATCH;
    }

    for (size_t i = 0; i < buttonCount; ++i) {
      if (buttonList[i] != buttons[i].buttonIndex) {
        return ComboMatch::NO_MATCH;
      }
    }
    return ComboMatch::PARTIAL_MATCH;
  }

  // At this point on, state bits match.
  if (!isOrdered) {
    return ComboMatch::FULL_MATCH;
  }

  for (size_t i = 0; i < buttonCount; ++i) {
    if (buttonList[i] != buttons[i].buttonIndex) {
      return ComboMatch::NO_MATCH;
    }
  }
  return ComboMatch::FULL_MATCH;
}

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
    ReleaseButton(buttonIndex, scriptTime);

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
    PressButton(buttonIndex, scriptTime);

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

void ButtonScriptManager::PressButton(size_t buttonIndex, uint32_t scriptTime) {
  if (combos.IsEmpty()) {
    return TriggerPress(buttonIndex, scriptTime);
  }

  pendingComboButtons.Add().Set(buttonIndex, scriptTime);

  do {
    const ButtonState buttonState = pendingComboButtons.CreateButtonState();
    const ComboMatches matches = Match(pendingComboButtons.GetCount());

    if (matches.partialMatch) {
      if (matches.partialMatch->comboTimeOut == 0) {
        break;
      }
      TimerManager::instance.StartTimer(
          TIMER_ID, matches.partialMatch->comboTimeOut, 0, this, scriptTime);
      return;
    }

    if (matches.fullMatch != nullptr) {
      TriggerCombo(*matches.fullMatch, scriptTime);
      pendingComboButtons.Reset();
      break;
    }

    const size_t comboLength = pendingComboButtons.GetCount() - 1;
    TriggerMaximumMatch(comboLength);
  } while (pendingComboButtons.IsNotEmpty());

  TimerManager::instance.StopTimer(TIMER_ID, scriptTime);
}

void ButtonScriptManager::ReleaseButton(size_t buttonIndex,
                                        uint32_t scriptTime) {
  if (combos.IsEmpty()) {
    return TriggerRelease(buttonIndex, scriptTime);
  }

  TimerManager::instance.StopTimer(TIMER_ID, scriptTime);

  // If the index is in the pending list, then trigger all items up to and
  // including the entry with index.
  for (size_t i = 0; i < pendingComboButtons.GetCount(); ++i) {
    if (pendingComboButtons[i].buttonIndex == buttonIndex) {
      TriggerPendingComboButtons(i + 1);
      break;
    }
  }

  if (activeComboButtonState.IsClear(buttonIndex)) {
    return TriggerRelease(buttonIndex, scriptTime);
  }
  activeComboButtonState.Clear(buttonIndex);

  for (Combo &combo : combos) {
    if (combo.isPressed && combo.buttonState.IsSet(buttonIndex)) {
      if (!combo.isReleased) {
        combo.isReleased = true;
        script.ExecuteScriptCallback(combo.byteCode, combo.releaseScriptOffset,
                                     scriptTime);
      }
      combo.isPressed = (activeComboButtonState & combo.buttonState).IsAnySet();
    }
  }
}

void ButtonScriptManager::Run(intptr_t id) {
  TriggerPendingComboButtons(pendingComboButtons.GetCount());
}

void ButtonScriptManager::TriggerPendingComboButtons(size_t count) {
  while (count) {
    count -= TriggerMaximumMatch(count);
  }
}

size_t ButtonScriptManager::TriggerMaximumMatch(size_t maxComboLength) {
  for (size_t comboLength = maxComboLength; comboLength > 0; --comboLength) {
    const ButtonState state =
        pendingComboButtons.CreateButtonState(comboLength);
    const ComboMatches matches = Match(comboLength);
    if (matches.fullMatch != nullptr) {
      TriggerCombo(*matches.fullMatch,
                   pendingComboButtons[comboLength - 1].scriptTime);
      pendingComboButtons.PopFrontCount(comboLength);
      return comboLength;
    }
  }

  const PendingComboButton &button = pendingComboButtons.Front();
  TriggerPress(button.buttonIndex, button.scriptTime);
  pendingComboButtons.PopFront();
  return 1;
}

void ButtonScriptManager::TriggerCombo(Combo &combo, uint32_t scriptTime) {
  combo.isPressed = true;
  combo.isReleased = false;
  activeComboButtonState |= combo.buttonState;
  script.ExecuteScriptCallback(combo.byteCode, combo.pressScriptOffset,
                               scriptTime);
}

void ButtonScriptManager::TriggerPress(size_t index, uint32_t scriptTime) {
  script.IncrementPressCount();
  script.HandlePress(index, scriptTime);
}

void ButtonScriptManager::TriggerRelease(size_t index, uint32_t scriptTime) {
  script.IncrementReleaseCount();
  script.HandleRelease(index, scriptTime);
}

ComboMatches ButtonScriptManager::Match(size_t buttonCount) {
  const ButtonState state = pendingComboButtons.CreateButtonState(buttonCount);
  ComboMatches matches;
  for (Combo &combo : combos) {
    switch (combo.Match(pendingComboButtons, buttonCount, state)) {
    case ComboMatch::NO_MATCH:
      break;
    case ComboMatch::PARTIAL_MATCH:
      matches.partialMatch = &combo;
      break;
    case ComboMatch::FULL_MATCH:
      matches.fullMatch = &combo;
      break;
    }
  }
  return matches;
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

void ButtonScriptManager::AddCombo(bool isOrdered, int comboTimeOut,
                                   const uint8_t *buttonList,
                                   const ScriptByteCode *byteCode,
                                   size_t pressScriptOffset,
                                   size_t releaseScriptOffset) {
  if (combos.IsFull()) {
    Console::Printf("AddCombo ignored - list full\n\n");
    return;
  }

  for (Combo &combo : combos) {
    if (combo.buttonList == buttonList) {
      Console::Printf(
          "AddCombo warning - replacing previously registered combo\n\n");
      combo.Set(isOrdered, comboTimeOut, buttonList, byteCode,
                pressScriptOffset, releaseScriptOffset);
      return;
    }
  }

  Combo &combo = combos.Add();
  combo.Set(isOrdered, comboTimeOut, buttonList, byteCode, pressScriptOffset,
            releaseScriptOffset);
}

void ButtonScriptManager::ResetCombos() {
  for (Combo &combo : combos) {
    if (combo.isPressed && !combo.isReleased) {
      script.ExecuteScriptCallback(combo.byteCode, combo.releaseScriptOffset,
                                   Clock::GetMilliseconds());
    }
  }
  combos.Reset();
}

void ButtonScriptManager::CancelAllCombosForByteCode(
    const Interval<const uint8_t *> &byteCodeRange) {
  for (size_t i = combos.GetCount(); i != 0;) {
    --i;
    const Combo &combo = combos[i];
    if (byteCodeRange.Contains(
            combo.byteCode->GetScriptData<uint8_t>(combo.pressScriptOffset)) ||
        byteCodeRange.Contains(combo.byteCode->GetScriptData<uint8_t>(
            combo.releaseScriptOffset))) {
      combos.RemoveIndex(i);
      Console::Printf("Removed stale combo\n");
    }
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
