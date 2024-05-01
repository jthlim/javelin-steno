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
  return magic == USER_DICTIONARY_MAGIC && data.hashTable == layout.hashTable &&
         data.hashTableSize == layout.hashTableSize &&
         version == USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION &&
         Crc32(&data, sizeof(data)) == crc32;
}

void StenoUserDictionaryDescriptor::UpdateCrc32() {
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
  const size_t activeOffset =
      (intptr_t)activeDescriptor - (intptr_t)descriptorBase;
  const size_t nextOffset =
      (activeOffset + DESCRIPTOR_OFFSET) % Flash::BLOCK_SIZE;
  return nextOffset;
}

StenoDictionaryLookupResult
StenoUserDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  size_t entryIndex = lookup.hash;
  for (;;) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

    const uint32_t offset = activeDescriptor->data.hashTable[entryIndex];
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
          StenoStroke::Equals(lookup.strokes, entry->strokes, lookup.length)) {
        return StenoDictionaryLookupResult::CreateStaticString(
            entry->GetText());
      }
    }

    ++entryIndex;
  }
}

const StenoDictionary *StenoUserDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  size_t entryIndex = lookup.hash;
  for (;;) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

    const uint32_t offset = activeDescriptor->data.hashTable[entryIndex];
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
          StenoStroke::Equals(lookup.strokes, entry->strokes, lookup.length)) {
        return this;
      }
    }

    ++entryIndex;
  }
}

void StenoUserDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  if (activeDescriptor->version !=
      USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION) {
    return;
  }

  uint32_t entryIndex = lookup.GetLookupCrc();

  for (;;) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

    const uint32_t offset = activeDescriptor->data.reverseHashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      if (Str::Eq(entry->GetText(), lookup.definition)) {
        lookup.AddResult(entry->strokes, entry->strokeLength, this);
      }
    }

    ++entryIndex;
  }
}

//---------------------------------------------------------------------------

