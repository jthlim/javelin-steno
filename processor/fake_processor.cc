//---------------------------------------------------------------------------

#include "fake_processor.h"
#include "processor.h"

//---------------------------------------------------------------------------

void FakeStenoProcessor::Process(const StenoKeyState &value,
                                 StenoAction action) {
  ProcessArguments arguments = {.stroke = value, .action = action};
  history.push_back(arguments);
  if (action == StenoAction::TRIGGER) {
    triggers.push_back(value);
  }
}

//---------------------------------------------------------------------------
