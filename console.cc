//---------------------------------------------------------------------------

#include "console.h"
#include "bit_field.h"
#include "button_script_manager.h"
#include "str.h"
#include "unicode.h"
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

//---------------------------------------------------------------------------

ConsoleWriter::ClassData ConsoleWriter::classData = {
    {nullptr, nullptr, nullptr, nullptr},
    0,
    &ConsoleWriter::instance,
};

//---------------------------------------------------------------------------

ConsoleWriter ConsoleWriter::instance;

#ifdef RUN_TESTS

std::vector<char> Console::history;

[[gnu::weak]] void ConsoleWriter::Write(const char *data, size_t length) {
  std::copy(data, data + length, std::back_inserter(Console::history));
}

#endif

void ConsoleWriter::Push(IWriter *writer) {
  classData.data[classData.count++] = classData.active;
  classData.active = writer;
}

void ConsoleWriter::Pop() {
  if (classData.count == 0) {
    return;
  }
  classData.active = classData.data[--classData.count];
}

[[gnu::weak]] void ConsoleWriter::SetConnection(ConnectionId connectionId,
                                                uint16_t connectionHandle) {}

//---------------------------------------------------------------------------

int Console::ChannelHistory::AllocateId() {
  BitField<100> used = {};
  for (const uint8_t h : history) {
    used.Set(h);
  }

  for (int i = 1; i < 100; ++i) {
    if (!used.IsSet(i)) {
      AddHistoryEntry(i);
      return i;
    }
  }
  return 0;
}

void Console::ChannelHistory::Touch(int channelId) {
  if (channelId <= 0 || channelId > 99) {
    return;
  }

  if (history.Back() == channelId) {
    return;
  }
  history.Remove(channelId);
  AddHistoryEntry(channelId);
}

void Console::ChannelHistory::AddHistoryEntry(int channelId) {
  if (history.IsFull()) {
    history.PopFront();
  }
  history.Add(channelId);
}

//---------------------------------------------------------------------------

Console Console::instance;

Console::Console() {
  freeBuffers.Add(&channels[0]);
  freeBuffers.Add(&channels[1]);
  freeBuffers.Add(&channels[2]);
  freeBuffers.Add(&channels[3]);
}

static constexpr ConsoleCommand HELP_COMMAND = {
    .command = "help",
    .description = "Lists available commands",
    .handler = &Console::HelpCommand,
    .context = nullptr,
};

static constexpr ConsoleCommand HELLO_COMMAND = {
    .command = "hello",
    .description = "Initiates a stable communication channel",
    .handler = &Console::HelloCommand,
    .context = nullptr,
};

static constexpr ConsoleCommand ENABLE_EVENTS_COMMAND = {
    .command = "enable_events",
    .description = "Enables specified console events",
    .handler = &Console::EnableEvents,
    .context = nullptr,
};

static constexpr ConsoleCommand DISABLE_EVENTS_COMMAND = {
    .command = "disable_events",
    .description = "Disables specified console events",
    .handler = &Console::DisableEvents,
    .context = nullptr,
};

static constexpr size_t MAX_COMMAND_COUNT = 64;
static size_t commandCount = 4;
static ConsoleCommand commands[MAX_COMMAND_COUNT] = {
    HELLO_COMMAND,
    HELP_COMMAND,

    ENABLE_EVENTS_COMMAND,
    DISABLE_EVENTS_COMMAND,
};

static constexpr const char *EVENT_NAMES[] = {
    "button_state",      //
    "dictionary_status", //
    "paper_tape",        //
    "script",            //
#if JAVELIN_BLE          //
    "serial",            //
#endif                   //
    "suggestion",        //
    "template_value",    //
    "text",              //
    "analog_data",       //
};

static_assert(sizeof(EVENT_NAMES) / sizeof(*EVENT_NAMES) ==
              (size_t)ConsoleEvent::COUNT);

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

