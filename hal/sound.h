//---------------------------------------------------------------------------

#pragma once
#include "../uint16.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

struct SoundWaveformData {
  uint16_t format;
  uint16_t sampleRate;
  uint32_t length;
  uint8_t data[0];
};

struct SoundSequenceData {
  Uint16 data;

  static constexpr int END_NOTE = 0;
  static constexpr int PAUSE_NOTE = 1;

  //  Notes:
  //   0 means end of sequence.
  //   1 means pause.
  //   2+ is note, with middle C as 50, in semitone steps
  uint8_t GetNote() const { return data.b[0] & 127; }

  uint32_t GetDelayInMs() const { return 10 * (data.ToUint32() >> 7); }
};

class Sound {
public:
  static void Stop();
  static void PlayFrequency(uint32_t frequency);

  // Middle C (C4) is 50.
  // A4 is 59
  // Takes input up to 127
  static uint32_t CalculateFrequencyForNote(int note);

  static void PlaySequence(const SoundSequenceData *data);

  static void PlayWaveform(const SoundWaveformData *data);
};

//---------------------------------------------------------------------------
