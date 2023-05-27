//---------------------------------------------------------------------------

#pragma once
#include "clock.h"
#include <stdint.h>

//---------------------------------------------------------------------------

#if !defined(JAVELIN_DEBOUNCE_MS)
#define JAVELIN_DEBOUNCE_MS 5
#endif

template <typename T> struct Debounced {
public:
  Debounced(bool isUpdated, T value) : isUpdated(isUpdated), value(value) {}

  bool isUpdated;
  T value;
};

//---------------------------------------------------------------------------

// Only triggers a key after new state has been stable for DEBOUNCE_DELAY_MS.
template <typename T> class GlobalDeferredDebounce {
public:
  GlobalDeferredDebounce() {}

  GlobalDeferredDebounce(T initialState)
      : lastDebouncedState(initialState), lastState(initialState) {}

  Debounced<T> Update(T input);
  void Set(T input) {
    lastState = input;
    lastDebouncedState = input;
    lastStateTime = Clock::GetMilliseconds();
  }

  const T &GetCurrentState() const { return lastState; }
  const T &GetDebouncedState() const { return lastDebouncedState; }

private:
  T lastDebouncedState;
  T lastState;
  uint32_t lastStateTime = 0;

  static const uint32_t DEBOUNCE_DELAY_MS = JAVELIN_DEBOUNCE_MS;
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

template <typename T> Debounced<T> GlobalDeferredDebounce<T>::Update(T input) {
  uint32_t currentTime = Clock::GetMilliseconds();

  if (input != lastState) {
    lastState = input;
    lastStateTime = currentTime;
    return Debounced<T>(false, lastDebouncedState);
  }

  if (input == lastDebouncedState) {
    lastState = input;
    return Debounced<T>(false, lastDebouncedState);
  }

  if (currentTime - lastStateTime < DEBOUNCE_DELAY_MS) {
    return Debounced<T>(false, lastDebouncedState);
  }

  lastDebouncedState = lastState;
  lastStateTime = currentTime;
  return Debounced<T>(true, lastDebouncedState);
}

//---------------------------------------------------------------------------
