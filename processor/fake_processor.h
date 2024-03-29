//---------------------------------------------------------------------------

#include "processor.h"
#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

class FakeStenoProcessor final : public StenoProcessorElement
{
public:
  void Process(const StenoKeyState &value, StenoAction action);
  void Tick() {}
  void PrintInfo() const {}

  struct ProcessArguments
  {
    StenoKeyState stroke;
    StenoAction action;
  };

#if RUN_TESTS
  std::vector<ProcessArguments> history;
  std::vector<StenoKeyState> triggers;
#endif
};

//---------------------------------------------------------------------------
