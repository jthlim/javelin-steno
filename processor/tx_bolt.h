//---------------------------------------------------------------------------

#include "passthrough.h"

//---------------------------------------------------------------------------

class StenoTxBolt final : public StenoPassthrough {
public:
  StenoTxBolt(StenoProcessorElement *next) : StenoPassthrough(next) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  size_t counter = 0;
};

//---------------------------------------------------------------------------
