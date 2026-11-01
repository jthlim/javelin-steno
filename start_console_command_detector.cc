//---------------------------------------------------------------------------

#include "start_console_command_detector.h"
#include <assert.h>

//---------------------------------------------------------------------------

size_t StartConsoleCommandDetector::IsStartCommandPresent(const uint8_t *data,
                                                          size_t length) {
  for (size_t i = 0; i < length; ++i) {
    if (ProcessByte(data[i])) {
      return i + 1;
    }
  }
  return (size_t)-1;
}

bool StartConsoleCommandDetector::ProcessByte(uint8_t c) {
  static constexpr char TARGET[] = "start_javelin_console\n";

  assert(state >= 0 && state < (int)sizeof(TARGET) - 1);
  if (c == (uint8_t)TARGET[state]) {
    ++state;
    if (state == (int)sizeof(TARGET) - 1) {
      state = 0;
      return true;
    }
    return false;
  }

  switch (c) {
  case 't':
    state = (state == 18) ? 2 : 0;
    break;
  case 's':
    state = 1;
    break;
  default:
    state = 0;
    break;
  }
  return false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "assert.h"
#include "unit_test.h"

TEST_BEGIN("StartConsoleCommandDetector: Full match") {
  StartConsoleCommandDetector state;
  assert(state.IsStartCommandPresent((const uint8_t *)"start_javelin_console\n",
                                     22) == 22);
}
TEST_END

TEST_BEGIN("StartConsoleCommandDetector: Split match") {
  StartConsoleCommandDetector state;
  assert(state.IsStartCommandPresent((const uint8_t *)"start", 5) == -1);
  assert(state.IsStartCommandPresent((const uint8_t *)"_javelin", 8) == -1);
  assert(state.IsStartCommandPresent((const uint8_t *)"_console", 8) == -1);
  assert(state.IsStartCommandPresent((const uint8_t *)"\n", 1) == 1);
}
TEST_END

TEST_BEGIN("StartConsoleCommandDetector: Split match with starting mismatch") {
  StartConsoleCommandDetector state;
  assert(state.IsStartCommandPresent((const uint8_t *)"start_javelin_con",
                                     17) == -1);
  assert(state.IsStartCommandPresent((const uint8_t *)"start", 5) == -1);
  assert(state.IsStartCommandPresent((const uint8_t *)"_javelin", 8) == -1);
  assert(state.IsStartCommandPresent((const uint8_t *)"_console", 8) == -1);
  assert(state.IsStartCommandPresent((const uint8_t *)"\n", 1) == 1);
}
TEST_END

TEST_BEGIN(
    "StartConsoleCommandDetector: Overlapping prefix and repeated matches") {
  StartConsoleCommandDetector state;
  // Overlapping prefix: "start_javelin_cons" followed immediately by
  // "tart_javelin_console\n"
  const char *overlapping = "start_javelin_constart_javelin_console\n";
  assert(state.IsStartCommandPresent((const uint8_t *)overlapping, 39) == 39);

  // Repeated 's' prefix
  state.Reset();
  const char *repeated = "ssssstart_javelin_console\n";
  assert(state.IsStartCommandPresent((const uint8_t *)repeated, 26) == 26);
}
TEST_END

//---------------------------------------------------------------------------
