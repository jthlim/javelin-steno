//---------------------------------------------------------------------------

#include "first_up.h"

//---------------------------------------------------------------------------

void StenoFirstUp::Process(StenoKeyState value, StenoAction action) {
  nextProcessor.Process(value, action);

  switch (action) {
  case StenoAction::PRESS:
    wasLastEventAPress = true;
    pressedKeyState = value;
    break;

  case StenoAction::RELEASE:
    if (wasLastEventAPress) {
      wasLastEventAPress = false;
      // Trigger the chord from before the release.
      nextProcessor.Process(pressedKeyState, StenoAction::TRIGGER);
    }
    break;

  case StenoAction::CANCEL:
    wasLastEventAPress = false;
    break;

  case StenoAction::TRIGGER:
    break;
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "fake_processor.h"
#include "../unit_test.h"

TEST_BEGIN("FirstUp tests") {
  FakeStenoProcessor fakeProcessor;
  StenoFirstUp firstUp(fakeProcessor);
  StenoProcessor processor(firstUp);

  processor.Process(StenoKey::KL, true);
  processor.Process(StenoKey::A, true);
  processor.Process(StenoKey::TR, true);
  assert(fakeProcessor.triggers.size() == 0);

  processor.Process(StenoKey::A, false);
  assert(fakeProcessor.triggers.size() == 1);

  processor.Process(StenoKey::A, true);
  assert(fakeProcessor.triggers.size() == 1);

  processor.Process(StenoKey::KL, false);
  assert(fakeProcessor.triggers.size() == 2);

  processor.Process(StenoKey::A, false);
  processor.Process(StenoKey::TR, false);
  assert(fakeProcessor.triggers.size() == 2);
}
TEST_END

//---------------------------------------------------------------------------
