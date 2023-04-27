//---------------------------------------------------------------------------

#pragma once
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

class IWriter {
public:
  virtual void WriteByte(char c) { Write(&c, 1); }
  virtual void Write(const char *data, size_t length) = 0;

  void Printf(const char *p, ...) __attribute__((format(printf, 2, 3)));
  void Vprintf(const char *p, va_list args);

private:
  void WriteSegment(int flags, char *start, char *end, int width);
};

class NullWriter final : public IWriter {
public:
  void WriteByte(char c);
  void Write(const char *data, size_t length);

  static NullWriter instance;
};

class CountWriter final : public IWriter {
public:
  void WriteByte(char c);
  void Write(const char *data, size_t length);

  size_t GetCount() const { return count; }

private:
  size_t count = 0;
};

class MemoryWriter final : public IWriter {
public:
  MemoryWriter(void *target) : target((uint8_t *)target) {}

  void WriteByte(char c);
  void Write(const char *data, size_t length);

  const void *GetTarget() const { return target; }

private:
  uint8_t *target;
};

class BufferWriter final : public IWriter {
public:
  BufferWriter();
  ~BufferWriter() { free(buffer); }

  void Write(const char *data, size_t length) final;

  void WriteBufferTo(IWriter *writer) const {
    writer->Write(buffer, bufferUsedCount);
  }

  char *AdoptBuffer() {
    char *returnValue = buffer;
    buffer = nullptr;
    bufferSize = 0;
    bufferUsedCount = 0;
    return returnValue;
  }

  const char *GetBuffer() const { return buffer; }
  size_t GetCount() const { return bufferUsedCount; }

private:
  size_t bufferUsedCount;
  size_t bufferSize;
  char *buffer;
};

class LimitedBufferWriter final : public IWriter {
public:
  void Reset() { bufferUsedCount = 0; }

  void Write(const char *data, size_t length) final;

  void AddTrailingNull();

  static const size_t BUFFER_SIZE = 64;

  size_t bufferUsedCount = 0;
  uint8_t buffer[BUFFER_SIZE + 1];
};

//---------------------------------------------------------------------------
