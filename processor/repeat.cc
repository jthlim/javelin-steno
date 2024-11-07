//---------------------------------------------------------------------------

#include "repeat.h"
#include "../clock.h"
#include "../console.h"

//---------------------------------------------------------------------------

const int TIMER_ID = -(('R' << 24) | ('E' << 16) | ('P' << 8) | 'T');

//---------------------------------------------------------------------------

void StenoRepeat::Process(const StenoKeyState &value, StenoAction action) {
  super::Process(value, action);

  switch (action) {
  case StenoAction::PRESS:
    if (isRepeating) {
      TimerManager::instance.StopTimer(TIMER_ID, Clock::GetMilliseconds());
    }
    isRepeating = false;
    wasLastEventAPress = true;
    pressedKeyState = value;

    pressTime = Clock::GetMilliseconds();

    if (pressTime - releaseTime > FORGET_TIME_SPAN) {
      releasedKeyState.Reset();
    } else if (pressedKeyState == releasedKeyState) {
      isRepeating = true;
      TimerManager::instance.StartTimer(TIMER_ID, INITIAL_REPEAT_DELAY, false,
                                        this, Clock::GetMilliseconds());
    }
    break;

  case StenoAction::RELEASE:
    if (isRepeating) {
      TimerManager::instance.StopTimer(TIMER_ID, Clock::GetMilliseconds());
    }
    isRepeating = false;
    if (wasLastEventAPress) {
      wasLastEventAPress = false;
      // Capture the stroke before the release.
      releasedKeyState = pressedKeyState;
      releaseTime = Clock::GetMilliseconds();
    }
    break;

  case StenoAction::CANCEL_KEY:
  case StenoAction::CANCEL_ALL:
    if (isRepeating) {
      TimerManager::instance.StopTimer(TIMER_ID, Clock::GetMilliseconds());
    }
    isRepeating = false;
    wasLastEventAPress = false;
    releasedKeyState.Reset();
    break;

  case StenoAction::TRIGGER:
    break;
  }
}

void StenoRepeat::Run(intptr_t id) {
  super::Process(pressedKeyState, StenoAction::TRIGGER);
  TimerManager::instance.StartTimer(TIMER_ID, REPEAT_DELAY, false, this,
                                    TimerManager::instance.GetLastUpdateTime());
}

void StenoRepeat::PrintInfo() const {
  Console::Printf("  Repeat\n");
  super::PrintInfo();
}

//---------------------------------------------------------------------------
