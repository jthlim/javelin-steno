//---------------------------------------------------------------------------

#include "user_dictionary.h"
#include "../console.h"
#include "../crc32.h"
#include "../flash.h"
#include "../hal/external_flash.h"
#include "../mem.h"
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

constexpr uint32_t USER_DICTIONARY_MAGIC = 0x4455534a; // 'JSUD'

// TODO: July 2026: Remove support for version 2.
constexpr uint32_t USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION = 2;
constexpr uint32_t
    USER_DICTIONARY_WITH_REVERSE_LOOKUP_AND_REVERSE_DATABLOCK_VERSION = 3;

constexpr size_t DESCRIPTOR_ENTRY_SIZE = 64;

static_assert(sizeof(StenoUserDictionaryDescriptor) <= DESCRIPTOR_ENTRY_SIZE,
              "Descriptor size is larger than expected");
static_assert(StenoUserDictionaryData::ALL_DESCRIPTORS_SIZE %
                  DESCRIPTOR_ENTRY_SIZE ==
              0);

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

inline uint32_t StenoUserDictionaryData::Crc32() const {
  return ::Crc32::Hash(this, sizeof(*this));
}

const StenoUserDictionaryDescriptor *
StenoUserDictionaryData::FindMostRecentDescriptor() const {
  const StenoUserDictionaryDescriptor *result = nullptr;

  // Search backwards as the largest will likely occur last.
  // This limits the number of Crc32 calculations required.
  for (size_t i = ALL_DESCRIPTORS_SIZE; i != 0;) {
    i -= DESCRIPTOR_ENTRY_SIZE;

    const StenoUserDictionaryDescriptor *test = GetDescriptor(i);
    if ((!result || test->GetUsedDataBlockSize(dataBlockSize) >=
                        result->GetUsedDataBlockSize(dataBlockSize)) &&
        test->IsValid(*this)) {
      result = test;
    }
  }

  return result;
}

//---------------------------------------------------------------------------

bool StenoUserDictionaryDescriptor::IsValid(
    const StenoUserDictionaryData &layout) const {
  return magic == USER_DICTIONARY_MAGIC && data.hashTable == layout.hashTable &&
         data.hashTableSize == layout.hashTableSize &&
         (version == USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION ||
          version ==
              USER_DICTIONARY_WITH_REVERSE_LOOKUP_AND_REVERSE_DATABLOCK_VERSION) &&
         data.Crc32() == crc32;
}

inline void StenoUserDictionaryDescriptor::UpdateCrc32() {
  crc32 = data.Crc32();
}

size_t StenoUserDictionaryDescriptor::GetUsedDataBlockSize(
    size_t totalDataBlockSize) const {
  return version == USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION
             ? data.dataBlockSize
             : totalDataBlockSize - data.dataBlockSizeRemaining;
}

//---------------------------------------------------------------------------

StenoUserDictionary::StenoUserDictionary(const StenoUserDictionaryData &layout)
    : StenoDictionary(0), descriptorBase(layout.GetDescriptor()),
      layout(layout) {
  activeDescriptor = layout.FindMostRecentDescriptor();
  if (activeDescriptor == nullptr) {
    Reset();
  } else {
    activeDescriptorCopy = *activeDescriptor;
  }
  maximumOutlineLength = activeDescriptorCopy.data.maximumOutlineLength;
}

size_t StenoUserDictionary::GetNextDescriptorToWriteOffset() const {
  const size_t activeOffset =
      (intptr_t)activeDescriptor - (intptr_t)descriptorBase;
  const size_t nextOffset = (activeOffset + DESCRIPTOR_ENTRY_SIZE) %
                            StenoUserDictionaryData::ALL_DESCRIPTORS_SIZE;
  return nextOffset;
}

const StenoUserDictionaryEntry *
StenoUserDictionary::LookupEntry(const StenoDictionaryLookup &lookup) const {
  size_t entryIndex = lookup.hash;
  for (;;) {
    entryIndex &= activeDescriptorCopy.data.hashTableSize - 1;

    const uint32_t offset = activeDescriptorCopy.data.hashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return nullptr;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry
               *)(activeDescriptorCopy.data.dataBlock + offset - OFFSET_DATA);

      if (entry->strokeLength == lookup.length &&
          StenoStroke::Equals(lookup.strokes, entry->strokes, lookup.length)) {
        return entry;
      }
    }

    ++entryIndex;
  }
}