void StenoUserDictionary::Reset() {
  Flash::EraseBlock(layout.hashTable, layout.hashTableSize * sizeof(uint32_t));
  Flash::EraseBlock(layout.reverseHashTable,
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

  Flash::WriteBlock(descriptorBase, freshDescriptor, Flash::BLOCK_SIZE);

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

  const AddToDataBlockResult data =
      AddToDataBlock(strokes, (uint32_t)length, word);
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
  const size_t wordLength = strlen(word);

  // Need to store null terminator + round up to nearest 4 bytes.
  const size_t wordStorageLength = (wordLength + 4) & -4;

  const size_t totalLength =
      sizeof(uint32_t) + sizeof(StenoStroke) * length + wordStorageLength;

  uint8_t *buffer = (uint8_t *)malloc(totalLength);
  StenoUserDictionaryEntry *entry = (StenoUserDictionaryEntry *)buffer;
  entry->strokeLength = length;
  strokes->CopyTo(entry->strokes, length);
  memcpy(buffer + sizeof(uint32_t) + sizeof(StenoStroke) * length, word,
         wordLength + 1);

  const size_t offset = activeDescriptor->data.dataBlockSize;
  const uint8_t *target = activeDescriptor->data.dataBlock + offset;
  Flash::Write(target, buffer, totalLength);
  free(buffer);

  return AddToDataBlockResult(offset, totalLength);
}

void StenoUserDictionary::AddToDescriptor(size_t strokeLength,
                                          size_t dataLength) {
  StenoUserDictionaryDescriptor newDescriptor = *activeDescriptor;

  newDescriptor.data.dataBlockSize += dataLength;
  if (strokeLength > newDescriptor.data.maximumOutlineLength) {
    newDescriptor.data.maximumOutlineLength = (uint32_t)strokeLength;
  }
  newDescriptor.UpdateCrc32();

  const size_t newDescriptorOffset = GetNextDescriptorToWriteOffset();
  StenoUserDictionaryDescriptor *destination =
      (StenoUserDictionaryDescriptor *)((intptr_t)descriptorBase +
                                        newDescriptorOffset);
  activeDescriptor = destination;

  Flash::Write(destination, &newDescriptor, sizeof(newDescriptor));
}

bool StenoUserDictionary::AddToHashTable(const StenoStroke *strokes,
                                         size_t length, size_t dataOffset) {
  size_t entryIndex = StenoStroke::Hash(strokes, length);

  for (int probeCount = 0; probeCount < 64; ++probeCount) {
    entryIndex &= activeDescriptor->data.hashTableSize - 1;

    const uint32_t offset = activeDescriptor->data.hashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      WriteEntryIndex(entryIndex, uint32_t(dataOffset + OFFSET_DATA));
      return true;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      if (entry->strokeLength == length &&
          StenoStroke::Equals(strokes, entry->strokes, length)) {
        WriteEntryIndex(entryIndex, uint32_t(dataOffset + OFFSET_DATA));
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

    const uint32_t offset = activeDescriptor->data.reverseHashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      WriteReverseEntryIndex(entryIndex, uint32_t(dataOffset + OFFSET_DATA));
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

    const uint32_t offset = activeDescriptor->data.hashTable[entryIndex];
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
          StenoStroke::Equals(strokes, entry->strokes, length)) {
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

    const uint32_t offset = activeDescriptor->data.reverseHashTable[entryIndex];
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

void StenoUserDictionary::WriteEntryIndex(size_t entryIndex, uint32_t offset) {
  const uint32_t *entry = &activeDescriptor->data.hashTable[entryIndex];
  Flash::Write(entry, &offset, sizeof(offset));
}

void StenoUserDictionary::WriteReverseEntryIndex(size_t entryIndex,
                                                 uint32_t offset) {
  const uint32_t *entry = &activeDescriptor->data.reverseHashTable[entryIndex];
  Flash::Write(entry, &offset, sizeof(offset));
}

void StenoUserDictionary::PrintDictionary(
    PrintDictionaryContext &context) const {
  char *buffer = (char *)malloc(2048);

  for (size_t i = 0; i < activeDescriptor->data.hashTableSize; ++i) {
    const uint32_t offset = activeDescriptor->data.hashTable[i];
    switch (offset) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry *)(activeDescriptor->data.dataBlock +
                                             offset - OFFSET_DATA);

      context.Print(entry->strokes, entry->strokeLength, entry->GetText());
    }
  }

  free(buffer);
}

void StenoUserDictionary::PrintJsonDictionary() const {
  Console::Printf("{");
  PrintDictionaryContext context(GetName());
  PrintDictionary(context);
  Console::Printf("\n}\n\n");
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

__attribute__((aligned(4096))) static uint8_t userDictionaryBuffer[512 * 1024];

TEST_BEGIN("StenoUserDictionary will reset if descriptor is invalid") {
  const StenoUserDictionaryData layout(userDictionaryBuffer,
                                       sizeof(userDictionaryBuffer));

  const StenoUserDictionaryDescriptor *descriptor =
      (const StenoUserDictionaryDescriptor *)(userDictionaryBuffer +
                                              512 * 1024 - 4096);
  assert(descriptor == layout.GetDescriptor());

  for (size_t i = 0; i < sizeof(userDictionaryBuffer); ++i) {
    userDictionaryBuffer[i] = rand();
  }

  const StenoUserDictionary userDictionary(layout);
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
  const StenoUserDictionaryData layout(userDictionaryBuffer,
                                       sizeof(userDictionaryBuffer));

  for (size_t i = 0; i < sizeof(userDictionaryBuffer); ++i) {
    userDictionaryBuffer[i] = rand();
  }

  StenoUserDictionary userDictionary(layout);

  // spellchecker: disable
  const StenoStroke KAT[] = {StenoStroke("KAT")};
  const StenoStroke TKOG[] = {StenoStroke("TKOG")};
  const StenoStroke KAPBG_RAO[] = {StenoStroke("KAPBG"), StenoStroke("RAO")};

  for (size_t i = 0; i < 100; ++i) {
    userDictionary.Add(KAT, 1, "cat");
    userDictionary.Add(TKOG, 1, "dog");
    userDictionary.Add(KAPBG_RAO, 2, "kangaroo");

    assert(Str::Eq(userDictionary.Lookup(KAT, 1).GetText(), "cat"));
    assert(Str::Eq(userDictionary.Lookup(TKOG, 1).GetText(), "dog"));
    assert(Str::Eq(userDictionary.Lookup(KAPBG_RAO, 2).GetText(), "kangaroo"));

    userDictionary.Remove(KAT, 1);
    userDictionary.Remove(TKOG, 1);
    userDictionary.Remove(KAPBG_RAO, 2);

    assert(!userDictionary.Lookup(KAT, 1).IsValid());
    assert(!userDictionary.Lookup(TKOG, 1).IsValid());
    assert(!userDictionary.Lookup(KAPBG_RAO, 2).IsValid());
  }
  // spellchecker: enable
}
TEST_END

#if RUN_TESTS

static void VerifyReverseLookup(StenoUserDictionary &userDictionary,
                                const char *text, StenoStroke expected) {
  StenoReverseDictionaryLookup lookup(2, text);
  userDictionary.ReverseLookup(lookup);
  assert(lookup.results.IsNotEmpty() > 0);
  for (size_t i = 0; i < lookup.results.GetCount(); ++i) {
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
  assert(lookup.results.IsEmpty());
}

TEST_BEGIN("StenoUserDictionary will dump Json dictionary") {
  const StenoUserDictionaryData layout(userDictionaryBuffer,
                                       sizeof(userDictionaryBuffer));

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
