//---------------------------------------------------------------------------

#include "wpm_tracker.h"
#include "clock.h"
#include "mem.h"

//---------------------------------------------------------------------------

WpmTracker WpmTracker::instance;

//---------------------------------------------------------------------------

void WpmTracker::Tally(int count) {
  const uint32_t now = Clock::GetMilliseconds() / 1000;
  UpdateToNow(now);

  charactersTyped[now % NUMBER_OF_SECONDS] += count;
}

int WpmTracker::GetWpm(int seconds) {
  const uint32_t now = Clock::GetMilliseconds() / 1000;
  UpdateToNow(now);

  if (seconds > NUMBER_OF_SECONDS) {
    seconds = NUMBER_OF_SECONDS;
  }

  int tally = 0;
  uint32_t bucket = now;
  for (int i = seconds; i > 0; --i) {
    // Start one bucket earlier to avoid this second influencing the score too
    // much.
    --bucket;

    // Use a linear weighting to prioritize more recent speed.
    tally += i * charactersTyped[bucket % NUMBER_OF_SECONDS];
  }
  if (tally <= 0) {
    return 0;
  }
  return tally * (2 * 60 / 5) / (seconds * (seconds + 1));
}

void WpmTracker::UpdateToNow(uint32_t now) {
  const int delta = now - lastTime;
  if (delta >= 0) {
    if (delta >= NUMBER_OF_SECONDS) {
      Mem::Clear(charactersTyped);
    } else {
      uint32_t bucket = now;
      for (int i = 0; i < delta; ++i) {
        charactersTyped[bucket % NUMBER_OF_SECONDS] = 0;
        --bucket;
      }
    }
  }
  lastTime = now;
}

//---------------------------------------------------------------------------