StenoDictionaryLookupResult
StenoUserDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const StenoUserDictionaryEntry *entry = LookupEntry(lookup);
  if (entry == nullptr) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }
  return StenoDictionaryLookupResult::CreateStaticString(entry->GetText());
}

const StenoDictionary *StenoUserDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  const StenoUserDictionaryEntry *entry = LookupEntry(lookup);
  return entry != nullptr ? this : nullptr;
}

void StenoUserDictionary::PrintEntriesWithPartialOutline(
    PrintPartialOutlineContext &context) const {
  const size_t hashTableSize = activeDescriptorCopy.data.hashTableSize;
  const uint32_t *const hashTable = activeDescriptorCopy.data.hashTable;
  for (size_t i = 0; i < hashTableSize; ++i) {
    const uint32_t offset = hashTable[i];
    switch (offset) {
    [[likely]] case OFFSET_EMPTY:
      break;

    [[unlikely]] case OFFSET_DELETED:
      break;

    [[unlikely]] default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry
               *)(activeDescriptorCopy.data.dataBlock + offset - OFFSET_DATA);

      if (entry->strokeLength > context.length) {
        if (StenoStroke::HasPartialOutline(
                entry->strokes, context.strokes, context.length,
                entry->strokeLength - context.length)) {
          context.Add(entry->strokes, entry->strokeLength, entry->GetText(),
                      this);
          if (context.IsDone()) {
            return;
          }
        }
      }
    }
  }
}

void StenoUserDictionary::PrintEntriesWithPrefix(
    PrintPrefixContext &context) const {
  const size_t hashTableSize = activeDescriptorCopy.data.hashTableSize;
  const uint32_t *const hashTable = activeDescriptorCopy.data.hashTable;
  for (size_t i = 0; i < hashTableSize; ++i) {
    const uint32_t offset = hashTable[i];
    switch (offset) {
    [[likely]] case OFFSET_EMPTY:
      break;

    [[unlikely]] case OFFSET_DELETED:
      break;

    [[unlikely]] default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry
               *)(activeDescriptorCopy.data.dataBlock + offset - OFFSET_DATA);

      const char *text = entry->GetText();
      if (Str::HasPrefix(text, context.prefix) &&
          text[context.prefixLength] != 0) {
        context.Add(entry->strokes, entry->strokeLength, text, this);
      }
      break;
    }
  }
}

void StenoUserDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  uint32_t entryIndex = lookup.definitionCrc;

  for (;;) {
    entryIndex &= activeDescriptorCopy.data.hashTableSize - 1;

    const uint32_t offset =
        activeDescriptorCopy.data.reverseHashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry
               *)(activeDescriptorCopy.data.dataBlock + offset - OFFSET_DATA);

      if (Str::Eq(entry->GetText(), lookup.definition)) {
        lookup.AddResult(entry->strokes, entry->strokeLength, this);
      }
    }

    ++entryIndex;
  }
}

//---------------------------------------------------------------------------

void StenoUserDictionary::Reset() {
  Flash::EraseBlock(layout.GetDataStart(), layout.GetDataLength());

  StenoUserDictionaryDescriptor freshDescriptor;

  freshDescriptor.magic = USER_DICTIONARY_MAGIC;
  freshDescriptor.version =
      USER_DICTIONARY_WITH_REVERSE_LOOKUP_AND_REVERSE_DATABLOCK_VERSION;
  freshDescriptor.data.hashTable = layout.hashTable;
  freshDescriptor.data.hashTableSize = layout.hashTableSize;
  freshDescriptor.data.dataBlock = layout.dataBlock;
  freshDescriptor.data.dataBlockSizeRemaining = layout.dataBlockSize;
  freshDescriptor.data.maximumOutlineLength = 0;
  freshDescriptor.data.reverseHashTable = layout.reverseHashTable;
  freshDescriptor.UpdateCrc32();

  Flash::Write(descriptorBase, &freshDescriptor,
               sizeof(StenoUserDictionaryDescriptor), FlashWriteMode::RESET);

  activeDescriptor = descriptorBase;
  activeDescriptorCopy = freshDescriptor;
}

