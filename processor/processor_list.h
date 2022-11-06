//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

class StenoProcessorList : public StenoProcessorElement {
public:
  StenoProcessorList(StenoProcessorElement *const *processors,
                     size_t processorCount)
      : processors(processors), processorCount(processorCount) {}

  void Process(StenoKeyState value, StenoAction action) final;
  void Tick();
  void PrintInfo() const final;

private:
  StenoProcessorElement *const *processors;
  size_t processorCount;
};

//---------------------------------------------------------------------------
