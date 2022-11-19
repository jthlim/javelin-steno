//---------------------------------------------------------------------------

#include "chord.h"
#include "crc32.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

// Only for use in tests.
#ifdef RUN_TESTS
void StenoChord::Set(const char *string) {
  keyState = 0;
  const char *rightStart = RightStart(string);
  const char *p = string;

  while (p < rightStart) {
    switch (*p) {
    case '#':
      keyState |= ChordMask::NUM;
      break;

    case '1':
      keyState |= ChordMask::NUM | ChordMask::SL;
      break;

    case '2':
      keyState |= ChordMask::NUM | ChordMask::TL;
      break;

    case '3':
      keyState |= ChordMask::NUM | ChordMask::PL;
      break;

    case '4':
      keyState |= ChordMask::NUM | ChordMask::HL;
      break;

    case '5':
      keyState |= ChordMask::NUM | ChordMask::A;
      break;

    case '0':
      keyState |= ChordMask::NUM | ChordMask::O;
      break;

    case '6':
      keyState |= ChordMask::NUM | ChordMask::FR;
      break;

    case '7':
      keyState |= ChordMask::NUM | ChordMask::PR;
      break;

    case '8':
      keyState |= ChordMask::NUM | ChordMask::LR;
      break;

    case '9':
      keyState |= ChordMask::NUM | ChordMask::TR;
      break;

    case 'S':
      keyState |= ChordMask::SL;
      break;

    case 'T':
      keyState |= ChordMask::TL;
      break;

    case 'K':
      keyState |= ChordMask::KL;
      break;

    case 'P':
      keyState |= ChordMask::PL;
      break;

    case 'W':
      keyState |= ChordMask::WL;
      break;

    case 'H':
      keyState |= ChordMask::HL;
      break;

    case 'R':
      keyState |= ChordMask::RL;
      break;

    case 'A':
      keyState |= ChordMask::A;
      break;

    case 'O':
      keyState |= ChordMask::O;
      break;

    case '*':
      keyState |= ChordMask::STAR;
      break;

    case '-':
      break;

    case 'E':
      keyState |= ChordMask::E;
      break;

    case 'U':
      keyState |= ChordMask::U;
      break;
    }
    p++;
  }

  while (*p) {
    switch (*p) {
    case '6':
      keyState |= ChordMask::NUM | ChordMask::FR;
      break;

    case '7':
      keyState |= ChordMask::NUM | ChordMask::PR;
      break;

    case '8':
      keyState |= ChordMask::NUM | ChordMask::LR;
      break;

    case '9':
      keyState |= ChordMask::NUM | ChordMask::TR;
      break;

    case 'F':
      keyState |= ChordMask::FR;
      break;

    case 'R':
      keyState |= ChordMask::RR;
      break;

    case 'P':
      keyState |= ChordMask::PR;
      break;

    case 'B':
      keyState |= ChordMask::BR;
      break;

    case 'L':
      keyState |= ChordMask::LR;
      break;

    case 'G':
      keyState |= ChordMask::GR;
      break;

    case 'T':
      keyState |= ChordMask::TR;
      break;

    case 'S':
      keyState |= ChordMask::SR;
      break;

    case 'D':
      keyState |= ChordMask::DR;
      break;

    case 'Z':
      keyState |= ChordMask::ZR;
      break;
    }
    p++;
  }
}

const char *StenoChord::RightStart(const char *p) {
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

char *StenoChord::ToString(char *buffer) const {
  // cspell: disable-next-line
  const char *keys = "#STKPWHRAO*EUFRPBLGTSDZ";

  for (int i = 0; i < ChordBitIndex::STAR; ++i) {
    if (keyState & (1UL << i)) {
      *buffer++ = keys[i];
    }
  }

  const uint32_t remainderMask = ChordMask::A | ChordMask::O | ChordMask::STAR |
                                 ChordMask::E | ChordMask::U | ChordMask::FR |
                                 ChordMask::RR | ChordMask::PR | ChordMask::BR |
                                 ChordMask::LR | ChordMask::GR | ChordMask::TR |
                                 ChordMask::SR | ChordMask::DR | ChordMask::ZR;

  if (keyState & remainderMask) {
    const uint32_t centralKeyMask = ChordMask::A | ChordMask::O |
                                    ChordMask::STAR | ChordMask::E |
                                    ChordMask::U;

    if ((keyState & centralKeyMask) == 0) {
      *buffer++ = '-';
    }

    for (int i = ChordBitIndex::STAR; i < ChordBitIndex::COUNT; ++i) {
      if (keyState & (1UL << i)) {
        *buffer++ = keys[i];
      }
    }
  }

  *buffer = '\0';
  return buffer;
}

char *StenoChord::ToWideString(char *buffer) const {
  // cspell: disable-next-line
  const char *keys = "#STKPWHRAO*EUFRPBLGTSDZ";

  for (int i = 0; i < ChordBitIndex::COUNT; ++i) {
    *buffer++ = (keyState & (1UL << i)) ? keys[i] : ' ';
  }
  *buffer = '\0';
  return buffer;
}

uint32_t StenoChord::Hash(const StenoChord *chords, size_t length) {
  return Crc32(chords, 4 * length);
}

char *StenoChord::ToString(const StenoChord *chords, size_t length,
                           char *buffer) {
  for (size_t i = 0; i < length; ++i) {
    if (i != 0) {
      *buffer++ = '/';
    }
    buffer = chords[i].ToString(buffer);
  }
  return buffer;
}

//---------------------------------------------------------------------------

#include "unit_test.h"

TEST_BEGIN("Chord tests") {
  char buffer[32];

  const StenoChord hello(ChordMask::HL | ChordMask::LR);
  hello.ToString(buffer);
  assert(strcmp(buffer, "H-L") == 0);

  const StenoChord cat("KAT");
  cat.ToString(buffer);
  assert(strcmp(buffer, "KAT") == 0);

  const StenoChord star("STA*R");
  star.ToString(buffer);
  assert(strcmp(buffer, "STA*R") == 0);
}
TEST_END

//---------------------------------------------------------------------------
