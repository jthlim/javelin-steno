//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

class StenoProcessorList final : public StenoProcessorElement {
public:
  StenoProcessorList(StenoProcessorElement *const *processors,
                     size_t processorCount)
      : processors(processors), processorCount(processorCount) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void Tick();
  void PrintInfo() const;
  bool ProcessScanCode(int scanCodeAndModifiers, ScanCodeAction action);

private:
  StenoProcessorElement *const *processors;
  size_t processorCount;
};

//---------------------------------------------------------------------------
