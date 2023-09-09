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

__attribute__((weak)) void TimerManager::UpdateTickDelay(int ms) {}

size_t TimerManager::GetTimerIndex(intptr_t timerId) const {
  for (size_t i = 0; i < timerCount; ++i) {
    if (timers[i].id == timerId) {
      return i;
    }
  }
  return INVALID_TIMER_INDEX;
}

void *TimerManager::StopTimer(intptr_t timerId, uint32_t currentTime) {
  size_t index = GetTimerIndex(timerId);
  if (index == INVALID_TIMER_INDEX) {
    return nullptr;
  }
  void *result = timers[index].context;
  RemoveTimerIndex(index, currentTime);
  return result;
}

void *TimerManager::StartTimer(intptr_t timerId, uint32_t interval,
                               bool isRepeating,
                               void (*handler)(intptr_t, void *), void *context,
                               uint32_t currentTime) {
  size_t index = GetTimerIndex(timerId);
  void *result = nullptr;
  if (index == INVALID_TIMER_INDEX) {
    if (timerCount >= MAXIMUM_TIMER_COUNT) {
      return nullptr;
    }
    index = timerCount++;
    timers[index].id = timerId;
  } else {
    result = timers[index].context;
  }

  timers[index].isRepeating = isRepeating;
  timers[index].lastUpdateTime = currentTime;
  timers[index].interval = interval;
  timers[index].handler = handler;
  timers[index].context = context;

  OnTimersUpdated(currentTime);

  return result;
}

void TimerManager::ProcessTimers(uint32_t currentTime) {
  lastUpdateTime = currentTime;

  for (size_t i = 0; i < timerCount; ++i) {
    Timer &timer = timers[i];
    if (timer.ShouldTrigger(currentTime)) {
      timer.lastUpdateTime = currentTime;

      intptr_t id = timer.id;
      void (*handler)(intptr_t, void *) = timer.handler;
      void *context = timer.context;

      // For non-repeating timers, need to remove it immediately so that
      // if the timer script adds it back, it triggers again.
      if (!timer.isRepeating) {
        RemoveTimerIndex(i, currentTime);
        --i;
      }

      handler(id, context);
    }
  }
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
    int delay = timers[i].GetTriggerDelay(currentTime);
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

  int commonPowerOf2 = __builtin_ctz(a | b);

  a >>= __builtin_ctz(a);

  do {
    b >>= __builtin_ctz(b);

    if (a > b) {
      int temp = a;
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
