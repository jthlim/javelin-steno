//---------------------------------------------------------------------------

#include "processor_list.h"

//---------------------------------------------------------------------------

void StenoProcessorList::Process(const StenoKeyState &value,
                                 StenoAction action) {
  for (size_t i = 0; i < processorCount; ++i) {
    processors[i]->Process(value, action);
  }
}

void StenoProcessorList::Tick() {
  for (size_t i = 0; i < processorCount; ++i) {
    processors[i]->Tick();
  }
}

void StenoProcessorList::PrintInfo() const {
  for (size_t i = 0; i < processorCount; ++i) {
    processors[i]->PrintInfo();
  }
}

bool StenoProcessorList::ProcessScanCode(int scanCodeAndModifiers,
                                         ScanCodeAction action) {
  for (size_t i = 0; i < processorCount; ++i) {
    if (processors[i]->ProcessScanCode(scanCodeAndModifiers, action)) {
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------
