//---------------------------------------------------------------------------

#pragma once
#include "../scan_code_action.h"
#include "../steno_key_state.h"

//---------------------------------------------------------------------------

enum class StenoAction {
  PRESS,
  RELEASE,
  TRIGGER,
  CANCEL_ALL, // Used to signal that any presses so far should be ignored.
  CANCEL_KEY, // Used to ignore a key that was pressed.
};

//---------------------------------------------------------------------------

class StenoProcessorElement {
public:
  StenoProcessorElement() {}

  virtual void Process(const StenoKeyState &value, StenoAction action) = 0;
  virtual void PrintInfo() const = 0;
  virtual bool ProcessScanCode(uint32_t scanCodeAndModifiers,
                               ScanCodeAction action);

private:
  // Disable copy and assignment, to ensure code isn't accidentally copying
  // objects.
  StenoProcessorElement(const StenoProcessorElement &) = delete;
  void operator=(const StenoProcessorElement &) = delete;
};

//---------------------------------------------------------------------------

class StenoProcessor {
public:
  StenoProcessor(StenoProcessorElement &next) : next(next) {}

  void Process(StenoKey key, bool isPress);
  void Process(const StenoKeyState &newState);
  void PrintInfo() const;

  const StenoKeyState &GetCurrentKeyState() const { return state; }

private:
  StenoKeyState state;
  StenoProcessorElement &next;
};

//---------------------------------------------------------------------------