void StenoUserDictionary::DestroyDescriptorBlock() {
  Flash::EraseBlock(layout.GetDescriptor(),
                    StenoUserDictionaryData::ALL_DESCRIPTORS_SIZE);
}

bool StenoUserDictionary::Add(const StenoStroke *strokes, size_t length,
                              const char *word) {
  // Verify that it doesn't already exist.
  const StenoUserDictionaryEntry *entry =
      LookupEntry(StenoDictionaryLookup(strokes, length));

  if (entry && Str::Eq(entry->GetText(), word)) {
    return true;
  }

  const AddToDataBlockResult data =
      AddToDataBlock(strokes, (uint32_t)length, word);
  if (data.length == 0) {
    return false;
  }
  AddToDescriptor(length, data);
  if (!AddToHashTable(strokes, length, data.offset)) {
    return false;
  }

  if (entry) {
    RemoveFromReverseHashTable(entry);
  }
  AddToReverseHashTable(word, data.offset);

  maximumOutlineLength = activeDescriptorCopy.data.maximumOutlineLength;
  OnLookupDataChanged();

  return true;
}

StenoUserDictionary::AddToDataBlockResult
StenoUserDictionary::AddToDataBlock(const StenoStroke *strokes, uint32_t length,
                                    const char *word) {
  const size_t wordLength = Str::Length(word);

  // Need to store null terminator + round up to nearest 4 bytes.
  const size_t wordStorageLength = (wordLength + 4) & -4;

  const size_t totalLength =
      sizeof(uint32_t) + sizeof(StenoStroke) * length + wordStorageLength;

  if (activeDescriptorCopy.version ==
      USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION) {
    if (activeDescriptorCopy.data.dataBlockSize + totalLength >
        layout.dataBlockSize) {
      // Too big!
      return AddToDataBlockResult(0, 0);
    }
  } else {
    if (activeDescriptorCopy.data.dataBlockSizeRemaining < totalLength) {
      // Too big!
      return AddToDataBlockResult(0, 0);
    }
  }

  uint8_t *buffer = (uint8_t *)malloc(totalLength);
  StenoUserDictionaryEntry *entry = (StenoUserDictionaryEntry *)buffer;
  entry->strokeLength = length;
  strokes->CopyTo(entry->strokes, length);
  memcpy(buffer + sizeof(uint32_t) + sizeof(StenoStroke) * length, word,
         wordLength + 1);

  const size_t dataBlockOffset =
      activeDescriptorCopy.version ==
              USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION
          ? activeDescriptorCopy.data.dataBlockSize
          : activeDescriptorCopy.data.dataBlockSizeRemaining - totalLength;

  const FlashWriteMode writeMode =
      activeDescriptorCopy.version ==
              USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION
          ? FlashWriteMode::PRESERVE_BEFORE
          : FlashWriteMode::PRESERVE_AFTER;

  const uint8_t *target = activeDescriptorCopy.data.dataBlock + dataBlockOffset;
  Flash::Write(target, buffer, totalLength, writeMode);
  free(buffer);

  return AddToDataBlockResult(dataBlockOffset, totalLength);
}

void StenoUserDictionary::AddToDescriptor(
    size_t strokeLength, AddToDataBlockResult dataBlockResult) {
  StenoUserDictionaryDescriptor newDescriptor = activeDescriptorCopy;

  if (activeDescriptorCopy.version ==
      USER_DICTIONARY_WITH_REVERSE_LOOKUP_VERSION) {
    newDescriptor.data.dataBlockSize += dataBlockResult.length;
  } else {
    newDescriptor.data.dataBlockSizeRemaining = dataBlockResult.offset;
  }

  if (strokeLength > newDescriptor.data.maximumOutlineLength) {
    newDescriptor.data.maximumOutlineLength = (uint32_t)strokeLength;
  }
  newDescriptor.UpdateCrc32();

  const size_t newDescriptorOffset = GetNextDescriptorToWriteOffset();
  StenoUserDictionaryDescriptor *destination =
      (StenoUserDictionaryDescriptor *)((intptr_t)descriptorBase +
                                        newDescriptorOffset);

  activeDescriptor = destination;
  activeDescriptorCopy = newDescriptor;

  Flash::Write(destination, &newDescriptor, sizeof(newDescriptor),
               FlashWriteMode::RESET);
}

