//---------------------------------------------------------------------------

#include "user_dictionary.h"
#include "../chord.h"
#include "../console.h"
#include "../crc32.h"
#include "../flash.h"
#include "../str.h"
#include <assert.h>

//---------------------------------------------------------------------------

// These are chosen to minimize flash erase cycles.
constexpr size_t OFFSET_EMPTY = 0xffffffff;
constexpr size_t OFFSET_DELETED = 0;
constexpr size_t OFFSET_DATA = 1;

static const uint32_t USER_DICTIONARY_MAGIC = 0x4455534a; // 'JSUD'
static const uint32_t USER_DICTIONARY_VERSION = 1;

// Offset within the flash page where descriptors will be stored.
const size_t DESCRIPTOR_OFFSET = 64;

static_assert(sizeof(StenoUserDictionaryDescriptor) <= DESCRIPTOR_OFFSET,
              "Descriptor size is larger than expected");

//---------------------------------------------------------------------------

struct StenoUserDictionaryEntry {
  uint32_t chordLength;
  StenoChord chords[1];
  // After chords is a null terminated string.

  void Print(char *buffer) const;
  char *GetText() const { return (char *)(chords + chordLength); }
};

//---------------------------------------------------------------------------

template <typename T> T *RoundToPage(T *p, size_t pageSize) {
  intptr_t i = (intptr_t)p;
  i &= -pageSize;
  return (T *)i;
}

//---------------------------------------------------------------------------

bool StenoUserDictionaryDescriptor::IsValid(
    const StenoUserDictionaryData &layout) const {
  return magic == USER_DICTIONARY_MAGIC && version == USER_DICTIONARY_VERSION &&
         data.hashTable == layout.hashTable &&
         data.hashTableSize == layout.hashTableSize &&
         data.dataBlock == layout.dataBlock &&
         Crc32(&data, sizeof(data)) == crc32;
}

void StenoUserDictionaryDescriptor::UpdateCrc32() {
  crc32 = Crc32(&data, sizeof(data));
}

//---------------------------------------------------------------------------

StenoUserDictionary::StenoUserDictionary(const StenoUserDictionaryData &layout)
    : descriptorBase(layout.GetDescriptor()), layout(layout) {
  activeDescriptor = FindMostRecentDescriptor();
  if (activeDescriptor == nullptr) {
    Reset();
  }
}

const StenoUserDictionaryDescriptor *
StenoUserDictionary::FindMostRecentDescriptor() const {
  const StenoUserDictionaryDescriptor *result = nullptr;

  for (size_t i = 0; i < Flash::BLOCK_SIZE; i += DESCRIPTOR_OFFSET) {
    const StenoUserDictionaryDescriptor *test =
        (const StenoUserDictionaryDescriptor *)((intptr_t)descriptorBase + i);

    if (test->IsValid(layout)) {
      if (!result) {
        result = test;
      } else {
        if (test->data.dataBlockSize > result->data.dataBlockSize) {
          result = test;
        }
      }
    }
  }

  return result;
}

size_t StenoUserDictionary::GetNextDescriptorToWriteOffset() const {
  size_t activeOffset = (intptr_t)activeDescriptor - (intptr_t)descriptorBase;
  size_t nextOffset = (activeOffset + DESCRIPTOR_OFFSET) % Flash::BLOCK_SIZE;
  return nextOffset;
}

StenoDictionaryLookupResult
StenoUserDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  if (lookup.length > activeDescriptor->data.maximumStrokeCount) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  size_t entryIndex = lookup.hash;
  for (;;) {
    entryIndex = entryIndex & (activeDescriptor->data.hashTableSize - 1);

    uint32_t offset = activeDescriptor->data.hashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return StenoDictionaryLookupResult::CreateInvalid();

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      if (entry->chordLength == lookup.length &&
          memcmp(lookup.chords, entry->chords,
                 sizeof(StenoChord) * lookup.length) == 0) {
        return StenoDictionaryLookupResult::CreateStaticString(
            entry->GetText());
      }
    }

    ++entryIndex;
  }
}

unsigned int StenoUserDictionary::GetMaximumMatchLength() const {
  return activeDescriptor->data.maximumStrokeCount;
}

//---------------------------------------------------------------------------

