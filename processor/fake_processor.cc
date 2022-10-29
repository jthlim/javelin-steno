//---------------------------------------------------------------------------

#include "fake_processor.h"
#include "processor.h"

//---------------------------------------------------------------------------

void FakeStenoProcessor::Process(StenoKeyState value, StenoAction action) {
  ProcessArguments arguments = {.chord = value, .action = action};
  history.push_back(arguments);
  if (action == StenoAction::TRIGGER) {
    triggers.push_back(value);
  }
}

//---------------------------------------------------------------------------