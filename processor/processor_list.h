//---------------------------------------------------------------------------

#include "../sized_list.h"
#include "processor.h"

//---------------------------------------------------------------------------

class StenoProcessorList final : public StenoProcessorElement {
public:
  StenoProcessorList(StenoProcessorElement *const *processors,
                     size_t processorCount)
      : processors({processorCount, processors}) {}

  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;
  bool ProcessScanCode(uint32_t scanCodeAndModifiers, ScanCodeAction action);

private:
  const SizedList<StenoProcessorElement *> processors;
};

//---------------------------------------------------------------------------