void StenoUserDictionary::Reset() {
  Flash::Erase(layout.hashTable, layout.hashTableSize * sizeof(uint32_t));

  StenoUserDictionaryDescriptor *freshDescriptor =
      (StenoUserDictionaryDescriptor *)malloc(Flash::BLOCK_SIZE);

  // Use 0xff rather than 0 to reduce flash I/O.
  memset(freshDescriptor, 0xff, Flash::BLOCK_SIZE);

  freshDescriptor->magic = USER_DICTIONARY_MAGIC;
  freshDescriptor->version = USER_DICTIONARY_VERSION;
  freshDescriptor->data.hashTable = layout.hashTable;
  freshDescriptor->data.hashTableSize = layout.hashTableSize;
  freshDescriptor->data.dataBlock = layout.dataBlock;
  freshDescriptor->data.dataBlockSize = 0;
  freshDescriptor->data.maximumStrokeCount = 0;
  freshDescriptor->UpdateCrc32();

  Flash::Write(descriptorBase, freshDescriptor, Flash::BLOCK_SIZE);

  free(freshDescriptor);
  activeDescriptor = descriptorBase;
}

bool StenoUserDictionary::Add(const StenoChord *chords, size_t length,
                              const char *word) {
  // Verify that it doesn't already exist.
  StenoDictionaryLookupResult lookup =
      Lookup(StenoDictionaryLookup(chords, length));

  if (lookup.IsValid() && Str::Eq(lookup.GetText(), word)) {
    lookup.Destroy();
    return true;
  }
  lookup.Destroy();

  AddToDataBlockResult data = AddToDataBlock(chords, (uint32_t)length, word);
  if (data.length == 0) {
    return false;
  }
  AddToDescriptor(length, data.length);
  return AddToHashTable(chords, length, data.offset);
}

StenoUserDictionary::AddToDataBlockResult
StenoUserDictionary::AddToDataBlock(const StenoChord *chords, uint32_t length,
                                    const char *word) {
  size_t wordLength = strlen(word);

  // Need to store null terminator + round up to nearest 4 bytes.
  size_t wordStorageLength = (wordLength + 4) & -4;

  size_t offset = activeDescriptor->data.dataBlockSize;

  size_t totalLength =
      sizeof(uint32_t) + sizeof(StenoChord) * length + wordStorageLength;

  // Safeguard...
  if (totalLength > 256 || activeDescriptor->data.dataBlockSize + totalLength >
                               layout.dataBlockSize) {
    return AddToDataBlockResult(0, 0);
  }

  // Allocate memory for two pages, since the data could span both.
  uint8_t *buffer = (uint8_t *)malloc(Flash::BLOCK_SIZE * 2);
  memset(buffer, 0xff, Flash::BLOCK_SIZE * 2);

  const uint8_t *originalData = activeDescriptor->data.dataBlock + offset;
  const uint8_t *originalDataPage =
      RoundToPage(originalData, Flash::BLOCK_SIZE);
  memcpy(buffer, originalDataPage, originalData - originalDataPage);

  uint8_t *p = &buffer[originalData - originalDataPage];
  memcpy(p, &length, sizeof(length));
  p += sizeof(length);
  memcpy(p, chords, sizeof(StenoChord) * length);
  p += sizeof(StenoChord) * length;
  memcpy(p, word, wordLength + 1);
  p += wordStorageLength;

  size_t bytesToWrite =
      ((p - buffer) + (Flash::BLOCK_SIZE - 1)) & -Flash::BLOCK_SIZE;

  assert(bytesToWrite <= Flash::BLOCK_SIZE * 2);
  Flash::Write(originalDataPage, buffer, bytesToWrite);

  free(buffer);

  return AddToDataBlockResult(offset, totalLength);
}

void StenoUserDictionary::AddToDescriptor(size_t chordLength,
                                          size_t dataLength) {
  char *buffer = (char *)malloc(Flash::BLOCK_SIZE);

  memcpy(buffer, descriptorBase, Flash::BLOCK_SIZE);

  size_t freshDescriptorOffset = GetNextDescriptorToWriteOffset();
  StenoUserDictionaryDescriptor *freshDescriptor =
      (StenoUserDictionaryDescriptor *)(buffer + freshDescriptorOffset);

  memcpy(freshDescriptor, activeDescriptor,
         sizeof(StenoUserDictionaryDescriptor));
  freshDescriptor->data.dataBlockSize =
      activeDescriptor->data.dataBlockSize + dataLength;
  if (chordLength > activeDescriptor->data.maximumStrokeCount) {
    freshDescriptor->data.maximumStrokeCount = (uint32_t)chordLength;
  }
  freshDescriptor->UpdateCrc32();

  Flash::Write(descriptorBase, buffer, Flash::BLOCK_SIZE);

  free(buffer);

  activeDescriptor =
      (StenoUserDictionaryDescriptor *)((intptr_t)descriptorBase +
                                        freshDescriptorOffset);
}

