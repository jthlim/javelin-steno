//---------------------------------------------------------------------------

#include "user_dictionary.h"
#include "../console.h"
#include "../crc.h"
#include "../flash.h"
#include "../str.h"
#include "../stroke.h"
#include "../stroke_list_parser.h"
#include <assert.h>
#include JAVELIN_BOARD_CONFIG

//---------------------------------------------------------------------------

// These are chosen to minimize flash erase cycles.
constexpr size_t OFFSET_EMPTY = 0xffffffff;
constexpr size_t OFFSET_DELETED = 0;
constexpr size_t OFFSET_DATA = 1;

static const uint32_t USER_DICTIONARY_MAGIC = 0x4455534a; // 'JSUD'

static const uint32_t LEGACY_USER_DICTIONARY_VERSION = 1;
static const uint32_t USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION = 2;

// Offset within the flash page where descriptors will be stored.
const size_t DESCRIPTOR_OFFSET = 64;

static_assert(sizeof(StenoUserDictionaryDescriptor) <= DESCRIPTOR_OFFSET,
              "Descriptor size is larger than expected");

//---------------------------------------------------------------------------

struct StenoUserDictionaryEntry {
  uint32_t strokeLength;
  StenoStroke strokes[1];
  // After strokes is a null terminated string.

  void Print() const;
  char *GetText() const { return (char *)(strokes + strokeLength); }
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
  if (magic != USER_DICTIONARY_MAGIC || data.hashTable != layout.hashTable ||
      data.hashTableSize != layout.hashTableSize) {
    return false;
  }

  switch (version) {
  case LEGACY_USER_DICTIONARY_VERSION:
    return Crc32(&data, sizeof(data) - sizeof(data.reverseHashTable)) ==
           (size_t)data.reverseHashTable;

  case USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION:
    return Crc32(&data, sizeof(data)) == crc32;

  default:
    return false;
  }
}

void StenoUserDictionaryDescriptor::UpdateCrc32() {
  if (version == LEGACY_USER_DICTIONARY_VERSION) {
    data.reverseHashTable = (uint32_t *)(size_t)Crc32(
        &data, sizeof(data) - sizeof(data.reverseHashTable));
    return;
  }
  crc32 = Crc32(&data, sizeof(data));
}

//---------------------------------------------------------------------------

StenoUserDictionary::StenoUserDictionary(const StenoUserDictionaryData &layout)
    : StenoDictionary(0),

      descriptorBase(layout.GetDescriptor()), layout(layout) {
  activeDescriptor = FindMostRecentDescriptor();
  if (activeDescriptor == nullptr) {
    Reset();
  }
  if (activeDescriptor->version == LEGACY_USER_DICTIONARY_VERSION) {
    UpgradeToVersionWithReverseLookup();
  }
  maximumOutlineLength = activeDescriptor->data.maximumOutlineLength;
}

const StenoUserDictionaryDescriptor *
StenoUserDictionary::FindMostRecentDescriptor() const {
  const StenoUserDictionaryDescriptor *result = nullptr;

  for (size_t i = 0; i < Flash::BLOCK_SIZE; i += DESCRIPTOR_OFFSET) {
    const StenoUserDictionaryDescriptor *test =
        (const StenoUserDictionaryDescriptor *)((intptr_t)descriptorBase + i);

    if (test->IsValid(layout)) {
      if (!result || test->data.dataBlockSize >= result->data.dataBlockSize) {
        result = test;
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
  size_t entryIndex = lookup.hash;
  for (;;) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

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

      if (entry->strokeLength == lookup.length &&
          memcmp(lookup.strokes, entry->strokes,
                 sizeof(StenoStroke) * lookup.length) == 0) {
        return StenoDictionaryLookupResult::CreateStaticString(
            entry->GetText());
      }
    }

    ++entryIndex;
  }
}

const StenoDictionary *StenoUserDictionary::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {
  size_t entryIndex = lookup.hash;
  for (;;) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

    uint32_t offset = activeDescriptor->data.hashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return nullptr;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      if (entry->strokeLength == lookup.length &&
          memcmp(lookup.strokes, entry->strokes,
                 sizeof(StenoStroke) * lookup.length) == 0) {
        return this;
      }
    }

    ++entryIndex;
  }
}

void StenoUserDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  if (activeDescriptor->version !=
      USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION) {
    return;
  }

  uint32_t entryIndex = Crc32(result.lookup, result.lookupLength);

  for (;;) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

    uint32_t offset = activeDescriptor->data.reverseHashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      if (Str::Eq(entry->GetText(), result.lookup)) {
        result.AddResult(entry->strokes, entry->strokeLength, this);
      }
    }

    ++entryIndex;
  }
}

void StenoUserDictionary::UpgradeToVersionWithReverseLookup() {
  // Don't upgrade if the data block exceeds what is available.
  if (activeDescriptor->data.dataBlockSize > layout.dataBlockSize) {
    return;
  }

  // 1. Create reverse hash table.
  class ReverseHashTable {
  public:
    ReverseHashTable(size_t size) : size(size), data(new uint32_t[size]) {
      memset(data, 0xff, sizeof(uint32_t) * size);
    }
    ~ReverseHashTable() { delete[] data; }

    void Add(const char *word, uint32_t dataOffset) {
      size_t entryIndex = Crc32(word, strlen(word));

      for (int probeCount = 0; probeCount < 64; ++probeCount) {
        entryIndex &= size - 1;

        uint32_t offset = data[entryIndex];
        switch (offset) {
        case OFFSET_EMPTY:
        case OFFSET_DELETED:
          data[entryIndex] = dataOffset;
          return;

        default:
          ++entryIndex;
        }
      }
    }

    void Write(const void *target) {
      Flash::Write(target, data, sizeof(uint32_t) * size);
    }

  private:
    size_t size;
    uint32_t *data;
  };

  ReverseHashTable reverseHashTable(layout.hashTableSize);

  for (size_t i = 0; i < activeDescriptor->data.hashTableSize; ++i) {
    uint32_t offset = activeDescriptor->data.hashTable[i];
    switch (offset) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      reverseHashTable.Add(entry->GetText(), offset);
    }
  }

  // 2. Write updated descriptor.
  StenoUserDictionaryDescriptor *freshDescriptor =
      (StenoUserDictionaryDescriptor *)malloc(Flash::BLOCK_SIZE);
  memset(freshDescriptor, 0xff, Flash::BLOCK_SIZE);
  memcpy(freshDescriptor, activeDescriptor,
         sizeof(StenoUserDictionaryDescriptor));
  freshDescriptor->version = USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION;
  freshDescriptor->data.reverseHashTable =
      (uint32_t *)descriptorBase - freshDescriptor->data.hashTableSize;
  freshDescriptor->UpdateCrc32();

  reverseHashTable.Write(freshDescriptor->data.reverseHashTable);
  Flash::Write(descriptorBase, freshDescriptor, Flash::BLOCK_SIZE);

  free(freshDescriptor);

  activeDescriptor = descriptorBase;
}

//---------------------------------------------------------------------------

void StenoUserDictionary::Reset() {
  Flash::Erase(layout.hashTable, layout.hashTableSize * sizeof(uint32_t));
  Flash::Erase(layout.reverseHashTable,
               layout.hashTableSize * sizeof(uint32_t));

  StenoUserDictionaryDescriptor *freshDescriptor =
      (StenoUserDictionaryDescriptor *)malloc(Flash::BLOCK_SIZE);

  // Use 0xff rather than 0 to reduce flash I/O.
  memset(freshDescriptor, 0xff, Flash::BLOCK_SIZE);

  freshDescriptor->magic = USER_DICTIONARY_MAGIC;
  freshDescriptor->version = USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION;
  freshDescriptor->data.hashTable = layout.hashTable;
  freshDescriptor->data.hashTableSize = layout.hashTableSize;
  freshDescriptor->data.dataBlock = layout.dataBlock;
  freshDescriptor->data.dataBlockSize = 0;
  freshDescriptor->data.maximumOutlineLength = 0;
  freshDescriptor->data.reverseHashTable = layout.reverseHashTable;
  freshDescriptor->UpdateCrc32();

  Flash::Write(descriptorBase, freshDescriptor, Flash::BLOCK_SIZE);

  free(freshDescriptor);
  activeDescriptor = descriptorBase;
}

