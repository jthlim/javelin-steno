//---------------------------------------------------------------------------

#pragma once
#include "button_script.h"
#include "button_state.h"
#include "static_allocate.h"

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

// Handles logical buttons -> key / steno presses.
class ButtonScriptManager {
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

  void PressButton(size_t index, uint32_t scriptTime);
  void ReleaseButton(size_t index, uint32_t scriptTime);

  void SetAllowButtonStateUpdates(bool value);

  static void ExecuteScript(ButtonScriptId scriptId);
  static ButtonScriptManager &GetInstance() { return container.value; }

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
  void AddConsoleCommands(Console &console);
  static void PrintEventHistory() { container->script.PrintEventHistory(); }

  uint32_t Crc() const { return isScriptValid ? script.Crc() : 0; }

private:
  ButtonScriptManager(const uint8_t *scriptByteCode);

  bool isScriptValid;

  // Controlled by console commands.
  bool isButtonStateUpdatesEnabled;

  // Controlled by scripts.
  bool allowButtonStateUpdates;
  ButtonState buttonState;
  ButtonScript script;

  void SendButtonStateUpdate() const;
  void SendButtonStateUpdate(const ButtonState &state) const;

  static JavelinStaticAllocate<ButtonScriptManager> container;
};

//---------------------------------------------------------------------------
