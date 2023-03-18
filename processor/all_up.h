//---------------------------------------------------------------------------

#include "passthrough.h"

//---------------------------------------------------------------------------

// Processor that sends a steno stroke when all keys are released.
//
// The is the typical steno behavior.
class StenoAllUp final : public StenoPassthrough {
public:
  StenoAllUp(StenoProcessorElement &nextProcessor)
      : StenoPassthrough(&nextProcessor) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  StenoKeyState pressedKeyState;
};

//---------------------------------------------------------------------------