void Console::PrintfInternal(const char *format, ...) {
  va_list args;
  va_start(args, format);

  ConsoleWriter::VprintfToActive(format, args);
  va_end(args);
}

void Console::WriteAsJson(const char *data, char *buffer) {
  char *p = Str::WriteJson(buffer, data);
  Write(buffer, p - buffer);
}

void Console::WriteAsJson(const char *data) {
  const size_t length = Str::Length(data);
  char *buffer = (char *)malloc(2 * length);
  WriteAsJson(data, buffer);
  free(buffer);
}

[[gnu::weak]] void Console::Flush() {}

//---------------------------------------------------------------------------

Console::Channel *Console::GetChannel(int channelId) {
  if (channelId >= 0) {
    for (Channel *c : activeBuffers) {
      if (c->id == channelId) {
        return c;
      }
    }
  }
  // No channel ID? Return the last active buffer if there is one with -1 id.
  if (channelId < 0) {
    for (Channel *channel : activeBuffers) {
      if (channel->id == channelId) {
        return channel;
      }
    }
  }

  Channel *result;
  if (freeBuffers.IsEmpty()) {
    // No free buffers?
    // This could happen if multiple messages were aborted mid-message for
    // whatever reason. Best is to just return the oldest in the activeBuffers
    // and move it to the top.
    result = activeBuffers.Front();
    activeBuffers.PopFront();
  } else {
    result = freeBuffers.PopBack();
  }
  activeBuffers.Add(result);
  result->Reset(channelId);
  return result;
}

int Console::AllocateChannelId() { return channelHistory.AllocateId(); }

void Console::HandleInput(const char *data, size_t length) {
  int channelId = -1;
  const char *end = data + length;

  // If the input starts with 'c##<space>', then treat it as channel input.
  if (length > 4 && data[0] == 'c' && Unicode::IsAsciiDigit(data[1]) &&
      Unicode::IsAsciiDigit(data[2]) && data[3] == ' ') {
    channelId = 10 * (data[1] - '0') + data[2] - '0';
    channelHistory.Touch(channelId);
    data += 4;
  }

  Channel *channel = GetChannel(channelId);
  for (const char *p = data; p < end; ++p) {
    const uint8_t c = *p;
    if (c == '\0') {
      continue;
    }

    if (c == '\n') {
      channel->AddByte('\0');
      ProcessChannelCommand(*channel);
      channel->Reset(channelId);
      continue;
    }

    channel->AddByte(c);
  }
  if (channel->bufferCount == 0) {
    activeBuffers.Remove(channel);
    freeBuffers.Add(channel);
  }
}

void Console::ProcessChannelCommand(Channel &channel) {
  if (channel.bufferCount == 0) {
    return;
  }

  if (channel.id >= 0) {
    Printf("c%02d ", channel.id);
  }

  if (isLocked) {
    Printf("ERR Console access is disabled\n\n");
    ButtonScriptManager::ExecuteScript(
        ButtonScriptId::CONSOLE_ACCESS_REQUESTED);
    return;
  }

  if (channel.isTooLong) {
    Printf("ERR Command too long.\n\n");
    return;
  }

  if (channel.buffer[0] == '\0') {
    SendOk();
    return;
  }

  const ConsoleCommand *command = GetCommand(channel.buffer);
  if (command) {
    (*command->handler)(command->context, channel.buffer);
  } else {
    Printf("ERR Invalid command. Use \"help\" for a list of commands\n\n");
  }
}

const ConsoleCommand *Console::GetCommand(const char *buffer) {
  for (size_t i = 0; i < commandCount; ++i) {
    if (Str::HasPrefix(buffer, commands[i].command) &&
        Unicode::IsWhitespace(buffer[Str::Length(commands[i].command)])) {
      return &commands[i];
    }
  }
  return nullptr;
}

