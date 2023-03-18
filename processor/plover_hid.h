//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

class StenoPloverHid final : public StenoProcessorElement {
public:
  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  size_t counter = 0;

  void SendPacket(const StenoPloverHidPacket &packet);
};

//---------------------------------------------------------------------------
