//---------------------------------------------------------------------------

#include "passthrough.h"

//---------------------------------------------------------------------------

class StenoProcat final : public StenoPassthrough {
public:
  StenoProcat(StenoProcessorElement *next) : StenoPassthrough(next) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  size_t counter = 0;
};

//---------------------------------------------------------------------------
