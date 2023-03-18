//---------------------------------------------------------------------------

#include "repeat.h"
#include "../clock.h"
#include "../console.h"

//---------------------------------------------------------------------------

void StenoRepeat::Process(const StenoKeyState &value, StenoAction action) {
  next->Process(value, action);

  switch (action) {
  case StenoAction::PRESS:
    isRepeating = false;
    wasLastEventAPress = true;
    pressedKeyState = value;

    pressTime = Clock::GetCurrentTime();

    if (pressTime - releaseTime > FORGET_TIME_SPAN) {
      releasedKeyState.Reset();
    } else if (pressedKeyState == releasedKeyState) {
      isRepeating = true;
      nextTriggerTime = pressTime + INITIAL_REPEAT_DELAY;
    }
    break;

  case StenoAction::RELEASE:
    isRepeating = false;
    if (wasLastEventAPress) {
      wasLastEventAPress = false;
      // Capture the stroke before the release.
      releasedKeyState = pressedKeyState;
      releaseTime = Clock::GetCurrentTime();
    }
    break;

  case StenoAction::CANCEL:
    isRepeating = false;
    wasLastEventAPress = false;
    releasedKeyState.Reset();
    break;

  case StenoAction::TRIGGER:
    break;
  }
}

void StenoRepeat::Tick() {
  next->Tick();

  if (!isRepeating) {
    return;
  }

  uint32_t now = Clock::GetCurrentTime();
  if (now - nextTriggerTime >= (uint32_t)-INITIAL_REPEAT_DELAY) {
    return;
  }

  nextTriggerTime = now + REPEAT_DELAY;
  next->Process(pressedKeyState, StenoAction::TRIGGER);
  now = Clock::GetCurrentTime();
  if (nextTriggerTime < now + REPEAT_DELAY_MINIMUM) {
    nextTriggerTime = now + REPEAT_DELAY_MINIMUM;
  }
}

void StenoRepeat::PrintInfo() const {
  Console::Printf("  Repeat\n");
  next->PrintInfo();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"
#include "fake_processor.h"

TEST_BEGIN("Repeat tests") {
  FakeStenoProcessor fakeProcessor;
  StenoRepeat repeat(fakeProcessor);
  StenoProcessor processor(repeat);

  processor.Process(StenoKey::KL, true);
  processor.Process(StenoKey::A, true);
  processor.Process(StenoKey::TR, true);
  const StenoKeyState katKeyState = processor.GetCurrentKeyState();

  repeat.Tick();
  assert(fakeProcessor.triggers.size() == 0);

  processor.Process(StenoKey::A, false);
  repeat.Tick();
  assert(fakeProcessor.triggers.size() == 0);

  processor.Process(StenoKey::A, true);
  repeat.Tick();
  assert(fakeProcessor.triggers.size() == 0);

  Clock::AdvanceTime(100);
  repeat.Tick();
  assert(fakeProcessor.triggers.size() == 0);

  Clock::AdvanceTime(100);
  repeat.Tick();
  assert(fakeProcessor.triggers.size() == 1);
  assert(fakeProcessor.triggers[0] == katKeyState);

  Clock::AdvanceTime(20);
  repeat.Tick();
  assert(fakeProcessor.triggers.size() == 1);

  Clock::AdvanceTime(20);
  repeat.Tick();
  assert(fakeProcessor.triggers.size() == 2);
  assert(fakeProcessor.triggers[0] == katKeyState);
}
TEST_END

//---------------------------------------------------------------------------
