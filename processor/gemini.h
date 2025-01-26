//---------------------------------------------------------------------------

#include "passthrough.h"

//---------------------------------------------------------------------------

class StenoGemini final : public StenoPassthrough {
public:
  StenoGemini(StenoProcessorElement *next) : StenoPassthrough(next) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  size_t counter = 0;
};

//---------------------------------------------------------------------------
