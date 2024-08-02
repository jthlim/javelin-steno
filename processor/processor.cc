//---------------------------------------------------------------------------

#include "processor.h"
#include "../console.h"

//---------------------------------------------------------------------------

bool StenoProcessorElement::ProcessScanCode(uint32_t scanCodeAndModifiers,
                                            ScanCodeAction action) {
  return false;
}

//---------------------------------------------------------------------------

void StenoProcessor::Process(StenoKey key, bool isPress) {
  state.Process(key, isPress);
  next.Process(state, isPress ? StenoAction::PRESS : StenoAction::RELEASE);
}

void StenoProcessor::Process(const StenoKeyState &newState) {
  const StenoKeyState pressed = newState & ~state;
  if (pressed.IsNotEmpty()) {
    next.Process(state | newState, StenoAction::PRESS);
  }

  if ((state & ~newState).IsNotEmpty()) {
    next.Process(newState, StenoAction::RELEASE);
  }

  state = newState;
}

void StenoProcessor::PrintInfo() const { next.PrintInfo(); }

//---------------------------------------------------------------------------
