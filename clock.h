//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

#if RUN_TESTS

class Clock {
public:
  static uint32_t GetCurrentTime() { return currentTime; }

  static void AdvanceTime(uint32_t amount) { currentTime += amount; }

private:
  static uint32_t currentTime;
};

#else

class Clock {
public:
  static uint32_t GetCurrentTime();
  static void AdvanceTime(uint32_t amount) {}
};

#endif

//---------------------------------------------------------------------------
