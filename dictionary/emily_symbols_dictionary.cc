//---------------------------------------------------------------------------

#include "emily_symbols_dictionary.h"
#include "../str.h"
#include "../stroke.h"
#include <assert.h>

//---------------------------------------------------------------------------

constexpr StenoStroke ACTIVATION_MASK(StrokeMask::SL | StrokeMask::TL |
                                      StrokeMask::KL | StrokeMask::PL |
                                      StrokeMask::WL | StrokeMask::HL |
                                      StrokeMask::RL | StrokeMask::DR |
                                      StrokeMask::ZR | StrokeMask::UNICODE);

constexpr StenoStroke ACTIVATION_MATCH(StrokeMask::SL | StrokeMask::KL |
                                       StrokeMask::WL | StrokeMask::HL);

constexpr StenoStroke CAPITALIZE_MASK(StrokeMask::STAR);

constexpr StenoStroke LEFT_SPACE_MASK(StrokeMask::A);
constexpr StenoStroke RIGHT_SPACE_MASK(StrokeMask::O);

constexpr StenoStroke VARIANT_1(StrokeMask::E);
constexpr StenoStroke VARIANT_2(StrokeMask::U);

constexpr StenoStroke REPEAT_EXTRA_1(StrokeMask::SR);
constexpr StenoStroke REPEAT_EXTRA_2(StrokeMask::TR);

constexpr StenoStroke DATA_MASK(StrokeMask::FR | StrokeMask::RR |
                                StrokeMask::PR | StrokeMask::BR |
                                StrokeMask::LR | StrokeMask::GR);

//---------------------------------------------------------------------------

StenoEmilySymbolsDictionary
    StenoEmilySymbolsDictionary::specifySpacesInstance(true);
StenoEmilySymbolsDictionary
    StenoEmilySymbolsDictionary::specifyGlueInstance(false);

//---------------------------------------------------------------------------

struct EmilySymbolData {
  StenoStroke trigger;
  const char *text[4];
};

