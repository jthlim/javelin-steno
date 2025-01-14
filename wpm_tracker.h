//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

// Tracks 60s wpm and 10s wpm.
class WpmTracker {
public:
  void Tally(int count);

  int GetWpm(int seconds);
  int Get5sWpm() { return GetWpm(5); }
  int Get10sWpm() { return GetWpm(10); }
  int Get60sWpm() { return GetWpm(60); }

  static WpmTracker instance;

private:
  static constexpr size_t NUMBER_OF_SECONDS = 64;

  uint32_t lastTime;
  int charactersTyped[NUMBER_OF_SECONDS];

  void UpdateToNow(uint32_t now);
};

//---------------------------------------------------------------------------
