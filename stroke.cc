//---------------------------------------------------------------------------

#include "stroke.h"
#include "crc.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

bool StenoStroke::ProcessCharacter(int c, const char *symbols,
                                   const uint32_t *masks) {
  const char *p = strchr(symbols, c);
  if (!p) {
    return false;
  }

  keyState |= masks[p - symbols];
  return true;
}

void StenoStroke::Set(const char *string) {
  // cspell: disable
  static const char LEFT_SYMBOLS[] = "#1234506789STKPWHRAO*-EU";
  static const char RIGHT_SYMBOLS[] = "6789FRPBLGTSDZ";
  // cspell: enable

  static const uint32_t LEFT_MASKS[] = {
      StrokeMask::NUM,
      StrokeMask::NUM | StrokeMask::SL,
      StrokeMask::NUM | StrokeMask::TL,
      StrokeMask::NUM | StrokeMask::PL,
      StrokeMask::NUM | StrokeMask::HL,
      StrokeMask::NUM | StrokeMask::A,
      StrokeMask::NUM | StrokeMask::O,
      StrokeMask::NUM | StrokeMask::FR,
      StrokeMask::NUM | StrokeMask::PR,
      StrokeMask::NUM | StrokeMask::LR,
      StrokeMask::NUM | StrokeMask::TR,
      StrokeMask::SL,
      StrokeMask::TL,
      StrokeMask::KL,
      StrokeMask::PL,
      StrokeMask::WL,
      StrokeMask::HL,
      StrokeMask::RL,
      StrokeMask::A,
      StrokeMask::O,
      StrokeMask::STAR,
      0,
      StrokeMask::E,
      StrokeMask::U,
  };

  static const uint32_t RIGHT_MASKS[] = {
      StrokeMask::NUM | StrokeMask::FR,
      StrokeMask::NUM | StrokeMask::PR,
      StrokeMask::NUM | StrokeMask::LR,
      StrokeMask::NUM | StrokeMask::TR,
      StrokeMask::FR,
      StrokeMask::RR,
      StrokeMask::PR,
      StrokeMask::BR,
      StrokeMask::LR,
      StrokeMask::GR,
      StrokeMask::TR,
      StrokeMask::SR,
      StrokeMask::DR,
      StrokeMask::ZR,
  };

  keyState = 0;
  const char *rightStart = RightStart(string);
  const char *p = string;

  while (p < rightStart) {
    if (!ProcessCharacter(*p++, LEFT_SYMBOLS, LEFT_MASKS)) {
      keyState = 0;
      return;
    }
  }

  while (*p) {
    if (!ProcessCharacter(*p++, RIGHT_SYMBOLS, RIGHT_MASKS)) {
      keyState = 0;
      return;
    }
  }
}

const char *StenoStroke::RightStart(const char *p) {
  // cspell: disable
  static const char MIDDLE_SYMBOLS[] = "AEOU-*";
  static const char RIGHT_ONLY_SYMBOLS[] = "FBLGDZ";
  // cspell: enable

  bool foundMiddle = false;
  for (;;) {
    if (!*p) {
      return p;
    }

    if (strchr(MIDDLE_SYMBOLS, *p)) {
      foundMiddle = true;
    } else if (strchr(RIGHT_ONLY_SYMBOLS, *p)) {
      return p;
    } else if (foundMiddle) {
      return p;
    }
    ++p;
  }
}

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

//---------------------------------------------------------------------------

#include "unit_test.h"

TEST_BEGIN("Stroke tests") {
  char buffer[StenoStroke::MAX_STRING_LENGTH];

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