bool StenoUserDictionary::Add(const StenoStroke *strokes, size_t length,
                              const char *word) {
  // Verify that it doesn't already exist.
  StenoDictionaryLookupResult lookup =
      Lookup(StenoDictionaryLookup(strokes, length));

  if (lookup.IsValid() && Str::Eq(lookup.GetText(), word)) {
    lookup.Destroy();
    return true;
  }
  lookup.Destroy();

  AddToDataBlockResult data = AddToDataBlock(strokes, (uint32_t)length, word);
  if (data.length == 0) {
    return false;
  }
  AddToDescriptor(length, data.length);
  if (!AddToHashTable(strokes, length, data.offset)) {
    return false;
  }

  AddToReverseHashTable(word, data.offset);

  maximumOutlineLength = activeDescriptor->data.maximumOutlineLength;
  UpdateMaximumOutlineLength();

  return true;
}

StenoUserDictionary::AddToDataBlockResult
StenoUserDictionary::AddToDataBlock(const StenoStroke *strokes, uint32_t length,
                                    const char *word) {
  size_t wordLength = strlen(word);

  // Need to store null terminator + round up to nearest 4 bytes.
  size_t wordStorageLength = (wordLength + 4) & -4;

  size_t offset = activeDescriptor->data.dataBlockSize;

  size_t totalLength =
      sizeof(uint32_t) + sizeof(StenoStroke) * length + wordStorageLength;

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
  memcpy(p, strokes, sizeof(StenoStroke) * length);
  p += sizeof(StenoStroke) * length;
  memcpy(p, word, wordLength + 1);
  p += wordStorageLength;

  size_t bytesToWrite =
      ((p - buffer) + (Flash::BLOCK_SIZE - 1)) & -Flash::BLOCK_SIZE;

  assert(bytesToWrite <= Flash::BLOCK_SIZE * 2);
  Flash::Write(originalDataPage, buffer, bytesToWrite);

  free(buffer);

  return AddToDataBlockResult(offset, totalLength);
}

void StenoUserDictionary::AddToDescriptor(size_t strokeLength,
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
  if (strokeLength > activeDescriptor->data.maximumOutlineLength) {
    freshDescriptor->data.maximumOutlineLength = (uint32_t)strokeLength;
  }
  freshDescriptor->UpdateCrc32();

  Flash::Write(descriptorBase, buffer, Flash::BLOCK_SIZE);

  free(buffer);

  activeDescriptor =
      (StenoUserDictionaryDescriptor *)((intptr_t)descriptorBase +
                                        freshDescriptorOffset);
}

bool StenoUserDictionary::AddToHashTable(const StenoStroke *strokes,
                                         size_t length, size_t dataOffset) {
  size_t entryIndex = StenoStroke::Hash(strokes, length);

  for (int probeCount = 0; probeCount < 64; ++probeCount) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

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

      if (entry->strokeLength == length &&
          memcmp(strokes, entry->strokes, sizeof(StenoStroke) * length) == 0) {
        WriteEntryIndex(entryIndex, dataOffset + OFFSET_DATA);
        return true;
      }
      ++entryIndex;
    }
  }
  return false;
}

bool StenoUserDictionary::AddToReverseHashTable(const char *word,
                                                size_t dataOffset) {
  if (activeDescriptor->version !=
      USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION) {
    return false;
  }

  size_t entryIndex = Crc32(word, strlen(word));

  for (int probeCount = 0; probeCount < 64; ++probeCount) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

    uint32_t offset = activeDescriptor->data.reverseHashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      WriteReverseEntryIndex(entryIndex, dataOffset + OFFSET_DATA);
      return true;

    default:
      ++entryIndex;
    }
  }
  return false;
}