constexpr EmilySymbolData DATA[] = {
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::GR),
        .text = {"{#Tab}", "{#Backspace}", "{#Delete}", "{#Escape}"},
    },
    {
        .trigger = StenoStroke(StrokeMask::RR | StrokeMask::PR |
                               StrokeMask::BR | StrokeMask::GR),
        .text = {"{#Up}", "{#Left}", "{#Right}", "{#Down}"},
    },
    {
        .trigger =
            StenoStroke(StrokeMask::FR | StrokeMask::RR | StrokeMask::PR |
                        StrokeMask::BR | StrokeMask::GR),
        .text = {"{#Page_Up}", "{#Home}", "{#End}", "{#Page_Down}"},
    },
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::RR |
                               StrokeMask::BR | StrokeMask::GR),
        .text = {"{#AudioPlay}", "{#AudioPrev}", "{#AudioNext}",
                 "{#AudioStop}"},
    },
    {
        .trigger =
            StenoStroke(StrokeMask::FR | StrokeMask::RR | StrokeMask::BR),
        .text = {"{#AudioMute}", "{#AudioLowerVolume}", "{#AudioRaiseVolume}",
                 "{#Eject}"},
    },
    {
        .trigger = StenoStroke(),
        .text = {"", "{*!}", "{*?}", "{#Space}"},
    },
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::LR),
        .text = {"{*-|}", "{*<}", "{<}", "{*>}"},
    },

    // Type-able symbols.
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::RR),
        .text = {"!", "¬", "↦", "¡"},
    },
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::PR),
        .text = {"\"", "“", "”", "„"},
    },
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::RR |
                               StrokeMask::LR | StrokeMask::GR),
        .text = {"#", "©", "®", "™"},
    },
    {
        .trigger = StenoStroke(StrokeMask::RR | StrokeMask::PR |
                               StrokeMask::BR | StrokeMask::LR),
        .text = {"$", "¥", "€", "£"},
    },
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::RR |
                               StrokeMask::PR | StrokeMask::BR),
        .text = {"%", "‰", "‱", "φ"},
    },
    {
        .trigger =
            StenoStroke(StrokeMask::FR | StrokeMask::BR | StrokeMask::GR),
        .text = {"&", "∩", "∧", "∈"},
    },
    {
        .trigger = StenoStroke(StrokeMask::FR),
        .text = {"'", "‘", "’", "‚"},
    },
    {
        .trigger =
            StenoStroke(StrokeMask::FR | StrokeMask::PR | StrokeMask::LR),
        .text = {"(", "[", "<", "\\{"},
    },
    {
        .trigger =
            StenoStroke(StrokeMask::RR | StrokeMask::BR | StrokeMask::GR),
        .text = {")", "]", ">", "\\}"},
    },
    {
        .trigger = StenoStroke(StrokeMask::LR),
        .text = {"*", "∏", "§", "×"},
    },
    {
        .trigger = StenoStroke(StrokeMask::GR),
        .text = {"+", "∑", "¶", "±"},
    },
    {
        .trigger = StenoStroke(StrokeMask::BR),
        .text = {",", "∪", "∨", "∉"},
    },
    {
        .trigger = StenoStroke(StrokeMask::PR | StrokeMask::LR),
        .text = {"-", "−", "–", "—"},
    },
    {
        .trigger = StenoStroke(StrokeMask::RR),
        .text = {".", "•", "·", "…"},
    },
    {
        .trigger = StenoStroke(StrokeMask::RR | StrokeMask::PR),
        .text = {"/", "⇒", "⇔", "÷"},
    },
    {
        .trigger = StenoStroke(StrokeMask::LR | StrokeMask::GR),
        .text = {":", "∋", "∵", "∴"},
    },
    {
        .trigger = StenoStroke(StrokeMask::RR | StrokeMask::BR),
        .text = {";", "∀", "∃", "∄"},
    },
    {
        .trigger = StenoStroke(StrokeMask::PR | StrokeMask::BR |
                               StrokeMask::LR | StrokeMask::GR),
        .text = {"=", "≡", "≈", "≠"},
    },
    {
        .trigger =
            StenoStroke(StrokeMask::FR | StrokeMask::PR | StrokeMask::BR),
        .text = {"?", "¿", "∝", "‽"},
    },
    {
        .trigger =
            StenoStroke(StrokeMask::FR | StrokeMask::RR | StrokeMask::PR |
                        StrokeMask::BR | StrokeMask::LR | StrokeMask::GR),
        .text = {"@", "⊕", "⊗", "∅"},
    },
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::BR),
        .text = {"\\\\", "Δ", "√", "∞"},
    },
    {
        .trigger =
            StenoStroke(StrokeMask::RR | StrokeMask::PR | StrokeMask::GR),
        .text = {"^", "«", "»", "°"},
    },
    {
        .trigger = StenoStroke(StrokeMask::BR | StrokeMask::GR),
        .text = {"_", "≤", "≥", "µ"},
    },
    {
        .trigger = StenoStroke(StrokeMask::PR),
        .text = {"`", "⊂", "⊃", "π"},
    },
    {
        .trigger = StenoStroke(StrokeMask::PR | StrokeMask::BR),
        .text = {"|", "⊤", "⊥", "¦"},
    },
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::PR |
                               StrokeMask::GR | StrokeMask::BR),
        .text = {"~", "⊆", "⊇", "˜"},
    },
    {
        .trigger = StenoStroke(StrokeMask::FR | StrokeMask::PR |
                               StrokeMask::BR | StrokeMask::LR),
        .text = {"↑", "←", "→", "↓"},
    },
};