bool StenoUserDictionary::AddToHashTable(const StenoStroke *strokes,
                                         size_t length, size_t dataOffset) {
  size_t entryIndex = StenoStroke::Hash(strokes, length);

  for (int probeCount = 0; probeCount < 64; ++probeCount) {
    entryIndex &= activeDescriptorCopy.data.hashTableSize - 1;

    const uint32_t offset = activeDescriptorCopy.data.hashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      WriteEntryIndex(entryIndex, uint32_t(dataOffset + OFFSET_DATA));
      return true;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry
               *)(activeDescriptorCopy.data.dataBlock + offset - OFFSET_DATA);

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
  size_t entryIndex = Crc32::Hash(word, Str::Length(word));

  for (int probeCount = 0; probeCount < 64; ++probeCount) {
    entryIndex &= activeDescriptorCopy.data.hashTableSize - 1;

    const uint32_t offset =
        activeDescriptorCopy.data.reverseHashTable[entryIndex];
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
  OnLookupDataChanged();
  return true;
}

const StenoUserDictionaryEntry *
StenoUserDictionary::RemoveFromHashTable(const StenoStroke *strokes,
                                         size_t length) {
  size_t entryIndex = StenoStroke::Hash(strokes, length);
  for (;;) {
    entryIndex &= activeDescriptorCopy.data.hashTableSize - 1;

    const uint32_t offset = activeDescriptorCopy.data.hashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return nullptr;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry
               *)(activeDescriptorCopy.data.dataBlock + offset - OFFSET_DATA);

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
  const char *text = entryToDelete->GetText();
  size_t entryIndex = Crc32::Hash(text, Str::Length(text));

  for (;;) {
    entryIndex &= activeDescriptorCopy.data.hashTableSize - 1;

    const uint32_t offset =
        activeDescriptorCopy.data.reverseHashTable[entryIndex];
    switch (offset) {
    case OFFSET_EMPTY:
      return false;

    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry
               *)(activeDescriptorCopy.data.dataBlock + offset - OFFSET_DATA);

      if (entry == entryToDelete) {
        WriteReverseEntryIndex(entryIndex, OFFSET_DELETED);
        return entry;
      }
    }

    ++entryIndex;
  }
}

void StenoUserDictionary::WriteEntryIndex(size_t entryIndex, uint32_t offset) {
  const uint32_t *entry = &activeDescriptorCopy.data.hashTable[entryIndex];
  Flash::Write(entry, &offset, sizeof(offset), FlashWriteMode::PRESERVE);
}

void StenoUserDictionary::WriteReverseEntryIndex(size_t entryIndex,
                                                 uint32_t offset) {
  const uint32_t *entry =
      &activeDescriptorCopy.data.reverseHashTable[entryIndex];
  Flash::Write(entry, &offset, sizeof(offset), FlashWriteMode::PRESERVE);
}

void StenoUserDictionary::PrintDictionary(
    PrintDictionaryContext &context) const {
  for (size_t i = 0; i < activeDescriptorCopy.data.hashTableSize; ++i) {
    const uint32_t offset = activeDescriptorCopy.data.hashTable[i];
    switch (offset) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      break;

    default:
      const StenoUserDictionaryEntry *entry =
          (const StenoUserDictionaryEntry
               *)(activeDescriptorCopy.data.dataBlock + offset - OFFSET_DATA);

      context.Print(entry->strokes, entry->strokeLength, entry->GetText());
    }
  }
}

void StenoUserDictionary::PrintJsonDictionary() const {
  Console::Printf("{");
  ConsolePrintDictionaryContext context(GetName());
  PrintDictionary(context);
  Console::Printf("\n}\n\n");
}

