//---------------------------------------------------------------------------

#include "asset_manager.h"
#include "base64.h"
#include "console.h"
#include "crc32.h"
#include "flash.h"
#include "mem.h"
#include "str.h"

//---------------------------------------------------------------------------

const uint32_t ASSET_DIRECTORY_MAGIC = 0x4A414431; // "JAD1"
const size_t MINIMUM_ASSET_DIRECTORY_SIZE = 2 * Flash::BLOCK_SIZE;

//---------------------------------------------------------------------------

AssetManager AssetManager::instance;

//---------------------------------------------------------------------------

const void *AssetEntry::GetData() const {
  const size_t idLength = Str::Length(id);
  const size_t paddedIdLength = (idLength + 4) & -4;
  return id + paddedIdLength;
}

//---------------------------------------------------------------------------

bool AssetDirectory::IsValid() const { return magic == ASSET_DIRECTORY_MAGIC; }

size_t AssetDirectory::GetAssetCount() const {
  size_t count = 0;
  if (IsValid()) {
    for (intptr_t value : hashTable) {
      if (value != 0 && value != -1) {
        ++count;
      }
    }
  }
  return count;
}

const AssetEntry *AssetDirectory::GetAsset(const char *id) const {
  const size_t idLength = Str::Length(id);
  const uint32_t hash = Crc32::Hash(id, idLength);
  uint32_t index = hash;
  for (;;) {
    const intptr_t value = hashTable[index & HASH_TABLE_MASK];
    if (value == -1) {
      return nullptr;
    }

    if (value != 0) {
      const AssetEntry *entry = (const AssetEntry *)value;
      if (entry->idHash == hash && Str::Eq(entry->id, id)) {
        return entry;
      }
    }

    ++index;
  }
}

void AssetDirectory::ListAssets() const {
  Console::Printf("[");
  bool isFirst = true;
  for (intptr_t value : hashTable) {
    if (value == 0 || value == -1) {
      continue;
    }

    const AssetEntry *entry = (const AssetEntry *)value;
    const char *formatString = ",{\"id\":\"%J\",\"size\":%u}";
    Console::Printf(formatString + isFirst, entry->id, entry->size);
    isFirst = false;
  }
  Console::Printf("]\n\n");
}

//---------------------------------------------------------------------------

void AssetManager::Initialize(intptr_t directoryAddress, intptr_t directorySize,
                              uint32_t timestamp) {
  instance.directory = (const AssetDirectory *)directoryAddress;
  instance.directorySize = directorySize;
  instance.timestamp = timestamp;
}

bool AssetManager::IsValid() const {
  return directory != nullptr && directory->IsValid() &&
         directorySize >= MINIMUM_ASSET_DIRECTORY_SIZE &&
         directory->timestamp == timestamp;
}

const AssetEntry *AssetManager::GetAsset(const char *id) {
  if (!instance.IsValid()) {
    return nullptr;
  }

  const AssetEntry *entry = instance.directory->GetAsset(id);
  if (entry == nullptr) {
    return nullptr;
  }
  return entry;
}

const void *AssetManager::GetAssetData(const char *id) {
  const AssetEntry *entry = GetAsset(id);
  if (entry == nullptr) {
    return nullptr;
  }
  return entry->GetData();
}

intptr_t AssetManager::GetEndEmptyDataRegion() const {
  intptr_t endEmptyRegion = intptr_t(directory) + directorySize;
  if (IsValid()) {
    for (intptr_t value : directory->hashTable) {
      if (value != 0 && value != -1) {
        if (value < endEmptyRegion) {
          endEmptyRegion = value;
        }
      }
    }
  }
  return endEmptyRegion;
}

size_t AssetManager::GetFreeSize() const {
  if (directory == nullptr || directorySize < MINIMUM_ASSET_DIRECTORY_SIZE) {
    return 0;
  }
  const intptr_t endEmptyRegion = GetEndEmptyDataRegion();
  const intptr_t startDataRegion = (intptr_t)directory->GetStartDataRegion();
  return endEmptyRegion - startDataRegion;
}

//---------------------------------------------------------------------------

