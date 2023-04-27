//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

#if RUN_TESTS

class Clock {
public:
  static void Initialize() {}

  static uint32_t GetCurrentTime() { return currentTime; }
  static uint32_t GetMicroseconds();

  static void AdvanceTime(uint32_t amount) { currentTime += amount; }

private:
  static uint32_t currentTime;
};

#else

class Clock {
public:
  static void Initialize(); // Only implemented on platforms that need it.

  static uint32_t GetCurrentTime();
  static uint32_t GetMicroseconds();

  static void AdvanceTime(uint32_t amount) {}
};

#endif

//---------------------------------------------------------------------------
