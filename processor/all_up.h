//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

// Processor that sends a steno chord when all keys are released.
//
// The is the typical steno behavior.
class StenoAllUp final : public StenoProcessorElement {
public:
  StenoAllUp(StenoProcessorElement &nextProcessor)
      : nextProcessor(nextProcessor) {}

  void Process(StenoKeyState value, StenoAction action);
  void Tick() { nextProcessor.Tick(); }
  void PrintInfo() const final;

private:
  StenoKeyState pressedKeyState;
  StenoProcessorElement &nextProcessor;
};

//---------------------------------------------------------------------------