const char *AssetManager::AddAsset(const char *id, size_t size) {
  if (!IsValid()) {
    ResetHeader();
  }

  if (directory->GetAssetCount() >= AssetDirectory::MAX_ASSET_COUNT) {
    return "Too many assets";
  }

  const size_t idLength = Str::Length(id);
  if (idLength > 248) {
    return "Asset name too long";
  }

  const size_t paddedAssetEntryLength = (idLength + sizeof(AssetEntry)) & -4;
  const size_t paddedDataSize = (size + 3) & -4;
  const size_t totalSize = paddedAssetEntryLength + paddedDataSize;
  if (totalSize > GetFreeSize()) {
    return "Insufficient free space";
  }

  const intptr_t endEmptyRegion = GetEndEmptyDataRegion();
  const intptr_t startWriteAddress = endEmptyRegion - totalSize;

  uint8_t buffer[256] = {};
  AssetEntry *entry = (AssetEntry *)buffer;
  entry->size = (uint32_t)size;
  entry->idHash = Crc32::Hash(id, idLength);
  Mem::Copy(entry->id, id, idLength);

  Flash::instance.BeginWrite((uint8_t *)startWriteAddress);
  Flash::instance.AddData(buffer, paddedAssetEntryLength);
  assetDataBytesRemaining = (uint32_t)size;

  return nullptr;
}

void AssetManager::BeginWrite(const uint8_t *address) {
  Flash::instance.target = address;
  Flash::instance.writeStart = address;

  const size_t offsetIntoPage =
      size_t(address) & (Flash::WRITE_DATA_BUFFER_SIZE - 1);
  Mem::Fill(Flash::instance.buffer, offsetIntoPage);

  const uint8_t *baseAddress =
      (const uint8_t *)(intptr_t(address) & -Flash::WRITE_DATA_BUFFER_SIZE);
  if (IsValid() &&
      (uint8_t *)directory + sizeof(AssetDirectory) > baseAddress) {
    const size_t copyBytes =
        (uint8_t *)directory + sizeof(AssetDirectory) - baseAddress;
    Mem::Copy(Flash::instance.buffer, baseAddress,
              copyBytes & (Flash::WRITE_DATA_BUFFER_SIZE - 1));
  }
}

void AssetManager::ResetHeader() {
  AssetDirectory *newDirectory = new AssetDirectory;
  newDirectory->magic = ASSET_DIRECTORY_MAGIC;
  newDirectory->timestamp = timestamp;
  Mem::Fill(newDirectory->hashTable);

  Flash::Write(directory, newDirectory, sizeof(AssetDirectory),
               FlashWriteMode::RESET);

  delete newDirectory;
}

void AssetManager::WriteHeader() {
  const AssetEntry *newEntry = (AssetEntry *)Flash::instance.writeStart;
  const uint32_t hash = newEntry->idHash;

  uint32_t index = hash;
  for (;;) {
    index &= AssetDirectory::HASH_TABLE_MASK;
    const intptr_t value = directory->hashTable[index];
    if (value == 0 || value == -1) {
      Flash::Write(&directory->hashTable[index], &newEntry,
                   sizeof(AssetEntry *), FlashWriteMode::PRESERVE);
      return;
    }

    const AssetEntry *entry = (const AssetEntry *)value;
    if (entry->idHash == hash && Str::Eq(entry->id, newEntry->id)) {
      Flash::Write(&directory->hashTable[index], &newEntry,
                   sizeof(AssetEntry *), FlashWriteMode::PRESERVE);
      return;
    }

    ++index;
  }
}

bool AssetManager::Write(const uint8_t *data, size_t byteCount) {
  Flash::instance.AddData(data, byteCount);

  assetDataBytesRemaining -= byteCount;
  if (assetDataBytesRemaining != 0) {
    return false;
  }

  Flash::instance.WriteRemaining();
  WriteHeader();
  return true;
}

//---------------------------------------------------------------------------

void AssetManager::PrintFreeSize() {
  Console::Printf("0x%x\n\n", instance.GetFreeSize());
}

