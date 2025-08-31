//---------------------------------------------------------------------------

#include "flash.h"
#include "base64.h"
#include "button_script_manager.h"
#include "console.h"
#include "hal/external_flash.h"
#include "unicode.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

Flash Flash::instance;

//---------------------------------------------------------------------------

#if RUN_TESTS
int flashEraseCount = 0;
int flashWriteCount = 0;

[[gnu::weak]] void Flash::EraseBlockInternal(const void *target, size_t size) {
  assert((intptr_t(target) & (BLOCK_SIZE - 1)) == 0);
  assert((size & (BLOCK_SIZE - 1)) == 0);

  instance.erasedBytes += size;
  memset((void *)target, 0xff, size);
  ++flashEraseCount;
}

[[gnu::weak]] void Flash::WriteBlockInternal(const void *target,
                                             const void *data, size_t size) {
  assert(target != data);
  assert((intptr_t(target) & 255) == 0);
  assert((size & 255) == 0);

  instance.erasedBytes += size;
  instance.programmedBytes += size;
  memcpy((void *)target, data, size);
  ++flashWriteCount;
}
#endif

//---------------------------------------------------------------------------

bool Flash::RequiresErase(const void *target, size_t size) {
  assert((intptr_t(target) & (sizeof(size_t) - 1)) == 0);
  assert((size & (sizeof(size_t) - 1)) == 0);

  const size_t *p = (const size_t *)target;
  const size_t *end = (const size_t *)(intptr_t(target) + size);

  while (p < end) {
    if (*p++ != (size_t)-1) {
      return true;
    }
  }
  return false;
}

bool Flash::RequiresErase(const void *target, const void *data, size_t size) {
  assert((intptr_t(target) & (sizeof(size_t) - 1)) == 0);
  assert((intptr_t(data) & (sizeof(size_t) - 1)) == 0);
  assert((size & (sizeof(size_t) - 1)) == 0);

  const size_t *t = (const size_t *)target;
  const size_t *d = (const size_t *)data;
  const size_t *dEnd = (const size_t *)(intptr_t(data) + size);

  while (d < dEnd) {
    // Programming lowers a bit from 1 to 0, erasing raises it to 1.
    // Erasing is required if the data needs a 1, but the current state is a 0.
    if (*d++ & ~*t++) {
      return true;
    }
  }

  return false;
}

bool Flash::RequiresProgram(const void *target, const void *data, size_t size) {
  return memcmp(target, data, size) != 0;
}

//---------------------------------------------------------------------------

void Flash::BeginWrite(const uint8_t *address) {
  target = address;
  writeStart = address;
  writeInProgress = true;

  const void *baseAddress =
      (const void *)(intptr_t(address) & -WRITE_DATA_BUFFER_SIZE);
  const size_t offsetIntoPage = size_t(address) & (WRITE_DATA_BUFFER_SIZE - 1);
  memcpy(buffer, baseAddress, offsetIntoPage);
}

void Flash::AddData(const uint8_t *data, size_t length) {
  size_t offset = intptr_t(target) & (WRITE_DATA_BUFFER_SIZE - 1);
  if (offset + length >= WRITE_DATA_BUFFER_SIZE) {
    const size_t remainingBufferLength = WRITE_DATA_BUFFER_SIZE - offset;
    memcpy(buffer + offset, data, remainingBufferLength);

    ExternalFlash::Begin();
    const void *writeAddress =
        (const void *)(size_t(target) & -WRITE_DATA_BUFFER_SIZE);
    Flash::WriteBlock(writeAddress, buffer, WRITE_DATA_BUFFER_SIZE);
    ExternalFlash::End();

    target += remainingBufferLength;
    length -= remainingBufferLength;
    data += remainingBufferLength;
    offset = 0;
  }
  memcpy(buffer + offset, data, length);
  target += length;
}

void Flash::WriteRemaining() {
  const size_t bytesToWrite = size_t(target) & (WRITE_DATA_BUFFER_SIZE - 1);
  if (bytesToWrite != 0) {
    const ExternalFlashSentry sentry;

    // Copy the remainder
    const size_t bytesToCopy = WRITE_DATA_BUFFER_SIZE - bytesToWrite;
    memcpy(buffer + bytesToWrite, target, bytesToCopy);

    const void *writeAddress =
        (const void *)(size_t(target) & -WRITE_DATA_BUFFER_SIZE);

    Flash::WriteBlock(writeAddress, buffer, WRITE_DATA_BUFFER_SIZE);
  }

  target = nullptr;
  writeInProgress = false;
}

