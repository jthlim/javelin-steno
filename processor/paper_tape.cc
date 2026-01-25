//---------------------------------------------------------------------------

#include "paper_tape.h"
#include "../console.h"

//---------------------------------------------------------------------------

void PaperTape::Process(const StenoKeyState &value, StenoAction action) {
  if (action != StenoAction::TRIGGER) {
    return;
  }
  if (!Console::IsEventEnabled(ConsoleEvent::PAPER_TAPE)) {
    return;
  }
  const StenoStroke stroke = value.ToStroke();
  Console::Printf("EV e: p\no: %t\n\n", &stroke);
}

void PaperTape::PrintInfo() const {}

//---------------------------------------------------------------------------