const char *StenoUserDictionary::GetName() const { return "user_dictionary"; }

void StenoUserDictionary::PrintInfo(int depth) const {
  size_t hashTableUsed = 0;
  for (size_t i = 0; i < activeDescriptorCopy.data.hashTableSize; ++i) {
    switch (activeDescriptorCopy.data.hashTable[i]) {
    case OFFSET_EMPTY:
    case OFFSET_DELETED:
      break;

    default:
      ++hashTableUsed;
    }
  }

  Console::Printf("%s%s\n", Spaces(depth), GetName());

  const char *prefix = Spaces(depth + 2);
  Console::Printf("%sFormat version: %u\n", prefix,
                  activeDescriptorCopy.version);
  Console::Printf("%sHash table usage: %zu/%zu\n", prefix, hashTableUsed,
                  activeDescriptorCopy.data.hashTableSize);
  Console::Printf(
      "%sData block usage: %zu/%zu\n", prefix,
      activeDescriptorCopy.GetUsedDataBlockSize(layout.dataBlockSize),
      layout.dataBlockSize);
}

//---------------------------------------------------------------------------

void StenoUserDictionary::PrintJsonDictionary_Binding(void *context,
                                                      const char *commandLine) {
  const ExternalFlashSentry sentry;
  StenoUserDictionary *userDictionary = (StenoUserDictionary *)context;
  userDictionary->PrintJsonDictionary();
}

void StenoUserDictionary::Reset_Binding(void *context,
                                        const char *commandLine) {
  const ExternalFlashSentry sentry;
  StenoUserDictionary *userDictionary = (StenoUserDictionary *)context;
  userDictionary->Reset();
  userDictionary->OnLookupDataChanged();
  Console::SendOk();
}

