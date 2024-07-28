//---------------------------------------------------------------------------

#pragma once
#include "../sized_list.h"
#include "../stroke.h"

//---------------------------------------------------------------------------

class BufferWriter;

//---------------------------------------------------------------------------

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

  struct Context {
    const Letter **letters;

    typedef const Letter *(LetterBuffer[32]);

    Context(LetterBuffer &letters) : letters(letters) { *letters = nullptr; }

    void Add(const Letter *letter) {
      *letters++ = letter;
      *letters = nullptr;
    }

    bool IsEmpty() const { return *letters == nullptr; }

    void WriteToBuffer(BufferWriter &writer);
    uint32_t FindLowestOrder() const;
  };

  const char *name;
  SizedList<Starter> starters;
  SizedList<Letter> letters;
  SizedList<Exit> exits;

  // Returns nullptr if the stroke is not a starting stroke.
  const Starter *GetStarterDefinition(StenoStroke stroke) const;
  const bool IsExit(StenoStroke stroke) const;

  bool ResolveStroke(StenoStroke stroke, Context context,
                     size_t startingIndex = 0) const;
};

//---------------------------------------------------------------------------
