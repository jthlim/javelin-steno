//---------------------------------------------------------------------------

#pragma once
#include "clock.h"
#include <stdint.h>

//---------------------------------------------------------------------------

template <typename T> struct Debounced {
public:
  Debounced(bool isUpdated, T value) : isUpdated(isUpdated), value(value) {}

  bool isUpdated;
  T value;
};

//---------------------------------------------------------------------------

// Any changes in key state and stable for DEBOUNCE_DELAY_MS before
template <typename T> class GlobalDeferredDebounce {
public:
  GlobalDeferredDebounce(T initialState)
      : lastDebouncedState(initialState), lastState(initialState) {}

  Debounced<T> Update(T input);

private:
  T lastDebouncedState;
  T lastState;
  uint32_t lastStateTime = 0;

  static const uint32_t DEBOUNCE_DELAY_MS = 5;
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

template <typename T> Debounced<T> GlobalDeferredDebounce<T>::Update(T input) {
  uint32_t currentTime = Clock::GetCurrentTime();

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