constexpr size_t REVERSE_LOOKUP_SIZE = 256;
// Autogenerated.
constexpr uint32_t REVERSE_LOOKUP[256] = {
    0x00000005, 0,          0x3e872c00, 0,          0,          0x90fc8003,
    0xd0566404, 0xd2cc1d17, 0x90908b20, 0,          0xdeca1c05, 0xfd6d930e,
    0x6a71b816, 0xd6878415, 0,          0,          0,          0x97fd4f1e,
    0,          0x9000a80e, 0,          0x906ec320, 0,          0,
    0,          0,          0,          0xa5c5f60b, 0x3219f412, 0xed108114,
    0xa4deae1a, 0,          0,          0,          0,          0,
    0,          0,          0x1363f20f, 0x79630d0f, 0,          0x04732411,
    0,          0,          0,          0,          0xdb7a5110, 0xe3cb0919,
    0x3bafb81a, 0x6edd991d, 0x031ee01d, 0x22433400, 0,          0,
    0x69b05d18, 0xdc179516, 0x3fe42115, 0xe4a6cd1c, 0,          0,
    0,          0x9d2c9303, 0,          0x915bbe01, 0x3afb2901, 0x140e3616,
    0x0ed4e214, 0,          0xe6b09c05, 0x7e70e412, 0,          0,
    0,          0,          0x51114314, 0xc7d12d06, 0,          0x19c43407,
    0x21756c11, 0,          0,          0x1a048706, 0,          0x567c8714,
    0x1ea9f00a, 0xf911dd1c, 0xc8f17121, 0x32104209, 0,          0xa9fa8703,
    0,          0x09b92610, 0xee010b0a, 0xf40bb015, 0xe0035a1e, 0,
    0,          0xf7ca550a, 0xe3c2bf1b, 0,          0,          0,
    0x28c32019, 0,          0,          0x0762ae08, 0x47446e00, 0x0960ff0b,
    0x907e8512, 0x07a28713, 0xe01aaa08, 0xe0da8312, 0x000f6a0c, 0,
    0x97e0e402, 0,          0,          0,          0,          0xe7776e0d,
    0xe7b7470e, 0xf0a79110, 0x350a1d1b, 0x32f7fa1f, 0x26ff1021, 0,
    0x5ed1931c, 0x171fbc09, 0,          0,          0,          0,
    0,          0xd6803c04, 0,          0xe0fa7f1e, 0,          0,
    0x9a17b111, 0,          0,          0xd2c2c60b, 0x451ec40c, 0x77454502,
    0,          0,          0,          0,          0,          0xcf659010,
    0xb8790001, 0,          0,          0,          0x8bb1d21f, 0,
    0x1d768816, 0x8a6aa318, 0xe797bb20, 0,          0x1eb76d18, 0,
    0xbca01f00, 0,          0,          0,          0xc5d44604, 0x4ca88817,
    0xdb748a19, 0x63090617, 0,          0,          0,          0,
    0,          0x4a7d9801, 0x6464c219, 0,          0,          0,
    0,          0,          0x65bfb320, 0,          0,          0x19daa91d,
    0,          0,          0,          0,          0,          0x4bc54c17,
    0x7374141c, 0,          0,          0xbff64121, 0xc7d8c20f, 0xb6a7b51b,
    0,          0,          0,          0xf5e74f06, 0x99063b0b, 0x99c61213,
    0x074be407, 0x7e7e3f0d, 0x7ebe1611, 0x5c05f518, 0,          0,
    0,          0x9e6bff07, 0x7913fb08, 0x0a8b2305, 0x83f2c80a, 0x0977d40c,
    0xeecff90c, 0x79d3d215, 0x142eca1a, 0x9fb08e1e, 0x897b2909, 0xfaa22a06,
    0xb1ca711b, 0,          0,          0x90705e0d, 0x90b0770f, 0,
    0,          0,          0x99ad1303, 0x77085a0d, 0x29d6a31d, 0,
    0,          0,          0x45f0ca1f, 0,          0x87a0a11f, 0x92095c02,
    0x67754807, 0x2ebb670e, 0x51f82021, 0xf11c9b04, 0,          0,
    0,          0xb87c3d13, 0x971d9a08, 0x97ddb313, 0x8fcc1202, 0xfa20ab1a,
    0,          0,          0,          0x70659e09,
};
//---------------------------------------------------------------------------

