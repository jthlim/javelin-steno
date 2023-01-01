//---------------------------------------------------------------------------

#include "processor.h"
#include <vector>

//---------------------------------------------------------------------------

class FakeStenoProcessor final : public StenoProcessorElement {
public:
  void Process(const StenoKeyState &value, StenoAction action);
  void Tick() {}
  void PrintInfo() const {}

  struct ProcessArguments {
    StenoKeyState chord;
    StenoAction action;
  };

  std::vector<ProcessArguments> history;
  std::vector<StenoKeyState> triggers;
};

//---------------------------------------------------------------------------
