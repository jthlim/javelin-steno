//---------------------------------------------------------------------------

#include "passthrough.h"

//---------------------------------------------------------------------------

class StenoPassport final : public StenoPassthrough {
public:
  StenoPassport(StenoProcessorElement *next) : StenoPassthrough(next) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  size_t counter = 0;
};

//---------------------------------------------------------------------------
