//---------------------------------------------------------------------------

#include "switch.h"

//---------------------------------------------------------------------------

const StenoKeyState switchTrigger((1ULL << (int) StenoKey::S1) |
                                  (1ULL << (int) StenoKey::NUM1) |
                                  (1ULL << (int) StenoKey::NUM2) |
                                  (1ULL << (int) StenoKey::DR));

//---------------------------------------------------------------------------

void StenoSwitch::Process(StenoKeyState value, StenoAction action) {
  if (action == StenoAction::TRIGGER && value == switchTrigger) {
    useDefault = !useDefault;
    return;
  }

  StenoProcessorElement &nextProcessor =
      useDefault ? defaultProcessor : alternateProcessor;
  nextProcessor.Process(value, action);
}

void StenoSwitch::Tick() {
  StenoProcessorElement &nextProcessor =
      useDefault ? defaultProcessor : alternateProcessor;
  nextProcessor.Tick();
}

//---------------------------------------------------------------------------
