//---------------------------------------------------------------------------

#include "start_console_command_detector.h"

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
  switch (state) {
  case 0:
    state = (c == 's') ? 1 : 0;
    return false;

  case 1:
    state = (c == 't') ? 2 : (c == 's') ? 1 : 0;
    return false;

  case 2:
    state = (c == 'a') ? 3 : (c == 's') ? 1 : 0;
    return false;

  case 3:
    state = (c == 'r') ? 4 : (c == 's') ? 1 : 0;
    return false;

  case 4:
    state = (c == 't') ? 5 : (c == 's') ? 1 : 0;
    return false;

  case 5:
    state = (c == '_') ? 6 : (c == 's') ? 1 : 0;
    return false;

  case 6:
    state = (c == 'j') ? 7 : (c == 's') ? 1 : 0;
    return false;

  case 7:
    state = (c == 'a') ? 8 : (c == 's') ? 1 : 0;
    return false;

  case 8:
    state = (c == 'v') ? 9 : (c == 's') ? 1 : 0;
    return false;

  case 9:
    state = (c == 'e') ? 10 : (c == 's') ? 1 : 0;
    return false;

  case 10:
    state = (c == 'l') ? 11 : (c == 's') ? 1 : 0;
    return false;

  case 11:
    state = (c == 'i') ? 12 : (c == 's') ? 1 : 0;
    return false;

  case 12:
    state = (c == 'n') ? 13 : (c == 's') ? 1 : 0;
    return false;

  case 13:
    state = (c == '_') ? 14 : (c == 's') ? 1 : 0;
    return false;

  case 14:
    state = (c == 'c') ? 15 : (c == 's') ? 1 : 0;
    return false;

  case 15:
    state = (c == 'o') ? 16 : (c == 's') ? 1 : 0;
    return false;

  case 16:
    state = (c == 'n') ? 17 : (c == 's') ? 1 : 0;
    return false;

  case 17:
    state = (c == 's') ? 18 : (c == 's') ? 1 : 0;
    return false;

  case 18:
    state = (c == 'o') ? 19 : (c == 't') ? 2 : (c == 's') ? 1 : 0;
    return false;

  case 19:
    state = (c == 'l') ? 20 : (c == 's') ? 1 : 0;
    return false;

  case 20:
    state = (c == 'e') ? 21 : (c == 's') ? 1 : 0;
    return false;

  case 21:
    if (c == '\n') {
      state = 0;
      return true;
    }
    state = (c == 's') ? 1 : 0;
    return false;

  default:
    state = 0;
    return false;
  }
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

//---------------------------------------------------------------------------
