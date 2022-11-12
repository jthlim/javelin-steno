//---------------------------------------------------------------------------

#include "console.h"
#include "steno_key_code_emitter.h"
#include "str.h"
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

void Console::Write(const char *data, size_t length) {
  std::copy(data, data + length, std::back_inserter(history));
}

#endif

void Console::Printf(const char *format, ...) {
  va_list v;
  va_start(v, format);

  size_t length = vsnprintf(nullptr, 0, format, v) + 1;
  char *buffer = (char *)malloc(length);
  vsnprintf(buffer, length, format, v);
  va_end(v);

  Console::Write(buffer, length - 1);

  free(buffer);
}

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

  const ConsoleCommand *command = GetCommand();
  if (command) {
    (*command->handler)(command->context, lineBuffer);
  } else {
    Console::Printf(
        "ERR Invalid command. Use \"help\" for a list of commands\n\n");
  }
}

const ConsoleCommand *Console::GetCommand() const {
  for (size_t i = 0; i < commandCount; ++i) {
    if (Str::HasPrefix(lineBuffer, commands[i].command) &&
        IsWhitespace(lineBuffer[strlen(commands[i].command)])) {
      return &commands[i];
    }
  }
  return nullptr;
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
