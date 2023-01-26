//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>
#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

class IWriter {
public:
  virtual void Write(const char *data, size_t length) = 0;

  static void WriteToStackTop(const char *data, size_t length) {
    classData.active->Write(data, length);
  }

  static void Push(IWriter *writer);
  static void Pop();

private:
  struct ClassData {
    IWriter *data[4];
    size_t count;
    IWriter *active;
  };
  static ClassData classData;
};

class ConsoleWriter final : public IWriter {
public:
  virtual void Write(const char *data, size_t length);

  static void WriteToStackTop(const char *data, size_t length) {
    classData.active->Write(data, length);
  }

  static void Push(ConsoleWriter *writer);
  static void Pop();

  static ConsoleWriter instance;

private:
  struct ClassData {
    ConsoleWriter *data[4];
    size_t count;
    ConsoleWriter *active;
  };
  static ClassData classData;
};

class EmptyWriter final : public IWriter {
public:
  void Write(const char *data, size_t length);

  static EmptyWriter instance;
};

class BufferWriter final : public IWriter {
public:
  BufferWriter();
  ~BufferWriter() { free(buffer); }

  void Write(const char *data, size_t length) final;

  void WriteBufferToStackTop() const {
    WriteToStackTop(buffer, bufferUsedCount);
  }

private:
  size_t bufferUsedCount;
  size_t bufferSize;
  char *buffer;
};

//---------------------------------------------------------------------------