static const EmilySymbolData *LookupDataStroke(StenoStroke stroke) {
  for (size_t i = 0; i < sizeof(DATA) / sizeof(*DATA); ++i) {
    if (DATA[i].trigger == stroke)
      return &DATA[i];
  }
  return nullptr;
}

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoEmilySymbolsDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);
  const StenoStroke s = lookup.strokes[0];
  if ((s & ACTIVATION_MASK) != ACTIVATION_MATCH) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  const EmilySymbolData *data = LookupDataStroke(s & DATA_MASK);
  if (data == nullptr) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  int variant = 0;
  if ((s & VARIANT_1).IsNotEmpty()) {
    variant += 1;
  }
  if ((s & VARIANT_2).IsNotEmpty()) {
    variant += 2;
  }
  const char *text = data->text[variant];

  const char *capitalize = (s & CAPITALIZE_MASK).IsNotEmpty() ? "{-|}" : "";
  const char *leftSpace =
      (s & LEFT_SPACE_MASK).IsNotEmpty() == isSpecifySpacesMode ? "{}" : "{^}";
  const char *rightSpace =
      (s & RIGHT_SPACE_MASK).IsNotEmpty() == isSpecifySpacesMode ? "{}" : "{^}";

  if (text[0] == '{' && (Str::Eq(text, "{*!}") || Str::Eq(text, "{*?}"))) {
    leftSpace = "";
    rightSpace = "";
  }

  const char *r1 = (s & REPEAT_EXTRA_1).IsNotEmpty() ? text : "";
  const char *r2 = (s & REPEAT_EXTRA_2).IsNotEmpty() ? text : "";

  return StenoDictionaryLookupResult::CreateDynamicString(
      Str::Join(leftSpace, text, r1, r2, r2, rightSpace, capitalize));
}

const StenoDictionary *StenoEmilySymbolsDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);
  const StenoStroke stroke = lookup.strokes[0];
  if ((stroke & ACTIVATION_MASK) != ACTIVATION_MATCH) {
    return nullptr;
  }

  const EmilySymbolData *data = LookupDataStroke(stroke & DATA_MASK);
  return data != nullptr ? this : nullptr;
}

void StenoEmilySymbolsDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  if (lookup.definitionLength > 2 && lookup.definition[0] != '{') {
    return;
  }

  uint32_t index = lookup.definitionCrc;
  const uint32_t upperHash = lookup.definitionCrc >> 8;

  for (;;) {
    index &= (REVERSE_LOOKUP_SIZE - 1);
    const uint32_t data = REVERSE_LOOKUP[index];
    if (data == 0) {
      return;
    }

    if ((data >> 8) != upperHash) {
      ++index;
      continue;
    }

    const EmilySymbolData *symbolData = DATA + (data & 0xff);
    for (size_t i = 0; i < 4; ++i) {
      if (Str::Eq(lookup.definition, symbolData->text[i])) {
        // Have a match!
        static constexpr StenoStroke VARIANTS_STROKE[4] = {
            StenoStroke(0),
            StenoStroke(StrokeMask::E),
            StenoStroke(StrokeMask::U),
            StenoStroke(StrokeMask::E | StrokeMask::U),
        };
        const StenoStroke stroke =
            symbolData->trigger | ACTIVATION_MATCH | VARIANTS_STROKE[i];
        lookup.AddResult(&stroke, 1, this);
        return;
      }
    }
    return;
  }
}

const char *StenoEmilySymbolsDictionary::GetName() const {
  return "emily-symbols";
}

