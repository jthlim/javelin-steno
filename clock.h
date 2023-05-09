//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

#if RUN_TESTS

class Clock {
public:
  static uint32_t GetMilliseconds() {
    return (uint32_t)(currentMicroseconds / 1000);
  }
  static uint32_t GetMicroseconds() { return (uint32_t)currentMicroseconds; }

  static void AdvanceMilliseconds(uint32_t amount) {
    currentMicroseconds += 1000LL * amount;
  }
  static void AdvanceMicroseconds(uint32_t amount) {
    currentMicroseconds += amount;
  }

private:
  static uint64_t currentMicroseconds;
};

#else

class Clock {
public:
  static uint32_t GetMilliseconds();
  static uint32_t GetMicroseconds();

  static void AdvanceMilliseconds(uint32_t amount) {}
};

#endif

//---------------------------------------------------------------------------
