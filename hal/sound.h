//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class Sound {
public:
  static void Stop();
  static void PlayFrequency(uint32_t frequency);

  // Sequence data:
  //  Byte 0: Note from C-0 (2) to B-9 (121). 0 means end of sequence. 1 means
  //  pause. Byte 1: Delay in 10ms intervals.
  static void PlaySequence(const uint8_t *data);

  static void PlayWaveform(const uint8_t *data, size_t length,
                           uint32_t sampleRate);
};

//---------------------------------------------------------------------------
