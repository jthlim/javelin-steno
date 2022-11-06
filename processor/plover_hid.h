//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

class StenoPloverHid : public StenoProcessorElement {
public:
  void Process(StenoKeyState value, StenoAction action) final;
  void Tick() {}
  void PrintInfo() const final;

private:
  size_t counter = 0;

  void SendPacket(const StenoPloverHidPacket &packet);
};

//---------------------------------------------------------------------------
