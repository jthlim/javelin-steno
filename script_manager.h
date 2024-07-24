//---------------------------------------------------------------------------

#pragma once
#include "button_state.h"
#include "script.h"
#include "static_allocate.h"

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

// Handles logical buttons -> key / steno presses.
class ScriptManager {
public:
  static void Initialize(const uint8_t *scriptByteCode) {
    new (container) ScriptManager(scriptByteCode);
  }

  // Called when a new script is uploaded.
  void Reset();

  bool HasTickScript() const { return !script.IsTickScriptEmpty(); }
  void Update(const ButtonState &newButtonState, uint32_t scriptTime);
  void Tick(uint32_t scriptTime);
  void PrintInfo() const { script.PrintInfo(); }

  void PressButton(size_t index, uint32_t scriptTime);
  void ReleaseButton(size_t index, uint32_t scriptTime);

  static void ExecuteScript(ScriptId scriptId);
  static ScriptManager &GetInstance() { return container.value; }

  static void EnableScriptEvents_Binding(void *context,
                                         const char *commandLine);
  static void DisableScriptEvents_Binding(void *context,
                                          const char *commandLine);
  static void EnableButtonStateUpdates_Binding(void *context,
                                               const char *commandLine);
  static void DisableButtonStateUpdates_Binding(void *context,
                                                const char *commandLine);
  void AddConsoleCommands(Console &console);
  void PrintScriptHistory() { script.PrintScriptHistory(); }

  uint32_t Crc() const { return isScriptValid ? script.Crc() : 0; }

private:
  ScriptManager(const uint8_t *scriptByteCode);

  bool isScriptValid;
  bool isButtonStateUpdatesEnabled;
  ButtonState buttonState;
  Script script;

  void SendButtonStateUpdate() const;

  static JavelinStaticAllocate<ScriptManager> container;
};

//---------------------------------------------------------------------------
