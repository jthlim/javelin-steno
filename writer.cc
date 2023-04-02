//---------------------------------------------------------------------------

#include "writer.h"
#include "console.h"
#include <string.h>

//---------------------------------------------------------------------------

IWriter::ClassData IWriter::classData = {
    {nullptr, nullptr, nullptr, nullptr},
    0,
    &ConsoleWriter::instance,
};

void IWriter::Push(IWriter *writer) {
  classData.data[classData.count++] = classData.active;
  classData.active = writer;
}

void IWriter::Pop() {
  if (classData.count == 0) {
    return;
  }
  classData.active = classData.data[--classData.count];
}

//---------------------------------------------------------------------------

ConsoleWriter ConsoleWriter::instance;

#ifdef RUN_TESTS

std::vector<char> Console::history;

__attribute__((weak)) void ConsoleWriter::Write(const char *data,
                                                size_t length) {
  std::copy(data, data + length, std::back_inserter(Console::history));
}

#endif

//---------------------------------------------------------------------------

EmptyWriter EmptyWriter::instance;
void EmptyWriter::Write(const char *data, size_t length) {}

//---------------------------------------------------------------------------

BufferWriter::BufferWriter()
    : bufferUsedCount(0), bufferSize(128), buffer((char *)malloc(128)) {}

void BufferWriter::Write(const char *data, size_t length) {
  size_t newUsedCount = bufferUsedCount + length;
  if (newUsedCount > bufferSize) {
    do {
      bufferSize *= 2;
    } while (newUsedCount > bufferSize);

    char *newBuffer = (char *)malloc(bufferSize);
    memcpy(newBuffer, buffer, bufferUsedCount);
    free(buffer);
    buffer = newBuffer;
  }
  memcpy(buffer + bufferUsedCount, data, length);
  bufferUsedCount += length;
}

//---------------------------------------------------------------------------

void LimitedBufferWriter::Write(const char *data, size_t length) {
  if (bufferUsedCount + length > BUFFER_SIZE) {
    length = BUFFER_SIZE - bufferUsedCount;
  }
  memcpy(buffer + bufferUsedCount, data, length);
  bufferUsedCount += length;
}

void LimitedBufferWriter::AddTrailingNull() {
  while (bufferUsedCount > 0 && buffer[bufferUsedCount - 1] == '\n') {
    --bufferUsedCount;
  }
  buffer[bufferUsedCount] = '\0';
}

//---------------------------------------------------------------------------
