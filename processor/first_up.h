//---------------------------------------------------------------------------

#include "passthrough.h"

//---------------------------------------------------------------------------

// Processor that sends a steno stroke when the first key is released.
//
// This enable easy repeat with navigation.
class StenoFirstUp final : public StenoPassthrough {
public:
  StenoFirstUp(StenoProcessorElement &nextProcessor)
      : StenoPassthrough(&nextProcessor) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  bool wasLastEventAPress = false;
  StenoKeyState pressedKeyState;
};

//---------------------------------------------------------------------------
