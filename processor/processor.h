//---------------------------------------------------------------------------

#pragma once
#include "../steno_key_state.h"

//---------------------------------------------------------------------------

enum class StenoAction {
  PRESS,
  RELEASE,
  TRIGGER,
  CANCEL, // Used to signal that any presses so far should be ignored.
};

//---------------------------------------------------------------------------

class StenoProcessorElement {
public:
  StenoProcessorElement() {}

  virtual void Process(StenoKeyState value, StenoAction action) = 0;
  virtual void Tick() = 0;
  virtual void PrintInfo() const = 0;

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
  void Process(StenoKeyState newState);
  void Tick() { next.Tick(); }
  void PrintInfo() const;

  const StenoKeyState &GetCurrentKeyState() const { return state; }

private:
  StenoKeyState state;
  StenoProcessorElement &next;
};

//---------------------------------------------------------------------------
