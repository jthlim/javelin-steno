//---------------------------------------------------------------------------

#pragma once
#include "../flash.h"
#include "dictionary.h"
#include <assert.h>

//---------------------------------------------------------------------------

class Console;
struct StenoUserDictionaryDescriptor;
struct StenoUserDictionaryEntry;

//---------------------------------------------------------------------------

struct StenoUserDictionaryData {
  StenoUserDictionaryData() = default;
  constexpr StenoUserDictionaryData(const uint8_t *mem, size_t size) {
    assert((size & (size - 1)) == 0);
    hashTable = (uint32_t *)mem;
    reverseHashTable = (const uint32_t *)(mem + size / 8);
    hashTableSize = size / 32;
    dataBlock = mem + size / 4;
    dataBlockSize = 3 * size / 4 - ALL_DESCRIPTORS_SIZE;
    maximumOutlineLength = 0;
  }

  const uint32_t *hashTable;
  size_t hashTableSize; // Number of uint32_t, not number of bytes
  const uint8_t *dataBlock;
  union {
    size_t dataBlockSize;
    size_t dataBlockSizeRemaining;
  };
  uint32_t maximumOutlineLength;
  const uint32_t *reverseHashTable;

  uint32_t Crc32() const;

  static const uint32_t ALL_DESCRIPTORS_SIZE = 2 * Flash::BLOCK_SIZE;

  const void *GetDataStart() const { return hashTable; }
  size_t GetDataLength() const {
    return (char *)GetDescriptor(0) + ALL_DESCRIPTORS_SIZE - (char *)hashTable;
  }

  const StenoUserDictionaryDescriptor *
  GetDescriptor(size_t byteOffset = 0) const {
    return (const StenoUserDictionaryDescriptor *)(dataBlock + dataBlockSize +
                                                   byteOffset);
  }

  const StenoUserDictionaryDescriptor *FindMostRecentDescriptor() const;
};

struct StenoUserDictionaryDescriptor {
  uint32_t magic;
  uint32_t version;
  StenoUserDictionaryData data;
  uint32_t crc32;

  bool IsValid(const StenoUserDictionaryData &layout) const;
  void UpdateCrc32();

  size_t GetUsedDataBlockSize(size_t totalDataBlockSize) const;
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

  virtual void
  PrintEntriesWithPartialOutline(PrintPartialOutlineContext &context) const;

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual const char *GetName() const final;
  virtual void PrintInfo(int depth) const final;
  virtual void PrintDictionary(PrintDictionaryContext &context) const final;

  void PrintJsonDictionary() const;
  void Reset();
  void DestroyDescriptorBlock();

  // Returns true if successful.
  virtual bool Add(const StenoStroke *strokes, size_t length, const char *word);

  virtual bool CanRemove() const { return true; }
  virtual bool Remove(const char *dictionaryName, const StenoStroke *strokes,
                      size_t length) {
    return Remove(strokes, length);
  }

  // Returns true if successful.
  bool Remove(const StenoStroke *strokes, size_t length);

  static void PrintJsonDictionary_Binding(void *context,
                                          const char *commandLine);
  static void Reset_Binding(void *context, const char *commandLine);
  static void AddEntry_Binding(void *context, const char *commandLine);
  static void RemoveEntry_Binding(void *context, const char *commandLine);

  static constexpr size_t MAX_STROKE_COUNT = 16;

  void AddConsoleCommands(Console &console);

private:
  StenoUserDictionaryDescriptor activeDescriptorCopy;
  const StenoUserDictionaryDescriptor *descriptorBase;
  const StenoUserDictionaryDescriptor *activeDescriptor;
  const StenoUserDictionaryData &layout;

  struct AddToDataBlockResult {
    AddToDataBlockResult(size_t offset, size_t length)
        : offset(offset), length(length) {}

    const size_t offset;
    const size_t length;
  };

  AddToDataBlockResult AddToDataBlock(const StenoStroke *strokes,
                                      uint32_t length, const char *word);
  void AddToDescriptor(size_t strokeLength,
                       AddToDataBlockResult dataBlockResult);
  bool AddToHashTable(const StenoStroke *strokes, size_t length, size_t offset);
  bool AddToReverseHashTable(const char *word, size_t offset);
  void WriteEntryIndex(size_t entryIndex, uint32_t offset);
  void WriteReverseEntryIndex(size_t entryIndex, uint32_t offset);

  const StenoUserDictionaryEntry *
  LookupEntry(const StenoDictionaryLookup &lookup) const;

  const StenoUserDictionaryEntry *
  RemoveFromHashTable(const StenoStroke *strokes, size_t length);

  bool RemoveFromReverseHashTable(const StenoUserDictionaryEntry *entry);

  const StenoUserDictionaryDescriptor *FindMostRecentDescriptor() const;
  size_t GetNextDescriptorToWriteOffset() const;
};

//---------------------------------------------------------------------------
