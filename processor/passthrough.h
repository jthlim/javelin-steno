//---------------------------------------------------------------------------

#pragma once
#include "processor.h"

//---------------------------------------------------------------------------

class StenoPassthrough : public StenoProcessorElement {
public:
  StenoPassthrough(StenoProcessorElement *next) : next(next) {}

  void Process(const StenoKeyState &value, StenoAction action) {
    next->Process(value, action);
  }

  void PrintInfo() const { next->PrintInfo(); }
  bool ProcessScanCode(uint32_t scanCodeAndModifiers, ScanCodeAction action) {
    return next->ProcessScanCode(scanCodeAndModifiers, action);
  }

  StenoProcessorElement *GetNext() const { return next; }
  void SetNext(StenoProcessorElement *value) { next = value; }

protected:
  StenoProcessorElement *next;
};

//---------------------------------------------------------------------------