[[gnu::noinline]] void Flash::Write(const void *target, const void *data,
                                    size_t size, FlashWriteMode writeMode) {
  while (size > 0) {
    const uint8_t *baseAddress =
        (const uint8_t *)(intptr_t(target) & -WRITE_DATA_BUFFER_SIZE);
    const size_t offsetIntoPage = size_t(target) & (WRITE_DATA_BUFFER_SIZE - 1);
    memcpy(instance.buffer, baseAddress, offsetIntoPage);

    const size_t bytesRemainingInPage = WRITE_DATA_BUFFER_SIZE - offsetIntoPage;
    const size_t copyBytes =
        size < bytesRemainingInPage ? size : bytesRemainingInPage;
    memcpy(instance.buffer + offsetIntoPage, data, copyBytes);

    const size_t bytesAfter = bytesRemainingInPage - copyBytes;
    const size_t offsetAfter = offsetIntoPage + copyBytes;
    memcpy(instance.buffer + offsetAfter, baseAddress + offsetAfter,
           bytesAfter);

    if (Flash::RequiresErase(baseAddress, instance.buffer,
                             WRITE_DATA_BUFFER_SIZE)) {
      switch (writeMode) {
      case FlashWriteMode::PRESERVE:
        break;
      case FlashWriteMode::PRESERVE_BEFORE:
        Mem::Fill(instance.buffer + offsetAfter, bytesAfter);
        break;
      case FlashWriteMode::PRESERVE_AFTER:
        Mem::Fill(instance.buffer, offsetIntoPage);
        break;
      case FlashWriteMode::RESET:
        Mem::Fill(instance.buffer, offsetIntoPage);
        Mem::Fill(instance.buffer + offsetAfter, bytesAfter);
        break;
      }
    }
    WriteBlock(baseAddress, instance.buffer, WRITE_DATA_BUFFER_SIZE);

    target = (const uint8_t *)target + copyBytes;
    data = (const uint8_t *)data + copyBytes;
    size -= copyBytes;
  }
}

void Flash::EraseBlock(const void *target, size_t size) {
  const uint8_t *eraseStart = nullptr;
#if RUN_TESTS
  size_t eraseSize = 0;
#else
  size_t eraseSize;
#endif

  const uint8_t *const t = (const uint8_t *)target;
  for (size_t i = 0; i < size; i += 4096) {
    if (RequiresErase(t + i, 4096)) {
      if (eraseStart == nullptr) {
        eraseStart = t + i;
        eraseSize = 4096;
      } else {
        eraseSize += 4096;
      }
    } else if (eraseStart != nullptr) {
      EraseBlockInternal(eraseStart, eraseSize);
      eraseStart = nullptr;
    }
  }

  if (eraseStart != nullptr) {
    EraseBlockInternal(eraseStart, eraseSize);
  }
}

void Flash::EraseBlock(const void *target, const void *data, size_t size) {
  const uint8_t *const t = (const uint8_t *)target;
  const uint8_t *const d = (const uint8_t *)data;

  const uint8_t *eraseStart = nullptr;
#if RUN_TESTS
  size_t eraseSize = 0;
#else
  size_t eraseSize;
#endif

  for (size_t i = 0; i < size; i += 4096) {
    if (RequiresErase(t + i, d + i, 4096)) {
      if (eraseStart == nullptr) {
        eraseStart = t + i;
        eraseSize = 4096;
      } else {
        eraseSize += 4096;
      }
    } else if (eraseStart != nullptr) {
      EraseBlockInternal(eraseStart, eraseSize);
      eraseStart = nullptr;
    }
  }

  if (eraseStart != nullptr) {
    EraseBlockInternal(eraseStart, eraseSize);
  }
}

void Flash::WriteBlock(const void *const target, const void *const data,
                       const size_t size) {
  EraseBlock(target, data, size);

  const uint8_t *const t = (const uint8_t *)target;
  const uint8_t *const d = (const uint8_t *)data;

  const uint8_t *programTargetStart = nullptr;
  const uint8_t *programSourceStart;
#if RUN_TESTS
  size_t programSize = 0;
#else
  size_t programSize;
#endif

  for (size_t i = 0; i < size; i += 256) {
    if (RequiresProgram(t + i, d + i, 256)) {
      if (programTargetStart == nullptr) {
        programTargetStart = t + i;
        programSourceStart = d + i;
        programSize = 256;
      } else {
        programSize += 256;
      }
    } else {
      if (programTargetStart != nullptr) {
        WriteBlockInternal(programTargetStart, programSourceStart, programSize);
        programTargetStart = nullptr;
      }
    }
  }

  if (programTargetStart != nullptr) {
    WriteBlockInternal(programTargetStart, programSourceStart, programSize);
  }
}

