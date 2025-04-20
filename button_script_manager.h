//---------------------------------------------------------------------------

#pragma once
#include "button_script.h"
#include "button_state.h"
#include "container/static_list.h"
#include "static_allocate.h"
#include "timer_manager.h"

//---------------------------------------------------------------------------

class Console;
struct Combo;

//---------------------------------------------------------------------------

struct PendingComboButton {
  size_t buttonIndex;
  uint32_t scriptTime;

  void Set(size_t buttonIndex, uint32_t scriptTime) {
    this->buttonIndex = buttonIndex;
    this->scriptTime = scriptTime;
  }
};

struct ComboMatches {
  Combo *fullMatch = nullptr;
  Combo *partialMatch = nullptr;
};

class PendingComboButtons : public StaticList<PendingComboButton, 64> {
public:
  ButtonState CreateButtonState(size_t count) const;
  ButtonState CreateButtonState() const {
    return CreateButtonState(GetCount());
  }
};

enum ComboMatch {
  NO_MATCH,
  PARTIAL_MATCH,
  FULL_MATCH,
};

struct Combo {
  bool isOrdered : 1;
  bool isPressed : 1;
  bool isReleased : 1;
  uint16_t comboTimeOut;
  uint16_t pressScriptOffset;
  uint16_t releaseScriptOffset;
  const uint8_t *buttonList;
  const ScriptByteCode *byteCode;
  ButtonState buttonState;

  void Set(bool isOrdered, int comboTimeOut, const uint8_t *buttonList,
           const ScriptByteCode *scriptByteCode, size_t pressScriptOffset,
           size_t releaseScriptOffset);

  ComboMatch Match(const PendingComboButtons &buttons, size_t buttonCount,
                   const ButtonState &state) const;
};

//---------------------------------------------------------------------------

// Handles logical buttons -> script press/release and combos.
class ButtonScriptManager : private TimerHandler {
public:
  static void Initialize(const uint8_t *scriptByteCode) {
    new (container) ButtonScriptManager(scriptByteCode);
  }

  // Called when a new script is uploaded.
  void Reset();

  bool HasTickScript() const { return !script.IsTickScriptEmpty(); }
  void Update(const ButtonState &newButtonState, uint32_t scriptTime);
  void Tick(uint32_t scriptTime);
  void PrintInfo() const { script.PrintInfo(); }

  void SetAllowButtonStateUpdates(bool value);

  void ExecuteByteCode(const ScriptByteCode *byteCode);
  void ExecuteScriptIndex(size_t index, uint32_t scriptTime,
                          const intptr_t *parameters, size_t parameterCount) {
    script.ExecuteScriptIndex(index, scriptTime, parameters, parameterCount);
  }

  void CancelAllScriptsForByteCode(const ScriptByteCode *byteCode,
                                   size_t byteCodeSize) {
    script.CancelAllScriptsForByteCode(byteCode, byteCodeSize);
  }

  static void ExecuteScript(ButtonScriptId scriptId);
  static ButtonScriptManager &GetInstance() { return container.value; }

  static void CallScript_Binding(void *context, const char *commandLine);
  static void EnableScriptEvents_Binding(void *context,
                                         const char *commandLine);
  static void DisableScriptEvents_Binding(void *context,
                                          const char *commandLine);
  static void EnableButtonStateUpdates_Binding(void *context,
                                               const char *commandLine);
  static void DisableButtonStateUpdates_Binding(void *context,
                                                const char *commandLine);
  static void PrintScriptGlobals_Binding(void *context,
                                         const char *commandLine);
  static void SetScriptGlobal_Binding(void *context, const char *commandLine);
  static void RunScript_Binding(void *context, const char *commandLine);
  void AddConsoleCommands(Console &console);
  static void PrintEventHistory() { container->script.PrintEventHistory(); }

  uint32_t Crc() const { return isScriptValid ? script.Crc() : 0; }

  void AddCombo(bool isOrdered, int comboTimeOut, const uint8_t *buttonList,
                const ScriptByteCode *byteCode, size_t pressScriptOffset,
                size_t releaseScriptOffset);
  void ResetCombos();

  void
  CancelAllCombosForByteCode(const Interval<const uint8_t *> &byteCodeRange);

private:
  ButtonScriptManager(const uint8_t *scriptByteCode);

  bool isScriptValid;

  // Controlled by console commands.
  bool isButtonStateUpdatesEnabled;

  // Controlled by scripts.
  bool allowButtonStateUpdates;
  ButtonState buttonState;
  ButtonScript script;

  PendingComboButtons pendingComboButtons;
  ButtonState activeComboButtonState;
  StaticList<Combo, 64> combos;

  // From TimerHandler
  virtual void Run(intptr_t id) final;

  void SendButtonStateUpdate() const;
  void SendButtonStateUpdate(const ButtonState &state) const;
  void CallScript(ButtonScriptId scriptId, const char *p);

  void PressButton(size_t buttonIndex, uint32_t scriptTime);
  void ReleaseButton(size_t buttonIndex, uint32_t scriptTime);

  void TriggerPendingComboButtons(size_t count);
  void TriggerCombo(Combo &combo, uint32_t scriptTime);
  void TriggerPress(size_t buttonIndex, uint32_t scriptTime);
  void TriggerRelease(size_t buttonIndex, uint32_t scriptTime);

  // Returns the number of elements actually matched.
  size_t TriggerMaximumMatch(size_t maxComboLength);

  ComboMatches Match(size_t buttonCount);
  void ResetComboData();

  static JavelinStaticAllocate<ButtonScriptManager> container;
};

//---------------------------------------------------------------------------
