//---------------------------------------------------------------------------

#include "sound.h"
#include "../base64.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

uint32_t Sound::CalculateFrequencyForNote(int note) {
  // These frequencies are for note values 116-127;
  static const uint16_t FREQUENCIES[] = {
      11840, 12544, 13290, 14080, 14917, 15804,
      16744, 17740, 18795, 19912, 21096, 22351,
  };

  const int index = (note + 4) % 12;
  const int octaveShift = (127 - note) / 12;
  return FREQUENCIES[index] >> octaveShift;
}

//---------------------------------------------------------------------------

void Sound::AddConsoleCommands(Console &console) {
#if JAVELIN_SOUND
  console.RegisterCommand("play_frequency", "Plays the specified frequency",
                          &PlayFrequency_Binding, nullptr);
  console.RegisterCommand("play_sequence", "Plays the specified sequence",
                          &PlaySequence_Binding, nullptr);
  console.RegisterCommand("stop_sound", "Stops all sound playing",
                          &StopSound_Binding, nullptr);
#endif
}

void Sound::PlayFrequency_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR No frequency specified\n\n");
    return;
  }
  int frequency;
  p = Str::ParseInteger(&frequency, p + 1);
  if (!p || *p != '\0') {
    Console::Printf("ERR No frequency specified\n\n");
    return;
  }

  PlayFrequency(frequency);
  Console::SendOk();
}

void Sound::PlaySequence_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing data\n\n");
    return;
  }

  static uint8_t decodeBuffer[256];
  const size_t byteCount = Base64::Decode(decodeBuffer, (const uint8_t *)p);

  if (byteCount == 0) {
    Console::Printf("ERR No data\n\n");
    return;
  }
  decodeBuffer[byteCount] = 0;

  PlaySequence((const SoundSequenceData *)decodeBuffer);
  Console::SendOk();
}

void Sound::StopSound_Binding(void *context, const char *commandLine) {
  Stop();
  Console::SendOk();
}

//---------------------------------------------------------------------------

__attribute__((weak)) void Sound::Stop() {}
__attribute__((weak)) void Sound::PlayFrequency(uint32_t frequency) {}
__attribute__((weak)) void Sound::PlaySequence(const SoundSequenceData *data) {}
__attribute__((weak)) void
Sound::PlayWaveform(const uint8_t *data, size_t length, uint32_t sampleRate) {}

//---------------------------------------------------------------------------

#include "../unit_test.h"

TEST_BEGIN("Sound::CalculateFrequencyForNote tests") {
  assert(Sound::CalculateFrequencyForNote(50) == 261);
  assert(Sound::CalculateFrequencyForNote(59) == 440);
}
TEST_END

//---------------------------------------------------------------------------