void AssetManager::AddAsset_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR No size specified\n\n");
    return;
  }
  ++p;
  int size;
  p = Str::ParseInteger(&size, p);
  if (!p) {
    Console::Printf("ERR Unable to parse size\n\n");
    return;
  }
  if (*p != ' ') {
    Console::Printf("ERR No id specified\n\n");
    return;
  }
  ++p;

  AssetManager *instance = (AssetManager *)context;
  const char *errorMessage = instance->AddAsset(p, size);
  if (errorMessage) {
    Console::Printf("ERR %s\n\n", errorMessage);
    return;
  }

  Console::SendOk();
}

void AssetManager::AssetData_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing data\n\n");
    return;
  }

  if (Flash::instance.target == nullptr) {
    Console::Printf("ERR No write in progress\n\n");
    return;
  }

  uint8_t decodeBuffer[256];
  const size_t byteCount = Base64::Decode(decodeBuffer, (const uint8_t *)p);

  if (byteCount == 0) {
    Console::Printf("ERR No data\n\n");
    return;
  }

  AssetManager *instance = (AssetManager *)context;
  if (byteCount > instance->assetDataBytesRemaining) {
    Console::Printf("ERR Too much data\n\n");
    Flash::instance.target = nullptr;
    return;
  }

  if (!instance->Write(decodeBuffer, byteCount)) {
    Console::SendOk();
  } else {
    Console::Printf("OK Completed\n\n");
  }
}

void AssetManager::GetAsset_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR No assetId specified\n\n");
    return;
  }
  ++p;
  const AssetManager *instance = (AssetManager *)context;
  const AssetEntry *entry = instance->GetAsset(p);
  if (entry == nullptr) {
    Console::Printf("ERR assetId not found\n\n");
    return;
  }
  Console::Printf("%D\n\n", entry->GetData(), entry->size);
}

void AssetManager::ListAssets_Binding(void *context, const char *commandLine) {
  const AssetManager *instance = (AssetManager *)context;
  if (!instance->IsValid()) {
    Console::Printf("[]\n\n");
    return;
  }

  instance->directory->ListAssets();
}

void AssetManager::ResetAssets_Binding(void *context, const char *commandLine) {
  AssetManager *instance = (AssetManager *)context;
  if (instance->directory == nullptr) {
    Console::Printf("ERR Unable to reset assets\n\n");
    return;
  }
  instance->ResetHeader();
  Console::SendOk();
}

void AssetManager::AddConsoleCommands(Console &console) {
  console.RegisterCommand("list_assets", "Lists all assets on the device",
                          &ListAssets_Binding, &instance);
  console.RegisterCommand("get_asset", "Gets asset with specified id",
                          &GetAsset_Binding, &instance);
  console.RegisterCommand("add_asset", "Adds an asset to the device",
                          &AddAsset_Binding, &instance);
  console.RegisterCommand(
      "asset_data", "Uploads asset_data for the previous add_asset command",
      &AssetData_Binding, &instance);
  console.RegisterCommand("reset_assets", "Adds an asset to the device",
                          &ResetAssets_Binding, &instance);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"

//---------------------------------------------------------------------------

class AssetManagerTest {
public:
  static void TestAddition();
};

[[gnu::aligned(4096)]] static char assetData[16 * 1024];

void AssetManagerTest::TestAddition() {
  char *buffer = assetData;
  memset(buffer, 0, 16384);

  AssetManager::Initialize(intptr_t(buffer), 16384, 0);
  assert(!AssetManager::instance.IsValid());

  assert(AssetManager::instance.AddAsset("test_id", 64) == nullptr);

  uint8_t data[64];
  for (int i = 0; i < 64; ++i) {
    data[i] = i;
  }
  AssetManager::instance.Write(data, 64);

  const AssetEntry *entry = AssetManager::instance.GetAsset("test_id");
  assert(entry != nullptr);
  assert(entry->size == 64);
  assert(memcmp(entry->GetData(), data, 64) == 0);

  // Second id addition.
  assert(AssetManager::instance.AddAsset("test_id2", 64) == nullptr);
  AssetManager::instance.Write(data, 64);

  entry = AssetManager::instance.GetAsset("test_id2");
  assert(entry != nullptr);
  assert(entry->size == 64);
  assert(memcmp(entry->GetData(), data, 64) == 0);
}

TEST_BEGIN("AssetManager: Test addition of asset") {
  AssetManagerTest::TestAddition();
}
TEST_END

//---------------------------------------------------------------------------