bool Console::RunCommand(const char *command, IWriter &writer) {
  const ConsoleCommand *consoleCommand = GetCommand(command);
  if (!consoleCommand) {
    return false;
  }

  ConsoleWriter::Push(&writer);
  (*consoleCommand->handler)(consoleCommand->context, command);
  ConsoleWriter::Pop();

  return true;
}

void Console::SendOk() { Write("OK\n\n", 4); }

// Used to flush the output buffer and ensure a stable connection.
void Console::HelloCommand(void *context, const char *line) {
  const char *suffix = strchr(line, ' ');
  Printf("c%02d ID Hello%s\n\n", instance.AllocateChannelId(),
         suffix == nullptr ? "" : suffix);
}

void Console::HelpCommand(void *context, const char *line) {
  for (size_t i = 0; i < commandCount; ++i) {
    Printf("%s: %s\n", commands[i].command, commands[i].description);
  }
  Write("\n", 1);
}

void Console::EnableEvents(void *context, const char *line) {
  UpdateEvents(line, true);
}

void Console::DisableEvents(void *context, const char *line) {
  UpdateEvents(line, false);
}

void Console::UpdateEvents(const char *line, bool value) {
  const char *p = strchr(line, ' ');
  p = Str::AdvanceToWordCharacter(p);
  if (!p) {
    Console::Printf("ERR No events specified\n\n");
    return;
  }

  int count = 0;
  while (p) {
    const char *end = Str::AdvanceToNonWordCharacter(p);
    char *eventName = Str::DupN(p, end - p);

    bool found = false;
    for (size_t i = 0; i < (size_t)ConsoleEvent::COUNT; ++i) {
      if (Str::Eq(eventName, EVENT_NAMES[i])) {
        instance.isEventEnabled[i] = value;
        found = true;
        ++count;
        break;
      }
    }
    if (!found) {
      Console::Printf("WARN Unrecognized event %s\n", eventName);
    }

    free(eventName);
    p = Str::AdvanceToWordCharacter(end);
  }

  if (count == 0) {
    Console::Printf("ERR No events recognized\n\n");
    return;
  }

  Console::SendOk();
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

TEST_BEGIN("Console should handle enable & disable events") {
  Console console;
  char buffer[64];

  for (size_t i = 0; i < (size_t)ConsoleEvent::COUNT; ++i) {
    Console::DisableEvent((ConsoleEvent)i);
  }

  for (size_t i = 0; i < (size_t)ConsoleEvent::COUNT; ++i) {
    assert(!Console::IsEventEnabled((ConsoleEvent)i));

    console.HandleInput(
        buffer, Str::Sprintf(buffer, "enable_events %s\n", EVENT_NAMES[i]));
    assert(Console::IsEventEnabled((ConsoleEvent)i));

    console.HandleInput(
        buffer, Str::Sprintf(buffer, "disable_events %s\n", EVENT_NAMES[i]));
    assert(!Console::IsEventEnabled((ConsoleEvent)i));
  }

  Console::EnableEvents(nullptr,
                        "enable_events paper_tape, suggestion, button_state");
  assert(Console::IsEventEnabled(ConsoleEvent::PAPER_TAPE));
  assert(Console::IsEventEnabled(ConsoleEvent::SUGGESTION));
  assert(Console::IsEventEnabled(ConsoleEvent::BUTTON_STATE));

  Console::DisableEvents(nullptr, "disable_events paper_tape, button_state");
  assert(!Console::IsEventEnabled(ConsoleEvent::PAPER_TAPE));
  assert(Console::IsEventEnabled(ConsoleEvent::SUGGESTION));
  assert(!Console::IsEventEnabled(ConsoleEvent::BUTTON_STATE));

  for (size_t i = 0; i < (size_t)ConsoleEvent::COUNT; ++i) {
    Console::DisableEvent((ConsoleEvent)i);
  }
}
TEST_END

#endif

//---------------------------------------------------------------------------