bool StenoUserDictionary::Remove(const StenoStroke *strokes, size_t length) {
  const StenoUserDictionaryEntry *deletedEntry =
      RemoveFromHashTable(strokes, length);
  if (deletedEntry == nullptr) {
    return false;
  }

  RemoveFromReverseHashTable(deletedEntry);
  return true;
}

const StenoUserDictionaryEntry *
StenoUserDictionary::RemoveFromHashTable(const StenoStroke *strokes,
                                         size_t length) {
  size_t entryIndex = StenoStroke::Hash(strokes, length);
  for (;;) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

    uint32_t offset = activeDescriptor->data.hashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return nullptr;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      if (entry->strokeLength == length &&
          memcmp(strokes, entry->strokes, sizeof(StenoStroke) * length) == 0) {
        WriteEntryIndex(entryIndex, OFFSET_DELETED);
        return entry;
      }
    }

    ++entryIndex;
  }
}

bool StenoUserDictionary::RemoveFromReverseHashTable(
    const StenoUserDictionaryEntry *entryToDelete) {
  if (activeDescriptor->version !=
      USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION) {
    return false;
  }

  const char *text = entryToDelete->GetText();
  size_t entryIndex = Crc32(text, strlen(text));

  for (;;) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

    uint32_t offset = activeDescriptor->data.reverseHashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return false;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      if (entry == entryToDelete) {
        WriteReverseEntryIndex(entryIndex, OFFSET_DELETED);
        return entry;
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

void StenoUserDictionary::WriteReverseEntryIndex(size_t entryIndex,
                                                 size_t offset) {
  const uint32_t *entry = &activeDescriptor->data.reverseHashTable[entryIndex];
  uint32_t *buffer = (uint32_t *)malloc(Flash::BLOCK_SIZE);
  const uint32_t *entryPage = RoundToPage(entry, Flash::BLOCK_SIZE);
  memcpy(buffer, entryPage, Flash::BLOCK_SIZE);

  buffer[entry - entryPage] = (uint32_t)offset;
  Flash::Write(entryPage, buffer, Flash::BLOCK_SIZE);

  free(buffer);
}

bool StenoUserDictionary::PrintDictionary(const char *name,
                                          bool hasData) const {
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
        Console::Printf("\n\t");
      } else {
        Console::Printf(",\n\t");
      }
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      entry->Print();
    }
  }

  free(buffer);
  return hasData;
}

void StenoUserDictionary::PrintJsonDictionary() const {
  Console::Printf("{");
  PrintDictionary(nullptr, false);
  Console::Printf("\n}\n\n");
}

void StenoUserDictionaryEntry::Print() const {
  Console::Printf("\"%T\": \"%J\"", strokes, strokeLength, GetText());
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
  Console::Printf("%sFormat version: %u\n", prefix, activeDescriptor->version);
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
  Console::SendOk();
}

void StenoUserDictionary::AddEntry_Binding(void *context,
                                           const char *commandLine) {
  const char *strokeStart = strchr(commandLine, ' ');
  if (!strokeStart) {
    Console::Printf("ERR No stroke specified\n\n");
    return;
  }

  StrokeListParser parser;
  if (!parser.Parse(strokeStart + 1)) {
    Console::Printf("ERR Cannot parse stroke near %s\n\n", parser.failureOrEnd);
    return;
  }

  const char *translationStart = parser.failureOrEnd;
  if (*translationStart == '\0') {
    Console::Printf("ERR No translation specified\n\n");
    return;
  }

  StenoUserDictionary *userDictionary = (StenoUserDictionary *)context;
  if (!userDictionary->Add(parser.strokes, parser.length, translationStart)) {
    Console::Printf("ERR Unable to write to user dictionary\n\n");
    return;
  }

  Console::SendOk();
}

