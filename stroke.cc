//---------------------------------------------------------------------------

#include "stroke.h"
#include "crc32.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

// Only for use in tests.
#ifdef RUN_TESTS
void StenoStroke::Set(const char *string) {
  keyState = 0;
  const char *rightStart = RightStart(string);
  const char *p = string;

  while (p < rightStart) {
    switch (*p) {
    case '#':
      keyState |= StrokeMask::NUM;
      break;

    case '1':
      keyState |= StrokeMask::NUM | StrokeMask::SL;
      break;

    case '2':
      keyState |= StrokeMask::NUM | StrokeMask::TL;
      break;

    case '3':
      keyState |= StrokeMask::NUM | StrokeMask::PL;
      break;

    case '4':
      keyState |= StrokeMask::NUM | StrokeMask::HL;
      break;

    case '5':
      keyState |= StrokeMask::NUM | StrokeMask::A;
      break;

    case '0':
      keyState |= StrokeMask::NUM | StrokeMask::O;
      break;

    case '6':
      keyState |= StrokeMask::NUM | StrokeMask::FR;
      break;

    case '7':
      keyState |= StrokeMask::NUM | StrokeMask::PR;
      break;

    case '8':
      keyState |= StrokeMask::NUM | StrokeMask::LR;
      break;

    case '9':
      keyState |= StrokeMask::NUM | StrokeMask::TR;
      break;

    case 'S':
      keyState |= StrokeMask::SL;
      break;

    case 'T':
      keyState |= StrokeMask::TL;
      break;

    case 'K':
      keyState |= StrokeMask::KL;
      break;

    case 'P':
      keyState |= StrokeMask::PL;
      break;

    case 'W':
      keyState |= StrokeMask::WL;
      break;

    case 'H':
      keyState |= StrokeMask::HL;
      break;

    case 'R':
      keyState |= StrokeMask::RL;
      break;

    case 'A':
      keyState |= StrokeMask::A;
      break;

    case 'O':
      keyState |= StrokeMask::O;
      break;

    case '*':
      keyState |= StrokeMask::STAR;
      break;

    case '-':
      break;

    case 'E':
      keyState |= StrokeMask::E;
      break;

    case 'U':
      keyState |= StrokeMask::U;
      break;
    }
    p++;
  }

  while (*p) {
    switch (*p) {
    case '6':
      keyState |= StrokeMask::NUM | StrokeMask::FR;
      break;

    case '7':
      keyState |= StrokeMask::NUM | StrokeMask::PR;
      break;

    case '8':
      keyState |= StrokeMask::NUM | StrokeMask::LR;
      break;

    case '9':
      keyState |= StrokeMask::NUM | StrokeMask::TR;
      break;

    case 'F':
      keyState |= StrokeMask::FR;
      break;

    case 'R':
      keyState |= StrokeMask::RR;
      break;

    case 'P':
      keyState |= StrokeMask::PR;
      break;

    case 'B':
      keyState |= StrokeMask::BR;
      break;

    case 'L':
      keyState |= StrokeMask::LR;
      break;

    case 'G':
      keyState |= StrokeMask::GR;
      break;

    case 'T':
      keyState |= StrokeMask::TR;
      break;

    case 'S':
      keyState |= StrokeMask::SR;
      break;

    case 'D':
      keyState |= StrokeMask::DR;
      break;

    case 'Z':
      keyState |= StrokeMask::ZR;
      break;
    }
    p++;
  }
}

const char *StenoStroke::RightStart(const char *p) {
  bool foundMiddle = false;
  for (;;) {
    switch (*p) {
    case '\0':
      return p;

    case 'A':
    case 'E':
    case 'O':
    case 'U':
    case '-':
    case '*':
      foundMiddle = true;
      break;

    case 'F':
    case 'B':
    case 'L':
    case 'G':
    case 'D':
    case 'Z':
      return p;

    default:
      if (foundMiddle) {
        return p;
      }
      break;
    }
    ++p;
  }
}
#endif

char *StenoStroke::ToString(char *buffer) const {
  // cspell: disable-next-line
  const char *keys = "#STKPWHRAO*EUFRPBLGTSDZ";

  for (int i = 0; i < StrokeBitIndex::STAR; ++i) {
    if (keyState & (1UL << i)) {
      *buffer++ = keys[i];
    }
  }

  const uint32_t remainderMask =
      StrokeMask::A | StrokeMask::O | StrokeMask::STAR | StrokeMask::E |
      StrokeMask::U | StrokeMask::FR | StrokeMask::RR | StrokeMask::PR |
      StrokeMask::BR | StrokeMask::LR | StrokeMask::GR | StrokeMask::TR |
      StrokeMask::SR | StrokeMask::DR | StrokeMask::ZR;

  if (keyState & remainderMask) {
    const uint32_t centralKeyMask = StrokeMask::A | StrokeMask::O |
                                    StrokeMask::STAR | StrokeMask::E |
                                    StrokeMask::U;

    if ((keyState & centralKeyMask) == 0) {
      *buffer++ = '-';
    }

    for (int i = StrokeBitIndex::STAR; i < StrokeBitIndex::COUNT; ++i) {
      if (keyState & (1UL << i)) {
        *buffer++ = keys[i];
      }
    }
  }

  *buffer = '\0';
  return buffer;
}

char *StenoStroke::ToWideString(char *buffer) const {
  // cspell: disable-next-line
  const char *keys = "#STKPWHRAO*EUFRPBLGTSDZ";

  for (int i = 0; i < StrokeBitIndex::COUNT; ++i) {
    *buffer++ = (keyState & (1UL << i)) ? keys[i] : ' ';
  }
  *buffer = '\0';
  return buffer;
}

uint32_t StenoStroke::PopCount(const StenoStroke *strokes, size_t length) {
  uint32_t result = 0;
  for (size_t i = 0; i < length; ++i) {
    result += strokes[i].PopCount();
  }
  return result;
}

uint32_t StenoStroke::Hash(const StenoStroke *strokes, size_t length) {
  return Crc32(strokes, sizeof(StenoStroke) * length);
}

char *StenoStroke::ToString(const StenoStroke *strokes, size_t length,
                            char *buffer) {
  for (size_t i = 0; i < length; ++i) {
    if (i != 0) {
      *buffer++ = '/';
    }
    buffer = strokes[i].ToString(buffer);
  }
  return buffer;
}

//---------------------------------------------------------------------------

#include "unit_test.h"

TEST_BEGIN("Stroke tests") {
  char buffer[32];

  const StenoStroke hello(StrokeMask::HL | StrokeMask::LR);
  hello.ToString(buffer);
  assert(strcmp(buffer, "H-L") == 0);

  const StenoStroke cat("KAT");
  cat.ToString(buffer);
  assert(strcmp(buffer, "KAT") == 0);

  const StenoStroke star("STA*R");
  star.ToString(buffer);
  assert(strcmp(buffer, "STA*R") == 0);
}
TEST_END

//---------------------------------------------------------------------------