//---------------------------------------------------------------------------

[[gnu::weak]] bool Flash::IsScriptMemory(const void *start, const void *end) {
  return false;
}

//---------------------------------------------------------------------------

void Flash::BeginWriteBinding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing address\n\n");
    return;
  }

  if (instance.isLocked) {
    Console::Printf("ERR Write access is disabled\n\n");
    ButtonScriptManager::ExecuteScript(
        ButtonScriptId::FLASH_WRITE_ACCESS_REQUESTED);
    return;
  }

  if (IsUpdating()) {
    Console::Printf("ERR Write in progress\n\n");
  }

  size_t addressValue = 0;
  while (*p) {
    const int hexValue = Unicode::GetHexValue(*p++);
    if (hexValue == -1) {
      continue;
    }
    addressValue = 16 * addressValue + hexValue;
  }

  ExternalFlash::Begin();
  instance.BeginWrite((const uint8_t *)addressValue);
  ExternalFlash::End();

  Console::SendOk();
}

void Flash::WriteBinding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR Missing data\n\n");
    return;
  }

  if (!IsUpdating()) {
    Console::Printf("ERR No write in progress\n\n");
    return;
  }

  uint8_t decodeBuffer[256];
  const size_t byteCount = Base64::Decode(decodeBuffer, (const uint8_t *)p);

  if (byteCount == 0) {
    Console::Printf("ERR No data\n\n");
    return;
  }

  instance.AddData(decodeBuffer, byteCount);

  Console::SendOk();
}

void Flash::EndWriteBinding(void *context, const char *commandLine) {
  if (!IsUpdating()) {
    Console::Printf("ERR No write in progress\n\n");
    return;
  }

  const bool isScriptMemory =
      instance.IsScriptMemory(instance.writeStart, instance.target);

  instance.WriteRemaining();
  Console::SendOk();

  if (isScriptMemory) {
    ButtonScriptManager::GetInstance().Reset();
  }
}

void Flash::AddConsoleCommands(Console &console) {
  console.RegisterCommand("begin_write",
                          "Begin a flash write to the specified address",
                          &Flash::BeginWriteBinding, nullptr);
  console.RegisterCommand("write",
                          "Writes base64 data to the address specified in "
                          "begin_write",
                          &Flash::WriteBinding, nullptr);
  console.RegisterCommand("end_write", "Completes writing to flash",
                          &Flash::EndWriteBinding, nullptr);
}

//---------------------------------------------------------------------------

