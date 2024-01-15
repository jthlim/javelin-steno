//---------------------------------------------------------------------------

#pragma once
#include "../flash.h"
#include "dictionary.h"
#include <assert.h>

//---------------------------------------------------------------------------

class Console;
struct StenoUserDictionaryDescriptor;
struct StenoUserDictionaryEntry;

struct StenoUserDictionaryData {
  StenoUserDictionaryData();
  StenoUserDictionaryData(const uint8_t *mem, size_t size) {
    assert((size & (size - 1)) == 0);
    hashTable = (uint32_t *)mem;
    reverseHashTable = (const uint32_t *)(mem + size / 8);
    hashTableSize = size / 32;
    dataBlock = mem + size / 4;
    dataBlockSize = 3 * size / 4 - Flash::BLOCK_SIZE;
    maximumOutlineLength = 0;
  }

  const uint32_t *hashTable;
  size_t hashTableSize; // Number of uint32_t, not number of bytes
  const uint8_t *dataBlock;
  size_t dataBlockSize;
  uint32_t maximumOutlineLength;

  // New field in reverseLookupVersion
  const uint32_t *reverseHashTable;

  const StenoUserDictionaryDescriptor *GetDescriptor() const {
    return (const StenoUserDictionaryDescriptor *)(dataBlock + dataBlockSize);
  }
};

struct StenoUserDictionaryDescriptor {
  uint32_t magic;
  uint32_t version;
  StenoUserDictionaryData data;
  uint32_t crc32;

  bool IsValid(const StenoUserDictionaryData &layout) const;
  void UpdateCrc32();
};

//---------------------------------------------------------------------------

class StenoUserDictionary final : public StenoDictionary {
public:
  StenoUserDictionary(const StenoUserDictionaryData &layout);

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const final;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  virtual const char *GetName() const final;
  virtual void PrintInfo(int depth) const final;
  virtual bool PrintDictionary(const char *name, bool hasData) const final;

  void PrintJsonDictionary() const;
  void Reset();

  // Returns true if successful.
  virtual bool Add(const StenoStroke *strokes, size_t length, const char *word);

  // Returns true if successful.
  virtual bool Remove(const StenoStroke *strokes, size_t length);

  static void PrintJsonDictionary_Binding(void *context,
                                          const char *commandLine);
  static void Reset_Binding(void *context, const char *commandLine);
  static void AddEntry_Binding(void *context, const char *commandLine);
  static void RemoveEntry_Binding(void *context, const char *commandLine);

  static const size_t MAX_STROKE_COUNT = 16;

  void AddConsoleCommands(Console &console);

private:
  const StenoUserDictionaryDescriptor *descriptorBase;
  const StenoUserDictionaryDescriptor *activeDescriptor;
  const StenoUserDictionaryData &layout;

  struct AddToDataBlockResult {
    AddToDataBlockResult(size_t offset, size_t length)
        : offset(offset), length(length) {}

    size_t offset;
    size_t length;
  };

  AddToDataBlockResult AddToDataBlock(const StenoStroke *strokes,
                                      uint32_t length, const char *word);
  void AddToDescriptor(size_t strokeLength, size_t dataLength);
  bool AddToHashTable(const StenoStroke *strokes, size_t length, size_t offset);
  bool AddToReverseHashTable(const char *word, size_t offset);
  void WriteEntryIndex(size_t entryIndex, size_t offset);
  void WriteReverseEntryIndex(size_t entryIndex, size_t offset);

  const StenoUserDictionaryEntry *
  RemoveFromHashTable(const StenoStroke *strokes, size_t length);

  bool RemoveFromReverseHashTable(const StenoUserDictionaryEntry *entry);

  const StenoUserDictionaryDescriptor *FindMostRecentDescriptor() const;
  size_t GetNextDescriptorToWriteOffset() const;

  void UpgradeToVersionWithReverseLookup();
};

//---------------------------------------------------------------------------
