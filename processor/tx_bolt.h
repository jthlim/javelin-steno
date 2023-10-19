//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

class StenoTxBolt final : public StenoProcessorElement {
public:
  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  size_t counter = 0;
};

//---------------------------------------------------------------------------