void Flash::PrintInfo() {
  Console::Printf("Flash session statistics\n");
  Console::Printf("  Erased bytes: %zu\n", instance.erasedBytes);
  Console::Printf("  Programmed bytes: %zu\n", instance.programmedBytes);
  Console::Printf("  Reprogrammed bytes: %zu\n", instance.reprogrammedBytes);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"

//---------------------------------------------------------------------------

[[gnu::aligned(4096)]] static char flashWriteTestData[8 * 1024];

static void RandomizeFlashWriteTestData() {
  for (size_t i = 0; i < sizeof(flashWriteTestData); ++i) {
    flashWriteTestData[i] = rand();
  }
}

TEST_BEGIN("Flash: Write with FlashWriteMode::PRESERVE") {
  char writeData[32];
  for (size_t i = 0; i < 32; ++i) {
    writeData[i] = i;
  }

  const size_t testOffsets[] = {
      0, 16, 4096 - 32, 4096 - 16, 4096, 4096 + 16, 8192 - 32,
  };

  for (size_t testOffset : testOffsets) {
    RandomizeFlashWriteTestData();
    char expectedResult[8192];
    Mem::Copy(expectedResult, flashWriteTestData, 8192);
    Mem::Copy(expectedResult + testOffset, writeData, 32);
    Flash::Write(flashWriteTestData + testOffset, writeData, 32,
                 FlashWriteMode::PRESERVE);
    assert(Mem::Eq(flashWriteTestData, expectedResult, 8192));
  }
}
TEST_END

TEST_BEGIN("Flash: Write with FlashWriteMode::PRESERVE_BEFORE") {
  char writeData[32];
  for (size_t i = 0; i < 32; ++i) {
    writeData[i] = i;
  }

  const size_t testOffsets[] = {
      0, 16, 4096 - 32, 4096 - 16, 4096, 4096 + 16, 8192 - 32,
  };

  for (size_t testOffset : testOffsets) {
    // Test no reset case.
    RandomizeFlashWriteTestData();
    Mem::Fill(flashWriteTestData + testOffset, 32);

    char expectedResult[8192];
    Mem::Copy(expectedResult, flashWriteTestData, 8192);
    Mem::Copy(expectedResult + testOffset, writeData, 32);
    Flash::Write(flashWriteTestData + testOffset, writeData, 32,
                 FlashWriteMode::PRESERVE_BEFORE);
    assert(Mem::Eq(flashWriteTestData, expectedResult, 8192));

    // Test reset case.
    RandomizeFlashWriteTestData();
    Mem::Clear(flashWriteTestData + testOffset, 32);

    Mem::Copy(expectedResult, flashWriteTestData, 8192);
    Mem::Copy(expectedResult + testOffset, writeData, 32);
    Mem::Fill(expectedResult + testOffset + 32,
              (8192 - 32 - testOffset) & 0xfff);
    Flash::Write(flashWriteTestData + testOffset, writeData, 32,
                 FlashWriteMode::PRESERVE_BEFORE);
    assert(Mem::Eq(flashWriteTestData, expectedResult, 8192));
  }
}
TEST_END

TEST_BEGIN("Flash: Write with FlashWriteMode::PRESERVE_AFTER") {
  char writeData[32];
  for (size_t i = 0; i < 32; ++i) {
    writeData[i] = i;
  }

  const size_t testOffsets[] = {
      0, 16, 4096 - 32, 4096 - 16, 4096, 4096 + 16, 8192 - 32,
  };

  for (size_t testOffset : testOffsets) {
    // Test no reset case.
    RandomizeFlashWriteTestData();
    Mem::Fill(flashWriteTestData + testOffset, 32);

    char expectedResult[8192];
    Mem::Copy(expectedResult, flashWriteTestData, 8192);
    Mem::Copy(expectedResult + testOffset, writeData, 32);
    Flash::Write(flashWriteTestData + testOffset, writeData, 32,
                 FlashWriteMode::PRESERVE_AFTER);
    assert(Mem::Eq(flashWriteTestData, expectedResult, 8192));

    // Test reset case.
    RandomizeFlashWriteTestData();
    Mem::Clear(flashWriteTestData + testOffset, 32);

    Mem::Copy(expectedResult, flashWriteTestData, 8192);
    Mem::Copy(expectedResult + testOffset, writeData, 32);
    Mem::Fill(expectedResult + (testOffset & ~0xfff), testOffset & 0xfff);
    Flash::Write(flashWriteTestData + testOffset, writeData, 32,
                 FlashWriteMode::PRESERVE_AFTER);
    assert(Mem::Eq(flashWriteTestData, expectedResult, 8192));
  }
}
TEST_END

TEST_BEGIN("Flash: Write with FlashWriteMode::RESET") {
  char writeData[32];
  for (size_t i = 0; i < 32; ++i) {
    writeData[i] = i;
  }

  const size_t testOffsets[] = {
      0, 16, 4096 - 32, 4096 - 16, 4096, 4096 + 16, 8192 - 32,
  };

  for (size_t testOffset : testOffsets) {
    // Test no reset case.
    RandomizeFlashWriteTestData();
    Mem::Fill(flashWriteTestData + testOffset, 32);

    char expectedResult[8192];
    Mem::Copy(expectedResult, flashWriteTestData, 8192);
    Mem::Copy(expectedResult + testOffset, writeData, 32);
    Flash::Write(flashWriteTestData + testOffset, writeData, 32,
                 FlashWriteMode::RESET);
    assert(Mem::Eq(flashWriteTestData, expectedResult, 8192));

    // Test reset case.
    RandomizeFlashWriteTestData();
    Mem::Clear(flashWriteTestData + testOffset, 32);

    Mem::Copy(expectedResult, flashWriteTestData, 8192);
    Mem::Fill(expectedResult + (testOffset & ~0xfff), 4096);
    Mem::Fill(expectedResult + ((testOffset + 32 - 1) & ~0xfff), 4096);
    Mem::Copy(expectedResult + testOffset, writeData, 32);
    Flash::Write(flashWriteTestData + testOffset, writeData, 32,
                 FlashWriteMode::RESET);
    assert(Mem::Eq(flashWriteTestData, expectedResult, 8192));
  }
}
TEST_END

//---------------------------------------------------------------------------
