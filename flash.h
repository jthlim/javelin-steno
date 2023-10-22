//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

class Flash {
public:
  static bool IsUpdating() { return instance.target != nullptr; }

  // Sets all bits in the region to 1.
  static void Erase(const void *target, size_t size);

  static bool IsErased(const void *target, size_t size) {
    return !RequiresErase(target, size);
  }

  static void Write(const void *target, const void *data, size_t size);

  static const size_t BLOCK_SIZE = 4096;

  static void PrintInfo();

  static void BeginWriteBinding(void *context, const char *commandLine);
  static void WriteBinding(void *context, const char *commandLine);
  static void EndWriteBinding(void *context, const char *commandLine);

  static void AddConsoleCommands(Console &console);

private:
  static bool RequiresErase(const void *target, size_t size);
  static bool RequiresErase(const void *target, const void *data, size_t size);
  static bool RequiresProgram(const void *target, const void *data,
                              size_t size);

  static void EraseInternal(const void *target, size_t size);
  static void WriteInternal(const void *target, const void *data, size_t size);

  void BeginWrite(const uint8_t *address);
  void AddData(const uint8_t *data, size_t length);
  void WriteRemaining();

  static const size_t WRITE_DATA_BUFFER_SIZE = 4096;

  size_t erasedBytes;
  size_t programmedBytes;
  size_t reprogrammedBytes;

  const uint8_t *target;
  uint8_t buffer[WRITE_DATA_BUFFER_SIZE];

  static Flash instance;
};

//---------------------------------------------------------------------------