void StenoUserDictionary::RemoveEntry_Binding(void *context,
                                              const char *commandLine) {
  const char *strokeStart = strchr(commandLine, ' ');
  if (!strokeStart) {
    Console::Printf("ERR No stroke specified\n\n");
    return;
  }

  StrokeListParser parser;
  if (!parser.Parse(strokeStart + 1)) {
    Console::Printf("ERR Cannot parse stroke near %s\n\n", parser.failureOrEnd);
    return;
  }

  StenoUserDictionary *userDictionary = (StenoUserDictionary *)context;
  userDictionary->Remove(parser.strokes, parser.length);
  Console::SendOk();
}

void StenoUserDictionary::AddConsoleCommands(Console &console) {
#if JAVELIN_USE_USER_DICTIONARY
  console.RegisterCommand("print_user_dictionary",
                          "Prints the user dictionary in JSON format",
                          &PrintJsonDictionary_Binding, this);
  console.RegisterCommand("reset_user_dictionary", "Resets the user dictionary",
                          &Reset_Binding, this);
  console.RegisterCommand("add_user_entry",
                          "Adds a definition to the user dictionary",
                          &AddEntry_Binding, this);
  console.RegisterCommand("remove_user_entry",
                          "Removes a definition from the user dictionary",
                          &RemoveEntry_Binding, this);
#endif
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../str.h"
#include "../unit_test.h"

static uint8_t userDictionaryBuffer[512 * 1024];

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
  assert(Flash::IsErased(userDictionaryBuffer, 64 * 1024));
  assert(Flash::IsErased(userDictionaryBuffer + 64 * 1024, 64 * 1024));
  assert(
      !Flash::IsErased(userDictionaryBuffer + 128 * 1024, 384 * 1024 - 4096));
  assert(descriptor->data.hashTable == (void *)userDictionaryBuffer);
  assert(descriptor->data.reverseHashTable ==
         (void *)&userDictionaryBuffer[64 * 1024]);
  assert(descriptor->data.hashTableSize == 16 * 1024);
  assert(descriptor->data.dataBlock ==
         (void *)&userDictionaryBuffer[128 * 1024]);
  assert(descriptor->data.dataBlockSize == 0);
  assert(descriptor->data.maximumOutlineLength == 0);
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
  const StenoStroke KAT[] = {StenoStroke("KAT")};
  const StenoStroke TKOG[] = {StenoStroke("TKOG")};
  const StenoStroke KAPBG_RAO[] = {StenoStroke("KAPBG"), StenoStroke("RAO")};

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

static void VerifyReverseLookup(StenoUserDictionary &userDictionary,
                                const char *text, StenoStroke expected) {
  StenoReverseDictionaryLookup lookup(2, text);
  userDictionary.ReverseLookup(lookup);
  assert(lookup.resultCount > 0);
  for (size_t i = 0; i < lookup.resultCount; ++i) {
    assert(lookup.results[i].length == 1);
    if (lookup.strokes[i] == expected) {
      return;
    }
  }
  assert(false);
}

static void VerifyNoReverseLookup(StenoUserDictionary &userDictionary,
                                  const char *text) {
  StenoReverseDictionaryLookup lookup(2, text);
  userDictionary.ReverseLookup(lookup);
  assert(lookup.resultCount == 0);
}

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
  const StenoStroke KAT[] = {StenoStroke("KAT")};
  const StenoStroke TKOG[] = {StenoStroke("TKOG")};
  const StenoStroke KAPBG_RAO[] = {StenoStroke("KAPBG"), StenoStroke("RAO")};

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

  Console::history.clear();

  VerifyReverseLookup(userDictionary, "cat", StenoStroke("KAT"));
  VerifyReverseLookup(userDictionary, "dog", StenoStroke("TKOG"));

  userDictionary.Remove(KAT, 1);
  userDictionary.PrintJsonDictionary();
  Console::history.push_back(0);
  assert(Str::Eq(&Console::history.front(), "{\n"
                                            "\t\"TKOG\": \"dog\",\n"
                                            "\t\"KAPBG/RAO\": \"kangaroo\"\n"
                                            "}\n\n"));

  VerifyNoReverseLookup(userDictionary, "cat");
  // spellchecker: enable
}
TEST_END

#endif

//---------------------------------------------------------------------------
