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

  void WriteString(const char *s);
  void WriteBase64(const void *data, size_t length);

  void Printf(const char *p, ...);
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
  void WriteByte(char c) final;

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

  char *TerminateStringAndAdoptBuffer() {
    WriteByte('\0');
    return AdoptBuffer();
  }

  const char *GetBuffer() const { return buffer; }
  size_t GetCount() const { return bufferUsedCount; }

private:
  size_t bufferUsedCount;
  size_t bufferSize;
  char *buffer;
};

class BlockWriterBase : public IWriter {
public:
  virtual void WriteByte(char c);
  virtual void Write(const char *data, size_t length);

  void Flush();

protected:
  BlockWriterBase(size_t size, IWriter *next) : size(size), next(next) {}

private:
  size_t used = 0;
  size_t size;
  IWriter *next;
  char buffer[0];
};

template <size_t N> class BlockWriter : public BlockWriterBase {
public:
  BlockWriter(IWriter *next) : BlockWriterBase(N, next) {}

private:
  char buffer[N];
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
