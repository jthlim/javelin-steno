//---------------------------------------------------------------------------

#include "sound.h"

//---------------------------------------------------------------------------

__attribute__((weak)) void Sound::Stop() {}
__attribute__((weak)) void Sound::PlayFrequency(uint32_t frequency) {}
__attribute__((weak)) void Sound::PlaySequence(const uint8_t *data) {}
__attribute__((weak)) void
Sound::PlayWaveform(const uint8_t *data, size_t length, uint32_t sampleRate) {}

//---------------------------------------------------------------------------
