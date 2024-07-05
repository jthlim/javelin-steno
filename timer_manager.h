//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------
// Script timers should use values 0 upwards.
// Negative values are reserved for internal use.
//---------------------------------------------------------------------------

class TimerManager {
public:
  void PrintInfo() const;

  bool HasTimers() const { return timerCount != 0; }
  size_t GetTimerCount() const { return timerCount; }
  int GetNextTimerTriggerDelay(uint32_t currentTime) const;
  bool HasOnlyRepeatingTimers() const;
  int GetTimersGCD() const;
  bool HasTimer(intptr_t timerId) const {
    return GetTimerIndex(timerId) != INVALID_TIMER_INDEX;
  }

  // A previously matched timerId will have its destructor called.
  // If the timer can't be started, destructor will immediately be called on
  // context.
  void StartTimer(intptr_t timerId, uint32_t interval, bool isRepeating,
                  void (*handler)(intptr_t id, void *context), void *context,
                  void (*destructor)(void *context), uint32_t currentTime);

  // Removes the timer from the list and calls its destructor.
  void StopTimer(intptr_t timerId, uint32_t currentTime);

  void ProcessTimers(uint32_t currentTime);

  static void NoopDestructor(void *) {}

  static TimerManager instance;

private:
  TimerManager() = default;

  struct Timer {
    bool isRepeating;
    intptr_t id;
    uint32_t lastUpdateTime;
    uint32_t interval;
    void (*handler)(intptr_t id, void *context);
    void *context;
    void (*destructor)(void *context);

    void destroy() { (*destructor)(context); }

    int GetTriggerDelay(uint32_t currentTime) const {
      // Since time steps can be backwards in case of receiving delayed input
      // from a split pair, it's important that this returns a signed value.
      return lastUpdateTime + interval - currentTime;
    }
    bool ShouldTrigger(uint32_t currentTime) const {
      return GetTriggerDelay(currentTime) <= 0;
    }
  };

  static const size_t MAXIMUM_TIMER_COUNT = 16;
  static const size_t INVALID_TIMER_INDEX = (size_t)-1;

  uint8_t timerCount;
  int tickDelay = 1;
  uint32_t lastUpdateTime;
  Timer timers[MAXIMUM_TIMER_COUNT];

  size_t GetTimerIndex(intptr_t timerId) const;
  void RemoveTimerIndex(size_t index, uint32_t currentTime);

  void OnTimersUpdated(uint32_t currentTime);

  // Interception point for controlling update frequency.
  // 0 = interrupt only
  void UpdateTickDelay(int ms);
};

//---------------------------------------------------------------------------
