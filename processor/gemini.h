//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

class StenoGemini : public StenoProcessorElement {
public:
  void Process(StenoKeyState value, StenoAction action) final;
  void Tick() {}
  void PrintInfo() const final;

private:
  size_t counter = 0;
};

//---------------------------------------------------------------------------