void StenoUserDictionary::AddEntry_Binding(void *context,
                                           const char *commandLine) {
  const char *strokeStart = strchr(commandLine, ' ');
  if (!strokeStart) {
    Console::Printf("ERR No strokes specified\n\n");
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

  const ExternalFlashSentry sentry;
  StenoUserDictionary *userDictionary = (StenoUserDictionary *)context;
  if (!userDictionary->Add(parser.GetData(), parser.GetCount(),
                           translationStart)) {
    Console::Printf("ERR Unable to write to user dictionary\n\n");
    return;
  }

  Console::SendOk();
}

void StenoUserDictionary::RemoveEntry_Binding(void *context,
                                              const char *commandLine) {
  const char *strokeStart = strchr(commandLine, ' ');
  if (!strokeStart) {
    Console::Printf("ERR No strokes specified\n\n");
    return;
  }

  StrokeListParser parser;
  if (!parser.Parse(strokeStart + 1)) {
    Console::Printf("ERR Cannot parse stroke near %s\n\n", parser.failureOrEnd);
    return;
  }

  const ExternalFlashSentry sentry;
  StenoUserDictionary *userDictionary = (StenoUserDictionary *)context;
  userDictionary->Remove(parser.GetData(), parser.GetCount());
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

[[gnu::aligned(4096)]] static uint8_t userDictionaryBuffer[512 * 1024];

TEST_BEGIN("StenoUserDictionary will reset if descriptor is invalid") {
  const StenoUserDictionaryData layout(userDictionaryBuffer,
                                       sizeof(userDictionaryBuffer));

  const StenoUserDictionaryDescriptor *descriptor =
      (const StenoUserDictionaryDescriptor
           *)(userDictionaryBuffer + 512 * 1024 -
              StenoUserDictionaryData::ALL_DESCRIPTORS_SIZE);
  assert(descriptor == layout.GetDescriptor());

  for (size_t i = 0; i < sizeof(userDictionaryBuffer); ++i) {
    userDictionaryBuffer[i] = rand();
  }

  const StenoUserDictionary userDictionary(layout);
  assert(Flash::IsErased(userDictionaryBuffer, 64 * 1024));
  assert(Flash::IsErased(userDictionaryBuffer + 64 * 1024, 64 * 1024));
  assert(Flash::IsErased(userDictionaryBuffer + 128 * 1024,
                         384 * 1024 -
                             StenoUserDictionaryData::ALL_DESCRIPTORS_SIZE));
  assert(descriptor->data.hashTable == (void *)userDictionaryBuffer);
  assert(descriptor->data.reverseHashTable ==
         (void *)&userDictionaryBuffer[64 * 1024]);
  assert(descriptor->data.hashTableSize == 16 * 1024);
  assert(descriptor->data.dataBlock ==
         (void *)&userDictionaryBuffer[128 * 1024]);
  assert(descriptor->GetUsedDataBlockSize(layout.dataBlockSize) == 0);
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

extern int flashEraseCount;
TEST_BEGIN("StenoUserDictionary will not erase on each additions") {
  const StenoUserDictionaryData layout(userDictionaryBuffer,
                                       sizeof(userDictionaryBuffer));

  memset(userDictionaryBuffer, 0xff, sizeof(userDictionaryBuffer));

  flashEraseCount = 0;
  StenoUserDictionary userDictionary(layout);
  for (size_t i = 0; i < 256; ++i) {
    StenoStroke stroke((int)i);
    char buffer[16];
    MemoryWriter writer(buffer);
    writer.Printf("test%d", i);
    writer.WriteByte('\0');
    userDictionary.Add(&stroke, 1, buffer);
  }

  assert(flashEraseCount == 3);
  flashEraseCount = 0;

  for (size_t i = 256; i < 512; ++i) {
    StenoStroke stroke((int)i);
    char buffer[16];
    MemoryWriter writer(buffer);
    writer.Printf("test-%d", i);
    writer.WriteByte('\0');
    userDictionary.Add(&stroke, 1, buffer);
  }

  for (size_t i = 0; i < 512; ++i) {
    StenoStroke stroke((int)i);
    char buffer[16];

    MemoryWriter writer(buffer);
    writer.Printf(i < 256 ? "test%d" : "test-%d", i);
    writer.WriteByte('\0');
    assert(Str::Eq(userDictionary.Lookup(&stroke, 1).GetText(), buffer));
  }

  assert(flashEraseCount == 4);
}
TEST_END

#if RUN_TESTS

static void VerifyReverseLookup(StenoUserDictionary &userDictionary,
                                const char *text, StenoStroke expected) {
  StenoReverseDictionaryLookup lookup(text);
  userDictionary.ReverseLookup(lookup);
  assert(lookup.results.GetCount() == 1);

  const auto &result = lookup.results[0];
  assert(result.length == 1);
  assert(result.strokes[0] == expected);
}

static void VerifyNoReverseLookup(StenoUserDictionary &userDictionary,
                                  const char *text) {
  StenoReverseDictionaryLookup lookup(text);
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

TEST_BEGIN("StenoUserDictionary will not erase on every overwrite") {
  // spellchecker: disable
  const StenoStroke KAT[] = {StenoStroke("KAT")};
  const StenoStroke TKOG[] = {StenoStroke("TKOG")};

  const StenoUserDictionaryData layout(userDictionaryBuffer,
                                       sizeof(userDictionaryBuffer));

  memset(userDictionaryBuffer, 0xff, sizeof(userDictionaryBuffer));

  flashEraseCount = 0;
  StenoUserDictionary userDictionary(layout);
  userDictionary.Add(KAT, 1, "cat");
  userDictionary.Add(TKOG, 1, "dog");
  assert(flashEraseCount == 0);

  // Setup a series of writes where not all writes need an erase.
  userDictionary.Add(TKOG, 1, "pig");
  userDictionary.Add(TKOG, 1, "ant");
  userDictionary.Add(KAT, 1, "pig");
  assert(flashEraseCount == 2);

  assert(Str::Eq(userDictionary.Lookup(KAT, 1).GetText(), "pig"));
  assert(Str::Eq(userDictionary.Lookup(TKOG, 1).GetText(), "ant"));
  // spellchecker: enable

  VerifyReverseLookup(userDictionary, "pig", StenoStroke("KAT"));
  VerifyReverseLookup(userDictionary, "ant", StenoStroke("TKOG"));
}
TEST_END

#endif

//---------------------------------------------------------------------------
