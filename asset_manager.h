//---------------------------------------------------------------------------

#pragma once
#include "malloc_allocate.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

struct AssetEntry {
  uint32_t size;
  uint32_t idHash;
  char id[1];

  const void *GetData() const;
};

struct AssetDirectory : public JavelinMallocAllocate {
  static const size_t HASH_TABLE_SIZE = 1024;
  static const size_t HASH_TABLE_MASK = HASH_TABLE_SIZE - 1;
  static const size_t MAX_ASSET_COUNT = 512;

  uint32_t magic;
  uint32_t timestamp;
  intptr_t hashTable[HASH_TABLE_SIZE];

  bool IsValid() const;
  size_t GetAssetCount() const;
  const AssetEntry *GetAsset(const char *id) const;

  void ListAssets() const;

  const void *GetStartDataRegion() const { return hashTable + HASH_TABLE_SIZE; }
};

//---------------------------------------------------------------------------

class AssetManager {
public:
  static void Initialize(intptr_t directoryAddress, intptr_t directorySize,
                         uint32_t timestamp);

  static const AssetEntry *GetAsset(const char *id);
  static const void *GetAssetData(const char *id);

  static void AddConsoleCommands(Console &console);

  size_t GetFreeSize() const;

  static void PrintFreeSize();

private:
  bool IsValid() const;

  const AssetDirectory *directory;
  intptr_t directorySize;
  uint32_t timestamp;
  uint32_t assetDataBytesRemaining;

  intptr_t GetEndEmptyDataRegion() const;

  // Returns error message, or nullptr if successful.
  const char *AddAsset(const char *id, size_t size);

  void BeginWrite(const uint8_t *address);
  bool Write(const uint8_t *data, size_t byteCount);
  void ResetHeader();
  void WriteHeader();

  static void AddAsset_Binding(void *context, const char *commandLine);
  static void AssetData_Binding(void *context, const char *commandLine);
  static void GetAsset_Binding(void *context, const char *commandLine);
  static void ListAssets_Binding(void *context, const char *commandLine);
  static void ResetAssets_Binding(void *context, const char *commandLine);

  static AssetManager instance;

  friend class AssetManagerTest;
};

//---------------------------------------------------------------------------
