//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

// Processor that switches between primary and secondary processors with a
// given chord.
class StenoSwitch final : public StenoProcessorElement {
public:
  StenoSwitch(StenoProcessorElement &defaultProcessor,
              StenoProcessorElement &alternateProcessor)
      : defaultProcessor(defaultProcessor),
        alternateProcessor(alternateProcessor) {}

  void Process(StenoKeyState value, StenoAction action) final;
  void Tick() final;
  void PrintInfo() const final;

private:
  bool useDefault = true;

  StenoProcessorElement &defaultProcessor;
  StenoProcessorElement &alternateProcessor;
};

//---------------------------------------------------------------------------
