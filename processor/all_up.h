//---------------------------------------------------------------------------

#include "passthrough.h"

//---------------------------------------------------------------------------

// Processor that sends a steno stroke when all keys are released.
//
// The is the typical steno behavior.
class StenoAllUp final : public StenoPassthrough {
private:
  using super = StenoPassthrough;

public:
  StenoAllUp(StenoProcessorElement &nextProcessor) : super(&nextProcessor) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  StenoKeyState pressedKeyState;
};

//---------------------------------------------------------------------------
