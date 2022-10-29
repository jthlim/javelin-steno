//---------------------------------------------------------------------------

#include "processor.h"
#include <vector>

//---------------------------------------------------------------------------

class FakeStenoProcessor : public StenoProcessorElement {
public:
  void Process(StenoKeyState value, StenoAction action) final;
  void Tick() { }

  struct ProcessArguments {
    StenoKeyState chord;
    StenoAction action;
  };

  std::vector<ProcessArguments> history;
  std::vector<StenoKeyState> triggers;
};

//---------------------------------------------------------------------------
