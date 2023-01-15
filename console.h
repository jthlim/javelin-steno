//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>
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

class Console {
public:
  void HandleInput(const char *data, size_t length);
  static void RunCommand(const char *command);

  static void RegisterCommand(const ConsoleCommand &command);
  static void RegisterCommand(const char *command, const char *description,
                              void (*handler)(void *context, const char *line),
                              void *context);

  static void HelloCommand(void *context, const char *line);
  static void HelpCommand(void *context, const char *line);

  static void Write(const char *data, size_t length);
  static void WriteAsJson(const char *data, char *buffer);

  static void Printf(const char *format, ...) __printflike(1, 2);

  static void Flush();

#if RUN_TESTS
  static std::vector<char> history;
#endif

private:
  size_t lineBufferCount = 0;
  char lineBuffer[256];

  void ProcessLineBuffer();

  static const ConsoleCommand *GetCommand(const char *command);
  static void RawWrite(const char *data, size_t length);
};

//---------------------------------------------------------------------------
