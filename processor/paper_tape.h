//---------------------------------------------------------------------------

#pragma once
#include "processor.h"

//---------------------------------------------------------------------------

class Console;
class StenoKeyState;

//---------------------------------------------------------------------------

class PaperTape final : public StenoProcessorElement {
public:
  virtual void Process(const StenoKeyState &value, StenoAction action);
  virtual void PrintInfo() const;
};

//---------------------------------------------------------------------------
