//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

// Processor that switches between primary and secondary processors with a
// given stroke.
class StenoSwitch final : public StenoProcessorElement {
public:
  StenoSwitch(StenoProcessorElement &defaultProcessor,
              StenoProcessorElement &alternateProcessor)
      : defaultProcessor(defaultProcessor),
        alternateProcessor(alternateProcessor) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void Tick();
  void PrintInfo() const;

private:
  bool useDefault = true;

  StenoProcessorElement &defaultProcessor;
  StenoProcessorElement &alternateProcessor;
};

//---------------------------------------------------------------------------
