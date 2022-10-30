//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

class StenoPloverHid : public StenoProcessorElement {
public:
  void Process(StenoKeyState value, StenoAction action) final;
  void Tick() {}

private:
  void SendPacket(const StenoPloverHidPacket &packet);
};

//---------------------------------------------------------------------------