void StenoEmilySymbolsDictionary::PrintDictionary(
    PrintDictionaryContext &context) const {
  if (!context.HasName()) {
    return;
  }

  for (size_t i = 0; i < sizeof(DATA) / sizeof(*DATA); ++i) {
    const StenoStroke stroke = ACTIVATION_MATCH | DATA[i].trigger;

    for (int v = 0; v < 4; v++) {
      const StenoStroke variant = StenoStroke(v << StrokeBitIndex::E);

      for (int s = 0; s < 4; s++) {
        const StenoStroke space = StenoStroke(s << StrokeBitIndex::A);

        for (int c = 0; c < 2; c++) {
          const StenoStroke capitalize = StenoStroke(c << StrokeBitIndex::STAR);

          const StenoStroke localStroke = stroke | capitalize | space | variant;

          StenoDictionaryLookupResult lookup =
              Lookup(StenoDictionaryLookup(&localStroke, 1));
          context.Print(localStroke, lookup.GetText());
          lookup.Destroy();
        }
      }
    }
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#define BUILD_REVERSE_HASH_TABLE 0

#include "../unit_test.h"

static void VerifyStroke(const char *stroke, const char *result) {
  StenoStroke stenoStroke;
  stenoStroke.Set(stroke);

  auto lookup = StenoEmilySymbolsDictionary::specifySpacesInstance.Lookup(
      &stenoStroke, 1);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), result));
  lookup.Destroy();
}

#if BUILD_REVERSE_HASH_TABLE
static void BuildReverseLookupTable() {
  size_t entries = 4 * sizeof(DATA) / sizeof(*DATA);
  size_t minimumHashTableSize = entries + entries / 2;
  size_t hashTableSize = 1;
  while (hashTableSize < minimumHashTableSize) {
    hashTableSize <<= 1;
  }
  assert(hashTableSize <= 256);

  uint32_t *hashValues = new uint32_t[hashTableSize];
  const EmilySymbolData **hashTable =
      new const EmilySymbolData *[hashTableSize];
  memset(hashTable, 0, hashTableSize * sizeof(const EmilySymbolData *));

  for (const EmilySymbolData &data : DATA) {
    for (size_t i = 0; i < 4; ++i) {
      uint32_t hash = Crc32(data.text[i], Str::Length(data.text[i]));
      size_t index = hash;
      for (;;) {
        index &= (hashTableSize - 1);
        if (hashTable[index] == nullptr) {
          hashTable[index] = &data;
          hashValues[index] = hash;
          break;
        }
        ++index;
      }
    }
  }

  printf("constexpr uint32_t REVERSE_LOOKUP[%zu] = {\n", hashTableSize);
  for (size_t i = 0; i < hashTableSize; ++i) {
    if (hashTable[i] == nullptr) {
      printf("0,\n");
    } else {
      printf("0x%08x,\n",
             (uint32_t)((hashTable[i] - DATA) | (hashValues[i] & 0xffffff00)));
    }
  }
  printf("};\n");

  delete[] hashTable;
  delete[] hashValues;
}
#endif

TEST_BEGIN("EmilySymbolsDictionary tests") {
  // spellchecker: disable
  VerifyStroke("SKWHE", "{*!}");
  VerifyStroke("SKWH-R", "{^}.{^}");
  VerifyStroke("SKWH-RS", "{^}..{^}");
  VerifyStroke("SKWH-RT", "{^}...{^}");
  VerifyStroke("SKWH-RTS", "{^}....{^}");
  VerifyStroke("SKWHAR", "{}.{^}");
  VerifyStroke("SKWHOR", "{^}.{}");
  VerifyStroke("SKWHAOR", "{}.{}");
  VerifyStroke("SKWHAO*R", "{}.{}{-|}");
  VerifyStroke("SKWH-R", "{^}.{^}");
  VerifyStroke("SKWHER", "{^}•{^}");
  VerifyStroke("SKWHUR", "{^}·{^}");
  VerifyStroke("SKWHEUR", "{^}…{^}");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("EmilySymbolsDictionary Reverse Lookup") {
#if BUILD_REVERSE_HASH_TABLE
  BuildReverseLookupTable();
#endif
  StenoReverseDictionaryLookup lookup("!");

  StenoEmilySymbolsDictionary::specifySpacesInstance.ReverseLookup(lookup);
  assert(lookup.HasResults());
  assert(lookup.results[0].length == 1);

  // spellchecker: disable
  assert(lookup.results[0].strokes[0] == StenoStroke("SKWHFR"));
  // spellchecker: enable
}
TEST_END

//---------------------------------------------------------------------------
