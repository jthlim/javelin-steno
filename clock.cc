//---------------------------------------------------------------------------

#include "clock.h"

//---------------------------------------------------------------------------

#if RUN_TESTS

uint64_t Clock::currentMicroseconds = 0;

#else

[[gnu::weak]]
void Clock::Sleep(uint32_t milliseconds) {
  const uint32_t startTime = Clock::GetMicroseconds();
  const uint32_t microseconds = 1000 * milliseconds;
  while (Clock::GetMicroseconds() - startTime < microseconds) {
    // Busy wait.
  }
}

#endif

//---------------------------------------------------------------------------
