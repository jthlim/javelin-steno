//---------------------------------------------------------------------------

#include "processor.h"

//---------------------------------------------------------------------------

struct StenoPloverHidPacket {
  StenoPloverHidPacket(const StenoKeyState &state);

  uint8_t data[8];
};

//---------------------------------------------------------------------------

class StenoPloverHid final : public StenoProcessorElement {
public:
  void Process(const StenoKeyState &value, StenoAction action);
  void PrintInfo() const;

private:
  size_t counter = 0;

  static void SendPacket(const StenoPloverHidPacket &packet);
};

//---------------------------------------------------------------------------
