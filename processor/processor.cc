//---------------------------------------------------------------------------

#include "processor.h"
#include "../console.h"

//---------------------------------------------------------------------------

void StenoProcessor::Process(StenoKey key, bool isPress) {
  state.Process(key, isPress);
  next.Process(state, isPress ? StenoAction::PRESS : StenoAction::RELEASE);
}

void StenoProcessor::Process(StenoKeyState newState) {
  StenoKeyState pressed = newState & ~state;
  if (pressed.IsNotEmpty()) {
    next.Process(state | newState, StenoAction::PRESS);
  }

  if ((state & ~newState).IsNotEmpty()) {
    next.Process(newState, StenoAction::RELEASE);
  }

  state = newState;
}

void StenoProcessor::PrintInfo() const {
  Console::Printf("Processing chain\n");
  next.PrintInfo();
}

//---------------------------------------------------------------------------
