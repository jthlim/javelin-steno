//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

struct SoundSequenceData {
  uint8_t data[2];

  static const int END_NOTE = 0;
  static const int PAUSE_NOTE = 1;

  //  Notes:
  //   0 means end of sequence.
  //   1 means pause.
  //   2+ is note, with middle C as 50, in semitone steps
  uint8_t GetNote() const { return data[0] & 127; }

  uint32_t GetDelayInMs() const {
    return 10 * ((data[0] | (data[1] << 8)) >> 7);
  }
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

  static void PlayWaveform(const uint8_t *data, size_t length,
                           uint32_t sampleRate);

  static void AddConsoleCommands(Console &console);

private:
  static void PlayFrequency_Binding(void *context, const char *commandLine);
  static void PlaySequence_Binding(void *context, const char *commandLine);
  static void StopSound_Binding(void *context, const char *commandLine);
};

//---------------------------------------------------------------------------
