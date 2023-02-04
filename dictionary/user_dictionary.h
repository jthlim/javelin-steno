//---------------------------------------------------------------------------

#pragma once
#include "../flash.h"
#include "dictionary.h"
#include <assert.h>

//---------------------------------------------------------------------------

class Console;
struct StenoUserDictionaryDescriptor;

struct StenoUserDictionaryData {
  StenoUserDictionaryData();
  StenoUserDictionaryData(const uint8_t *mem, size_t size) {
    assert((size & (size - 1)) == 0);
    hashTable = (uint32_t *)mem;
    hashTableSize = size / 32;
    dataBlock = mem + size / 8;
    dataBlockSize = size - 4 * hashTableSize - Flash::BLOCK_SIZE;
    maximumOutlineLength = 0;
  }

  const uint32_t *hashTable;
  size_t hashTableSize; // Number of uint32_t, not number of bytes
  const uint8_t *dataBlock;
  size_t dataBlockSize;
  uint32_t maximumOutlineLength;

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
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual size_t GetMaximumOutlineLength() const final;
  virtual const char *GetName() const final;
  virtual void PrintInfo(int depth) const final;
  virtual bool PrintDictionary(bool hasData) const final;

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
  void WriteEntryIndex(size_t entryIndex, size_t offset);

  const StenoUserDictionaryDescriptor *FindMostRecentDescriptor() const;
  size_t GetNextDescriptorToWriteOffset() const;
};

//---------------------------------------------------------------------------
