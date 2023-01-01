//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

// Processor that sends a steno chord when the first key is released.
//
// This enable easy repeat with navigation.
class StenoFirstUp final : public StenoProcessorElement {
public:
  StenoFirstUp(StenoProcessorElement &nextProcessor)
      : nextProcessor(nextProcessor) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void Tick() { nextProcessor.Tick(); }
  void PrintInfo() const;

private:
  bool wasLastEventAPress = false;
  StenoKeyState pressedKeyState;
  StenoProcessorElement &nextProcessor;
};

//---------------------------------------------------------------------------
