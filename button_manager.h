//---------------------------------------------------------------------------

#pragma once
#include "button_state.h"
#include "script.h"
#include "static_allocate.h"

//---------------------------------------------------------------------------

// Handles logical buttons -> key / steno presses.
class ButtonManager {
public:
  static void Initialize(const uint8_t *scriptByteCode) {
    new (container) ButtonManager(scriptByteCode);
  }

  bool HasTickScript() const { return !script.IsTickScriptEmpty(); }
  void Update(const ButtonState &newButtonState, uint32_t scriptTime);
  void Tick(uint32_t scriptTime);
  void ExecuteScript(ScriptId scriptId);
  void PrintInfo() const { script.PrintInfo(); }

  void PressButton(size_t index, uint32_t scriptTime);
  void ReleaseButton(size_t index, uint32_t scriptTime);

  static ButtonManager &GetInstance() { return container.value; }

private:
  ButtonManager(const uint8_t *scriptByteCode);

  bool isScriptValid;
  ButtonState buttonState;
  Script script;

  static JavelinStaticAllocate<ButtonManager> container;
};

//---------------------------------------------------------------------------
