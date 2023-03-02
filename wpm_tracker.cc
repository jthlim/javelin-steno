//---------------------------------------------------------------------------

#include "wpm_tracker.h"
#include "clock.h"
#include <string.h>

//---------------------------------------------------------------------------

WpmTracker WpmTracker::instance;

//---------------------------------------------------------------------------

void WpmTracker::Tally(int count) {
  uint32_t now = Clock::GetCurrentTime() / 1000;
  UpdateToNow(now);

  charactersTyped[now % NUMBER_OF_SECONDS] += count;
}

int WpmTracker::GetWpm(int seconds) {
  uint32_t now = Clock::GetCurrentTime() / 1000;
  UpdateToNow(now);

  int tally = 0;
  uint32_t bucket = now;
  for (int i = 0; i < seconds; ++i) {
    // Start one bucket earlier to avoid this second influencing the score too
    // much.
    --bucket;
    tally += charactersTyped[bucket % NUMBER_OF_SECONDS];
  }
  if (tally < 0) {
    return 0;
  }
  return tally * (60 / 5) / seconds;
}

void WpmTracker::UpdateToNow(uint32_t now) {
  int delta = now - lastTime;
  if (delta >= 0) {
    if (delta >= NUMBER_OF_SECONDS) {
      memset(charactersTyped, 0, sizeof(charactersTyped));
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
