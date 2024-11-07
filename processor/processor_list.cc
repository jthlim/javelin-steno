//---------------------------------------------------------------------------

#include "processor_list.h"

//---------------------------------------------------------------------------

void StenoProcessorList::Process(const StenoKeyState &value,
                                 StenoAction action) {
  for (StenoProcessorElement *processor : processors) {
    processor->Process(value, action);
  }
}

void StenoProcessorList::PrintInfo() const {
  for (StenoProcessorElement *processor : processors) {
    processor->PrintInfo();
  }
}

bool StenoProcessorList::ProcessScanCode(uint32_t scanCodeAndModifiers,
                                         ScanCodeAction action) {
  for (StenoProcessorElement *processor : processors) {
    if (processor->ProcessScanCode(scanCodeAndModifiers, action)) {
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------
