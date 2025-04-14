//---------------------------------------------------------------------------

#include "timer_manager.h"
#include "console.h"

//---------------------------------------------------------------------------

TimerManager TimerManager::instance;

//---------------------------------------------------------------------------

void TimerManager::PrintInfo() const {
  Console::Printf("Timers\n");
  Console::Printf("  Last updated: %u ms\n", lastUpdateTime);
  Console::Printf("  Tick: %d ms\n", tickDelay);
  for (size_t i = 0; i < timerCount; ++i) {
    const Timer &timer = timers[i];
    Console::Printf("    id %d: %ums%s, updated: %u ms\n", (int)timer.id,
                    timer.interval, timer.isRepeating ? " (repeating)" : "",
                    timer.lastUpdateTime);
  }
}

//---------------------------------------------------------------------------

void TimerManager::OnTimersUpdated(uint32_t currentTime) {
  int newTickDelay;
  if (!HasTimers()) {
    newTickDelay = 0;
  } else if (HasOnlyRepeatingTimers()) {
    newTickDelay = GetTimersGCD();
    if (newTickDelay < 1) {
      newTickDelay = 1;
    }
  } else {
    newTickDelay = GetNextTimerTriggerDelay(currentTime);
    if (newTickDelay < 1) {
      newTickDelay = 1;
    }
  }

  if (newTickDelay == tickDelay) {
    return;
  }
  tickDelay = newTickDelay;
  UpdateTickDelay(tickDelay);
}

[[gnu::weak]] void TimerManager::UpdateTickDelay(int ms) {}

size_t TimerManager::GetTimerIndex(int32_t timerId) const {
  for (size_t i = 0; i < timerCount; ++i) {
    if (timers[i].id == timerId) {
      return i;
    }
  }
  return INVALID_TIMER_INDEX;
}

void TimerManager::StopTimer(int32_t timerId, uint32_t currentTime) {
  const size_t index = GetTimerIndex(timerId);
  if (index == INVALID_TIMER_INDEX) {
    return;
  }

  timers[index].handler->OnTimerRemovedFromManager();
  RemoveTimerIndex(index, currentTime);
}

void TimerManager::StartTimer(int32_t timerId, uint32_t interval,
                              bool isRepeating, TimerHandler *handler,
                              uint32_t currentTime) {
  size_t index = GetTimerIndex(timerId);
  if (index == INVALID_TIMER_INDEX) {
    if (timerCount >= MAXIMUM_TIMER_COUNT) {
      return;
    }
    index = timerCount++;
    timers[index].id = timerId;
  } else {
    timers[index].handler->OnTimerRemovedFromManager();
  }

  timers[index].isRepeating = isRepeating;
  timers[index].lastUpdateTime = currentTime;
  timers[index].interval = interval;
  timers[index].handler = handler;

  OnTimersUpdated(currentTime);
}

void TimerManager::ProcessTimers(uint32_t currentTime) {
  lastUpdateTime = currentTime;

  for (size_t i = 0; i < timerCount; ++i) {
    Timer &timer = timers[i];
    if (timer.ShouldTrigger(currentTime)) {
      timer.lastUpdateTime = currentTime;

      const intptr_t id = timer.id;
      TimerHandler *const handler = timer.handler;
      const bool isRepeating = timer.isRepeating;

      // For non-repeating timers, need to remove it immediately so that
      // if the timer script adds it back, it triggers again.
      if (!isRepeating) {
        RemoveTimerIndex(i, currentTime);
        --i;
      }

      handler->Run(id);

      if (!isRepeating) {
        handler->OnTimerRemovedFromManager();
      }
    }
  }
}

void TimerManager::IterateTimers(void *context,
                                 void (*callback)(void *context, int id,
                                                  TimerHandler *handler)) {
  for (size_t i = timerCount; i != 0;) {
    --i;
    Timer &timer = timers[i];

    callback(context, timer.id, timer.handler);
  }
}

void TimerManager::RemoveScriptTimers(uint32_t currentTime) {
  for (size_t i = timerCount; i != 0;) {
    --i;
    if (timers[i].id >= 0) {
      timers[i].handler->OnTimerRemovedFromManager();
      --timerCount;
      if (i != timerCount) {
        timers[i] = timers[timerCount];
      }
    }
  }
  OnTimersUpdated(currentTime);
}

void TimerManager::RemoveTimerIndex(size_t index, uint32_t currentTime) {
  --timerCount;
  if (index != timerCount) {
    timers[index] = timers[timerCount];
  }
  OnTimersUpdated(currentTime);
}

int TimerManager::GetNextTimerTriggerDelay(uint32_t currentTime) const {
  int result = INT32_MAX;
  for (size_t i = 0; i < timerCount; ++i) {
    const int delay = timers[i].GetTriggerDelay(currentTime);
    if (delay < result) {
      result = delay;
    }
  }
  return result;
}

bool TimerManager::HasOnlyRepeatingTimers() const {
  for (size_t i = 0; i < timerCount; ++i) {
    if (!timers[i].isRepeating) {
      return false;
    }
  }
  return true;
}

// Stein's algorithm.
static int GCD(int a, int b) {
  if (a == 0) {
    return b;
  }
  if (b == 0) {
    return a;
  }

  const int commonPowerOf2 = __builtin_ctz(a | b);

  a >>= __builtin_ctz(a);

  do {
    b >>= __builtin_ctz(b);

    if (a > b) {
      const int temp = a;
      a = b;
      b = temp;
    }

    b -= a;
  } while (b != 0);

  return a << commonPowerOf2;
}

int TimerManager::GetTimersGCD() const {
  if (timerCount == 0) {
    return -1;
  }

  int result = timers[0].interval;
  for (size_t i = 1; i < timerCount; ++i) {
    result = GCD(result, timers[i].interval);
  }
  return result;
}

//---------------------------------------------------------------------------