bool StenoUserDictionary::AddToHashTable(const StenoChord *chords,
                                         size_t length, size_t dataOffset) {
  size_t entryIndex = StenoChord::Hash(chords, length);

  for (int probeCount = 0; probeCount < 128; ++probeCount) {
    entryIndex = entryIndex & (activeDescriptor->data.hashTableSize - 1);

    uint32_t offset = activeDescriptor->data.hashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      WriteEntryIndex(entryIndex, dataOffset + OFFSET_DATA);
      return true;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      if (entry->chordLength == length &&
          memcmp(chords, entry->chords, sizeof(StenoChord) * length) == 0) {
        WriteEntryIndex(entryIndex, dataOffset + OFFSET_DATA);
        return true;
      }
      ++entryIndex;
    }
  }
  return false;
}

bool StenoUserDictionary::Remove(const StenoChord *chords, size_t length) {
  size_t entryIndex = StenoChord::Hash(chords, length);
  for (;;) {
    entryIndex = entryIndex & (activeDescriptor->data.hashTableSize - 1);

    uint32_t offset = activeDescriptor->data.hashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return false;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      if (entry->chordLength == length &&
          memcmp(chords, entry->chords, sizeof(StenoChord) * length) == 0) {
        WriteEntryIndex(entryIndex, OFFSET_DELETED);
        return true;
      }
    }

    ++entryIndex;
  }
}

void StenoUserDictionary::WriteEntryIndex(size_t entryIndex, size_t offset) {
  const uint32_t *entry = &activeDescriptor->data.hashTable[entryIndex];
  uint32_t *buffer = (uint32_t *)malloc(Flash::BLOCK_SIZE);
  const uint32_t *entryPage = RoundToPage(entry, Flash::BLOCK_SIZE);
  memcpy(buffer, entryPage, Flash::BLOCK_SIZE);

  buffer[entry - entryPage] = (uint32_t)offset;
  Flash::Write(entryPage, buffer, Flash::BLOCK_SIZE);

  free(buffer);
}

bool StenoUserDictionary::PrintDictionary(bool hasData) const {
  char *buffer = (char *)malloc(2048);

  for (size_t i = 0; i < activeDescriptor->data.hashTableSize; ++i) {
    uint32_t offset = activeDescriptor->data.hashTable[i];
    switch (offset) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      break;

    default:
      if (!hasData) {
        hasData = true;
        Console::Write("\n\t", 2);
      } else {
        Console::Write(",\n\t", 3);
      }
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      entry->Print(buffer);
    }
  }

  free(buffer);
  return hasData;
}

void StenoUserDictionary::PrintJsonDictionary() const {
  Console::Write("{", 1);
  PrintDictionary(false);
  Console::Write("\n}\n\n", 4);
}

void StenoUserDictionaryEntry::Print(char *buffer) const {
  char *p = buffer;
  *p++ = '\"';
  p = StenoChord::ToString(chords, chordLength, p);
  *p++ = '\"';
  *p++ = ':';
  *p++ = ' ';

  *p++ = '\"';
  p = Str::WriteJson(p, GetText());
  *p++ = '\"';
  Console::Write(buffer, p - buffer);
}

const char *StenoUserDictionary::GetName() const { return "user_dictionary"; }

void StenoUserDictionary::PrintInfo(int depth) const {
  size_t hashTableUsed = 0;
  for (size_t i = 0; i < activeDescriptor->data.hashTableSize; ++i) {
    switch (activeDescriptor->data.hashTable[i]) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      break;

    default:
      ++hashTableUsed;
    }
  }

  Console::Printf("%s%s\n", Spaces(depth), GetName());

  const char *prefix = Spaces(depth + 2);
  Console::Printf("%sFormat version: %u\n", prefix, USER_DICTIONARY_VERSION);
  Console::Printf("%sHash table usage: %zu/%zu\n", prefix, hashTableUsed,
                  activeDescriptor->data.hashTableSize);
  Console::Printf("%sData block usage: %zu/%zu\n", prefix,
                  activeDescriptor->data.dataBlockSize, layout.dataBlockSize);
}

//---------------------------------------------------------------------------

void StenoUserDictionary::PrintJsonDictionary_Binding(void *context,
                                                      const char *commandLine) {
  StenoUserDictionary *userDictionary = (StenoUserDictionary *)context;
  userDictionary->PrintJsonDictionary();
}

void StenoUserDictionary::Reset_Binding(void *context,
                                        const char *commandLine) {
  StenoUserDictionary *userDictionary = (StenoUserDictionary *)context;
  userDictionary->Reset();
  Console::Write("OK\n\n", 4);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../str.h"
#include "../unit_test.h"

static uint8_t userDictionaryBuffer[512 * 1024];

static bool IsEmpty(const uint8_t *p, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    if (p[i] != 0xff)
      return false;
  }
  return true;
}

