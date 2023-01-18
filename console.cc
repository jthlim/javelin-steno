//---------------------------------------------------------------------------

#include "console.h"
#include "str.h"
#include "unicode.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

//---------------------------------------------------------------------------

static const ConsoleCommand HELP_COMMAND = {
    .command = "help",
    .description = "Provides a list of commands",
    .handler = &Console::HelpCommand,
    .context = nullptr,
};

static const ConsoleCommand HELLO_COMMAND = {
    .command = "hello",
    .description = "Used to initiate a stable communication channel",
    .handler = &Console::HelloCommand,
    .context = nullptr,
};

static const size_t MAX_COMMAND_COUNT = 32;
static bool enableConsoleWrite = true;
static size_t commandCount = 2;
static ConsoleCommand commands[MAX_COMMAND_COUNT] = {
    HELLO_COMMAND,
    HELP_COMMAND,
};

//---------------------------------------------------------------------------

void Console::RegisterCommand(const ConsoleCommand &command) {
  assert(commandCount < MAX_COMMAND_COUNT);
  commands[commandCount++] = command;
}

void Console::RegisterCommand(const char *command, const char *description,
                              void (*handler)(void *context, const char *line),
                              void *context) {
  assert(commandCount < MAX_COMMAND_COUNT);
  commands[commandCount].command = command;
  commands[commandCount].description = description;
  commands[commandCount].handler = handler;
  commands[commandCount].context = context;
  commandCount++;
}

//---------------------------------------------------------------------------

#ifdef RUN_TESTS

std::vector<char> Console::history;

void Console::RawWrite(const char *data, size_t length) {
  std::copy(data, data + length, std::back_inserter(history));
}

#endif

void Console::Printf(const char *format, ...) {
  if (!enableConsoleWrite) {
    return;
  }

  va_list v;
  va_start(v, format);

  size_t length = vsnprintf(nullptr, 0, format, v) + 1;
  char *buffer = (char *)malloc(length);
  vsnprintf(buffer, length, format, v);
  va_end(v);

  RawWrite(buffer, length - 1);

  free(buffer);
}

void Console::Write(const char *data, size_t length) {
  if (enableConsoleWrite) {
    RawWrite(data, length);
  }
}

void Console::WriteAsJson(const char *data, char *buffer) {
  char *p = Str::WriteJson(buffer, data);
  Console::Write(buffer, p - buffer);
}

__attribute__((weak)) void Console::Flush() {}

//---------------------------------------------------------------------------

void Console::HandleInput(const char *data, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    if (data[i] == 0) {
      continue;
    } else if (data[i] == '\n') {
      ProcessLineBuffer();
      continue;
    } else if (lineBufferCount == sizeof(lineBuffer) - 1) {
      ProcessLineBuffer();
    }
    lineBuffer[lineBufferCount++] = data[i];
  }
}

void Console::ProcessLineBuffer() {
  if (lineBufferCount == 0) {
    return;
  }

  lineBuffer[lineBufferCount] = '\0';
  lineBufferCount = 0;

  const ConsoleCommand *command = GetCommand(lineBuffer);
  if (command) {
    (*command->handler)(command->context, lineBuffer);
  } else {
    Console::Printf(
        "ERR Invalid command. Use \"help\" for a list of commands\n\n");
  }
}

const ConsoleCommand *Console::GetCommand(const char *buffer) {
  for (size_t i = 0; i < commandCount; ++i) {
    if (Str::HasPrefix(buffer, commands[i].command) &&
        Unicode::IsWhitespace(buffer[strlen(commands[i].command)])) {
      return &commands[i];
    }
  }
  return nullptr;
}

void Console::RunCommand(const char *command) {
  const ConsoleCommand *consoleCommand = GetCommand(command);
  if (!consoleCommand) {
    return;
  }

  const bool previousEnableConsoleWrite = enableConsoleWrite;
  enableConsoleWrite = false;
  (*consoleCommand->handler)(consoleCommand->context, command);
  enableConsoleWrite = previousEnableConsoleWrite;
}

// Used to flush the output buffer and ensure a stable connection.
void Console::HelloCommand(void *context, const char *line) {
  char buffer[256];
  memset(buffer, 0, 256);
  Console::Write(buffer, 256);
  Console::Write("Hello\n\n", 7);
}

void Console::HelpCommand(void *context, const char *line) {
  for (size_t i = 0; i < commandCount; ++i) {
    Console::Printf("%s: %s\n", commands[i].command, commands[i].description);
  }
  Console::Write("\n", 1);
}

//---------------------------------------------------------------------------

#if RUN_TESTS

#include "unit_test.h"

TEST_BEGIN("Console should handle invalid commands") {
  Console console;
  console.HandleInput("asdf\n", 5);

  Console::history.push_back(0);
  assert(
      Str::Eq(&Console::history.front(),
              "ERR Invalid command. Use \"help\" for a list of commands\n\n"));

  Console::history.clear();
}
TEST_END

#endif

//---------------------------------------------------------------------------
