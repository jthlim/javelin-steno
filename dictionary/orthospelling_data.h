//---------------------------------------------------------------------------

#pragma once
#include "../sized_list.h"
#include "../stroke.h"

//---------------------------------------------------------------------------

struct OrthospellingContext {
  OrthospellingContext(char (&buffer)[64]) {
    buffers[0] = &buffer[0];
    buffers[1] = &buffer[16];
    buffers[2] = &buffer[32];
    buffers[3] = &buffer[48];
    buffer[0] = '\0';
    buffer[16] = '\0';
    buffer[32] = '\0';
    buffer[48] = '\0';
  }

  char *buffers[4];
};

struct OrthospellingData {
  struct Starter {
    StenoStroke stroke;
    StenoStroke mask;
    const char *definition;
  };
  struct Letter {
    StenoStroke stroke;
    uint8_t order; // 0-2 inclusive, representing the buffer used in
    const char data[7];
  };
  struct Exit {
    StenoStroke stroke;
    union {
      struct {
        uint32_t mask : 30;
        bool polarity : 1;
        bool unicodeMask : 1;
      };
      uint32_t fullMask;
    };
  };

  const char *name;
  SizedList<Starter> starters;
  SizedList<Letter> letters;
  SizedList<Exit> exits;

  // Returns nullptr if the stroke is not a starting stroke.
  const Starter *GetStarterDefinition(StenoStroke stroke) const;
  const bool IsExit(StenoStroke stroke) const;

  bool ConvertToText(StenoStroke stroke, OrthospellingContext context,
                     size_t startingIndex = 0) const;
};

//---------------------------------------------------------------------------