TEST_BEGIN("StenoUserDictionary will reset if descriptor is invalid") {
  const StenoUserDictionaryDescriptor *descriptor =
      (const StenoUserDictionaryDescriptor *)(userDictionaryBuffer +
                                              512 * 1024 - 4096);

  StenoUserDictionaryData layout(userDictionaryBuffer,
                                 sizeof(userDictionaryBuffer));
  assert(descriptor == layout.GetDescriptor());

  for (size_t i = 0; i < sizeof(userDictionaryBuffer); ++i) {
    userDictionaryBuffer[i] = rand();
  }

  StenoUserDictionary userDictionary(layout);
  assert(IsEmpty(userDictionaryBuffer, 64 * 1024));
  assert(!IsEmpty(userDictionaryBuffer + 64 * 1024, 256));
  assert(descriptor->data.hashTable == (void *)userDictionaryBuffer);
  assert(descriptor->data.hashTableSize == 16 * 1024);
  assert(descriptor->data.dataBlock ==
         (void *)&userDictionaryBuffer[64 * 1024]);
  assert(descriptor->data.dataBlockSize == 0);
  assert(descriptor->data.maximumStrokeCount == 0);
  assert(descriptor->IsValid(layout));
}
TEST_END

TEST_BEGIN("StenoUserDictionary add and lookup test") {
  const StenoUserDictionaryDescriptor *descriptor =
      (const StenoUserDictionaryDescriptor *)(userDictionaryBuffer +
                                              512 * 1024 - 4096);

  StenoUserDictionaryData layout(userDictionaryBuffer,
                                 sizeof(userDictionaryBuffer));
  assert(descriptor == layout.GetDescriptor());

  for (size_t i = 0; i < sizeof(userDictionaryBuffer); ++i) {
    userDictionaryBuffer[i] = rand();
  }

  StenoUserDictionary userDictionary(layout);

  // spellchecker: disable
  const StenoChord KAT[] = {StenoChord("KAT")};
  const StenoChord TKOG[] = {StenoChord("TKOG")};
  const StenoChord KAPBG_RAO[] = {StenoChord("KAPBG"), StenoChord("RAO")};

  userDictionary.Add(KAT, 1, "cat");
  userDictionary.Add(TKOG, 1, "dog");
  userDictionary.Add(KAPBG_RAO, 2, "kangaroo");

  assert(Str::Eq(userDictionary.Lookup(KAT, 1).GetText(), "cat"));
  assert(Str::Eq(userDictionary.Lookup(TKOG, 1).GetText(), "dog"));
  assert(Str::Eq(userDictionary.Lookup(KAPBG_RAO, 2).GetText(), "kangaroo"));
  // spellchecker: enable
}
TEST_END

#if RUN_TESTS
TEST_BEGIN("StenoUserDictionary will dump Json dictionary") {
  const StenoUserDictionaryDescriptor *descriptor =
      (const StenoUserDictionaryDescriptor *)(userDictionaryBuffer +
                                              512 * 1024 - 4096);

  StenoUserDictionaryData layout(userDictionaryBuffer,
                                 sizeof(userDictionaryBuffer));
  assert(descriptor == layout.GetDescriptor());

  for (size_t i = 0; i < sizeof(userDictionaryBuffer); ++i) {
    userDictionaryBuffer[i] = rand();
  }

  StenoUserDictionary userDictionary(layout);

  // spellchecker: disable
  const StenoChord KAT[] = {StenoChord("KAT")};
  const StenoChord TKOG[] = {StenoChord("TKOG")};
  const StenoChord KAPBG_RAO[] = {StenoChord("KAPBG"), StenoChord("RAO")};

  userDictionary.Add(KAT, 1, "cat");
  userDictionary.Add(KAT, 1, "cat");
  userDictionary.Add(TKOG, 1, "dog");
  userDictionary.Add(KAPBG_RAO, 2, "kangaroo");

  userDictionary.PrintJsonDictionary();
  Console::history.push_back(0);
  assert(Str::Eq(&Console::history.front(), "{\n"
                                            "\t\"KAT\": \"cat\",\n"
                                            "\t\"TKOG\": \"dog\",\n"
                                            "\t\"KAPBG/RAO\": \"kangaroo\"\n"
                                            "}\n\n"));

  // spellchecker: enable
  Console::history.clear();
}
TEST_END

#endif

//---------------------------------------------------------------------------
