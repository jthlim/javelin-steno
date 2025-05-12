//---------------------------------------------------------------------------

#include "sound.h"

//---------------------------------------------------------------------------

uint32_t Sound::CalculateFrequencyForNote(int note) {
  // These frequencies are for note values 116-127;
  static constexpr uint16_t FREQUENCIES[] = {
      11840, 12544, 13290, 14080, 14917, 15804,
      16744, 17740, 18795, 19912, 21096, 22351,
  };

  const int index = (note + 4) % 12;
  const int octaveShift = (127 - note) / 12;
  return FREQUENCIES[index] >> octaveShift;
}

//---------------------------------------------------------------------------

[[gnu::weak]] void Sound::Stop() {}
[[gnu::weak]] void Sound::PlayFrequency(uint32_t frequency) {}
[[gnu::weak]] void Sound::PlaySequence(const SoundSequenceData *data) {}
[[gnu::weak]] void Sound::PlayWaveform(const uint8_t *data, size_t length,
                                       uint32_t sampleRate) {}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"

TEST_BEGIN("Sound::CalculateFrequencyForNote tests") {
  assert(Sound::CalculateFrequencyForNote(50) == 261);
  assert(Sound::CalculateFrequencyForNote(59) == 440);
}
TEST_END

//---------------------------------------------------------------------------
