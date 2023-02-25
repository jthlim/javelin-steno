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

  void Tick() { next->Tick(); }
  void PrintInfo() const { next->PrintInfo(); }

  void SetNext(StenoProcessorElement *value) { next = value; }

private:
  StenoProcessorElement *next;
};

//---------------------------------------------------------------------------
