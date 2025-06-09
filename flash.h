//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

enum class FlashWriteMode {
  // When writing to a page and an erase is required, preserve all data
  // outside of the write region.
  PRESERVE,

  // When writing to a page and an erase is required, preserve all data
  // before (lower memory address) the write region, and reset (to 0xff)
  // data after.
  PRESERVE_BEFORE,

  // When writing to a page and an erase is required, preserve all data
  // after (higher memory address) the write region, and reset (to 0xff)
  // data before.
  PRESERVE_AFTER,

  // When writing to a page and an erase is required, reset (to 0xff)
  // all data outside of the write region.
  RESET,
};

//---------------------------------------------------------------------------

class Flash {
public:
  static bool IsUpdating() { return instance.writeInProgress; }

  // Sets all bits in the region to 1.
  static void EraseBlock(const void *target, size_t size);
  static void EraseBlock(const void *target, const void *data, size_t size);

  static bool IsErased(const void *target, size_t size) {
    return !RequiresErase(target, size);
  }

  static void WriteBlock(const void *target, const void *data, size_t size);
  static void Write(const void *target, const void *data, size_t size,
                    FlashWriteMode writeMode);

  static constexpr size_t BLOCK_SIZE = 4096;

  static void PrintInfo();

  static void BeginWriteBinding(void *context, const char *commandLine);
  static void WriteBinding(void *context, const char *commandLine);
  static void EndWriteBinding(void *context, const char *commandLine);

  static void AddConsoleCommands(Console &console);

  bool IsScriptMemory(const void *start, const void *end);

  static bool IsWriteEnabled() { return !instance.isLocked; }
  static void EnableWrite() { instance.isLocked = false; }
  static void DisableWrite() { instance.isLocked = true; }

  static bool RequiresErase(const void *target, size_t size);
  static bool RequiresErase(const void *target, const void *data, size_t size);
  static bool RequiresProgram(const void *target, const void *data,
                              size_t size);

private:
  static void EraseBlockInternal(const void *target, size_t size);
  static void WriteBlockInternal(const void *target, const void *data,
                                 size_t size);

  void BeginWrite(const uint8_t *address);
  void AddData(const uint8_t *data, size_t length);
  void WriteRemaining();

  static constexpr size_t WRITE_DATA_BUFFER_SIZE = BLOCK_SIZE;

  bool isLocked = false;
  bool writeInProgress = false;
  size_t erasedBytes;
  size_t programmedBytes;
  size_t reprogrammedBytes;

  const uint8_t *target;
  const void *writeStart;
  uint8_t buffer[WRITE_DATA_BUFFER_SIZE];

  static Flash instance;

  friend class AssetManager;
};

//---------------------------------------------------------------------------
