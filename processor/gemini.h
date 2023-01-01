//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

class StenoGemini final : public StenoProcessorElement {
public:
  void Process(const StenoKeyState &value, StenoAction action);
  void Tick() {}
  void PrintInfo() const;

private:
  size_t counter = 0;
};

//---------------------------------------------------------------------------
