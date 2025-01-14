//---------------------------------------------------------------------------

#pragma once
#include "hal/connection.h"
#include "static_list.h"
#include "writer.h"
#include <stddef.h>

#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

struct ConsoleCommand {
  const char *command;
  const char *description;
  void (*handler)(void *context, const char *line);
  void *context;
};

//---------------------------------------------------------------------------

class ConsoleWriter final : public IWriter {
public:
  virtual void Write(const char *data, size_t length);

  static void WriteToActive(const char *data, size_t length) {
    classData.active->Write(data, length);
  }

  static void VprintfToActive(const char *p, va_list args) {
    classData.active->Vprintf(p, args);
  }

  static IWriter *GetActiveWriter() { return classData.active; }

  static void Push(IWriter *writer);
  static void Pop();

  static void SetConnection(ConnectionId connectionId,
                            uint16_t connectionHandle);

  static ConsoleWriter instance;

private:
  struct ClassData {
    IWriter *data[4];
    size_t count;
    IWriter *active;
  };
  static ClassData classData;
};

//---------------------------------------------------------------------------

class Console {
public:
  Console();

  int AllocateChannelId();

  void HandleInput(const char *data, size_t length);
  static void SendOk();

  // Returns true if successful.
  static bool RunCommand(const char *command, IWriter &writer);

  static void RegisterCommand(const ConsoleCommand &command);
  static void RegisterCommand(const char *command, const char *description,
                              void (*handler)(void *context, const char *line),
                              void *context);

  static void HelloCommand(void *context, const char *line);
  static void HelpCommand(void *context, const char *line);

  static void Write(const char *data, size_t length) {
    ConsoleWriter::WriteToActive(data, length);
  }
  static void WriteAsJson(const char *data, char *buffer);
  static void WriteAsJson(const char *data);

  static void WriteScriptEvent(const char *text);

  template <typename T, typename... T2>
  static void Printf(const char *format, T arg, T2... args) {
    PrintfInternal(format, arg, args...);
  }

  template <size_t N> static void Printf(const char (&text)[N]) {
    Write(text, N - 1);
  }

  static void Dump(const void *data, size_t length) {
    ConsoleWriter::instance.GetActiveWriter()->Dump(data, length);
  }
  static void Flush();

#if RUN_TESTS
  static std::vector<char> history;
#endif

  static Console instance;

private:
  struct Channel {
    int8_t id;
    bool isTooLong;
    uint16_t bufferCount;
    char buffer[256];

    void Reset(uint32_t channelId) {
      id = channelId;
      isTooLong = false;
      bufferCount = 0;
    }

    void AddByte(uint8_t c) {
      if (bufferCount >= sizeof(buffer)) {
        isTooLong = true;
        return;
      }
      buffer[bufferCount++] = c;
    }
  };

  class ChannelHistory {
  public:
    int AllocateId();
    void Touch(int channelId);

  private:
    void AddHistoryEntry(int channelId);

    StaticList<uint8_t, 16> history;
  };

  static constexpr size_t CHANNEL_COUNT = 4;
  StaticList<Channel *, CHANNEL_COUNT> freeBuffers;
  StaticList<Channel *, CHANNEL_COUNT> activeBuffers;

  ChannelHistory channelHistory;

  Channel channels[CHANNEL_COUNT];

  Channel *GetChannel(int channelId);
  void ProcessChannelCommand(Channel &channel);
  static void PrintfInternal(const char *format, ...);

  static const ConsoleCommand *GetCommand(const char *buffer);
};

//---------------------------------------------------------------------------
