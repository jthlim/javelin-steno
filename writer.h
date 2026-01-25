//---------------------------------------------------------------------------

#pragma once
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

class StenoStroke;

//---------------------------------------------------------------------------

class IWriter {
public:
  virtual void WriteByte(char c) { Write(&c, 1); }
  virtual void Write(const char *data, size_t length) = 0;

  void Dump(const void *data, size_t length);

  void WriteString(const char *s);
  void WriteBase64(const void *data, size_t length);
  void WriteIntList(const int32_t *data, size_t count);

  void WriteVarInt(int32_t value) { WriteVarUint(ZigZagEncode(value)); }
  void WriteVarUint(uint32_t value);

  void Printf(const char *p, ...);
  void Vprintf(const char *p, va_list args);

  void SetPrintfPointerOffset(const void *p) {
    printfPointerOffset = intptr_t(p);
  }

  static bool IsNumber(const char *p);
  static bool IsYamlSafe(const char *p);

private:
  intptr_t printfPointerOffset = 0;

  void WriteSegment(int flags, const char *start, const char *end, int width);

  static uint32_t ZigZagEncode(int32_t value) {
    return (value << 1) ^ (value >> 31);
  }

  void AddStroke(StenoStroke stroke);
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
  BufferWriter(char *scratch, size_t scratchLength)
      : bufferUsedCount(0), bufferSize(scratchLength), buffer(scratch),
        inlineBuffer(scratch) {}

  ~BufferWriter() {
    if (buffer != inlineBuffer) {
      free(buffer);
    }
  }

  void Write(const char *data, size_t length) final;
  void WriteByte(char c) final;
  void RemoveByte() { --bufferUsedCount; }

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
  bool IsEmpty() const { return bufferUsedCount == 0; }
  bool IsNotEmpty() const { return bufferUsedCount != 0; }

private:
  size_t bufferUsedCount;
  size_t bufferSize;
  char *buffer;
  char *inlineBuffer;

  void Reallocate();
};

class BlockWriterBase : public IWriter {
public:
  virtual void WriteByte(char c);
  virtual void Write(const char *data, size_t length);

  void Flush();

protected:
  BlockWriterBase(size_t size) : size(size) {}

  virtual void Flush(const char *data, size_t length) = 0;

private:
  size_t used = 0;
  size_t size;
  char buffer[4];
};

template <size_t N> class BlockWriter : public BlockWriterBase {
public:
  BlockWriter() : BlockWriterBase(N) {}

private:
  char buffer[N >= 4 ? N - 4 : 0];
};

class Base64Writer final : public BlockWriter<3> {
private:
  using super = BlockWriter<3>;

public:
  Base64Writer(IWriter *next) : next(next) {}

  using super::Flush;

private:
  IWriter *next;

  virtual void Flush(const char *data, size_t length);
};

class LimitedBufferWriter final : public IWriter {
public:
  void Reset() { bufferUsedCount = 0; }

  void Write(const char *data, size_t length) final;

  void AddTrailingNull();

  static constexpr size_t BUFFER_SIZE = 64;

  size_t bufferUsedCount = 0;
  uint8_t buffer[BUFFER_SIZE + 1];
};

//---------------------------------------------------------------------------
