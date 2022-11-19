//---------------------------------------------------------------------------

#include "emily_symbols_dictionary.h"
#include "../chord.h"
#include "../console.h"
#include "../str.h"
#include <assert.h>

//---------------------------------------------------------------------------

const StenoChord ACTIVATION_MASK(ChordMask::SL | ChordMask::TL | ChordMask::KL |
                                 ChordMask::PL | ChordMask::WL | ChordMask::HL |
                                 ChordMask::RL | ChordMask::DR | ChordMask::ZR);

const StenoChord ACTIVATION_MATCH(ChordMask::SL | ChordMask::KL |
                                  ChordMask::WL | ChordMask::HL);

const StenoChord CAPITALIZE_MASK(ChordMask::STAR);

const StenoChord LEFT_SPACE_MASK(ChordMask::A);
const StenoChord RIGHT_SPACE_MASK(ChordMask::O);

const StenoChord VARIANT_1(ChordMask::E);
const StenoChord VARIANT_2(ChordMask::U);

const StenoChord REPEAT_EXTRA_1(ChordMask::SR);
const StenoChord REPEAT_EXTRA_2(ChordMask::TR);

const StenoChord DATA_MASK(ChordMask::FR | ChordMask::RR | ChordMask::PR |
                           ChordMask::BR | ChordMask::LR | ChordMask::GR);

struct EmilySymbolData {
  StenoChord trigger;
  const char *text[4];
};

const StenoEmilySymbolsDictionary StenoEmilySymbolsDictionary::instance;

constexpr EmilySymbolData DATA[] = {
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::GR),
        .text = {"{#Tab}", "{#Backspace}", "{#Delete}", "{#Escape}"},
    },
    {
        .trigger = StenoChord(ChordMask::RR | ChordMask::PR | ChordMask::BR |
                              ChordMask::GR),
        .text = {"{#Up}", "{#Left}", "{#Right}", "{#Down}"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::RR | ChordMask::PR |
                              ChordMask::BR | ChordMask::GR),
        .text = {"{#Page_Up}", "{#Home}", "{#End}", "{#Page_Down}"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::RR | ChordMask::BR |
                              ChordMask::GR),
        .text = {"{#AudioPlay}", "{#AudioPrev}", "{#AudioNext}",
                 "{#AudioStop}"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::RR | ChordMask::BR),
        .text = {"{#AudioMute}", "{#AudioLowerVolume}", "{#AudioRaiseVolume}",
                 "{#Eject}"},
    },
    {
        .trigger = StenoChord(),
        .text = {"", "{*!}", "{*?}", "{#Space}"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::LR),
        .text = {"{*-|}", "{*<}", "{<}", "{*>}"},
    },

    // Type-able symbols.
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::RR),
        .text = {"!", "¬", "↦", "¡"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::PR),
        .text = {"\"", "“", "”", "„"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::RR | ChordMask::LR |
                              ChordMask::GR),
        .text = {"#", "©", "®", "™"},
    },
    {
        .trigger = StenoChord(ChordMask::RR | ChordMask::PR | ChordMask::BR |
                              ChordMask::LR),
        .text = {"$", "¥", "€", "£"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::RR | ChordMask::PR |
                              ChordMask::BR),
        .text = {"%", "‰", "‱", "φ"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::BR | ChordMask::GR),
        .text = {"&", "∩", "∧", "∈"},
    },
    {
        .trigger = StenoChord(ChordMask::FR),
        .text = {"'", "‘", "’", "‚"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::PR | ChordMask::LR),
        .text = {"(", "[", "<", "\\{"},
    },
    {
        .trigger = StenoChord(ChordMask::RR | ChordMask::BR | ChordMask::GR),
        .text = {")", "]", ">", "\\}"},
    },
    {
        .trigger = StenoChord(ChordMask::LR),
        .text = {"*", "∏", "§", "×"},
    },
    {
        .trigger = StenoChord(ChordMask::GR),
        .text = {"+", "∑", "¶", "±"},
    },
    {
        .trigger = StenoChord(ChordMask::BR),
        .text = {",", "∪", "∨", "∉"},
    },
    {
        .trigger = StenoChord(ChordMask::PR | ChordMask::LR),
        .text = {"-", "−", "–", "—"},
    },
    {
        .trigger = StenoChord(ChordMask::RR),
        .text = {".", "•", "·", "…"},
    },
    {
        .trigger = StenoChord(ChordMask::RR | ChordMask::PR),
        .text = {"/", "⇒", "⇔", "÷"},
    },
    {
        .trigger = StenoChord(ChordMask::LR | ChordMask::GR),
        .text = {":", "∋", "∵", "∴"},
    },
    {
        .trigger = StenoChord(ChordMask::RR | ChordMask::BR),
        .text = {";", "∀", "∃", "∄"},
    },
    {
        .trigger = StenoChord(ChordMask::PR | ChordMask::BR | ChordMask::LR |
                              ChordMask::GR),
        .text = {"=", "≡", "≈", "≠"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::PR | ChordMask::BR),
        .text = {"?", "¿", "∝", "‽"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::RR | ChordMask::PR |
                              ChordMask::BR | ChordMask::LR | ChordMask::GR),
        .text = {"@", "⊕", "⊗", "∅"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::BR),
        .text = {"\\\\", "Δ", "√", "∞"},
    },
    {
        .trigger = StenoChord(ChordMask::RR | ChordMask::PR | ChordMask::GR),
        .text = {"^", "«", "»", "°"},
    },
    {
        .trigger = StenoChord(ChordMask::BR | ChordMask::GR),
        .text = {"_", "≤", "≥", "µ"},
    },
    {
        .trigger = StenoChord(ChordMask::PR),
        .text = {"`", "⊂", "⊃", "π"},
    },
    {
        .trigger = StenoChord(ChordMask::PR | ChordMask::BR),
        .text = {"|", "⊤", "⊥", "¦"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::PR | ChordMask::GR |
                              ChordMask::BR),
        .text = {"~", "⊆", "⊇", "˜"},
    },
    {
        .trigger = StenoChord(ChordMask::FR | ChordMask::PR | ChordMask::BR |
                              ChordMask::LR),
        .text = {"↑", "←", "→", "↓"},
    },
};

//---------------------------------------------------------------------------

static const EmilySymbolData *LookupDataChord(StenoChord chord) {
  for (size_t i = 0; i < sizeof(DATA) / sizeof(*DATA); ++i) {
    if (DATA[i].trigger == chord)
      return &DATA[i];
  }
  return nullptr;
}

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoEmilySymbolsDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);
  const StenoChord c = lookup.chords[0];
  if ((c & ACTIVATION_MASK) != ACTIVATION_MATCH) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  const EmilySymbolData *data = LookupDataChord(c & DATA_MASK);
  if (data == nullptr) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  int variant = 0;
  if ((c & VARIANT_1).IsNotEmpty()) {
    variant += 1;
  }
  if ((c & VARIANT_2).IsNotEmpty()) {
    variant += 2;
  }
  const char *text = data->text[variant];

  const char *capitalize = (c & CAPITALIZE_MASK).IsNotEmpty() ? "{-|}" : "";
  const char *leftSpace = (c & LEFT_SPACE_MASK).IsNotEmpty() ? "{}" : "{^}";
  const char *rightSpace = (c & RIGHT_SPACE_MASK).IsNotEmpty() ? "{}" : "{^}";

  if (Str::Eq(text, "{*!}") || Str::Eq(text, "{*?}")) {
    leftSpace = "";
    rightSpace = "";
  }

  const char *r1 = (c & REPEAT_EXTRA_1).IsNotEmpty() ? text : "";
  const char *r2 = (c & REPEAT_EXTRA_2).IsNotEmpty() ? text : "";

  return StenoDictionaryLookupResult::CreateDynamicString(Str::Asprintf(
      "%s%s%s%s%s%s%s", leftSpace, text, r1, r2, r2, rightSpace, capitalize));
}

const char *StenoEmilySymbolsDictionary::GetName() const {
  return "emily_symbols";
}

bool StenoEmilySymbolsDictionary::PrintDictionary(bool hasData) const {
  char chordBuffer[32];
  char translationBuffer[32];
  for (size_t i = 0; i < sizeof(DATA) / sizeof(*DATA); ++i) {
    StenoChord chord = ACTIVATION_MATCH | DATA[i].trigger;

    for (int j = 0; j < 4; ++j) {
      StenoChord localChord = chord;
      if (j & 1) {
        localChord |= VARIANT_1;
      }
      if (j & 2) {
        localChord |= VARIANT_2;
      }

      localChord.ToString(chordBuffer);

      char *p = Str::WriteJson(translationBuffer, DATA[i].text[j]);
      *p++ = '\0';

      if (!hasData) {
        hasData = true;
        Console::Write("\n\t", 2);
      } else {
        Console::Write(",\n\t", 3);
      }

      Console::Printf("\"%s\": \"%s\"", chordBuffer, translationBuffer);
    }
  }
  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"

static void VerifyChord(const char *chord, const char *result) {
  StenoChord stenoChord;
  stenoChord.Set(chord);

  auto lookup = StenoEmilySymbolsDictionary::instance.Lookup(&stenoChord, 1);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), result));
  lookup.Destroy();
}

TEST_BEGIN("EmilySymbolsDictionary tests") {
  // spellchecker: disable
  VerifyChord("SKWHE", "{*!}");
  VerifyChord("SKWH-R", "{^}.{^}");
  VerifyChord("SKWH-RS", "{^}..{^}");
  VerifyChord("SKWH-RT", "{^}...{^}");
  VerifyChord("SKWH-RTS", "{^}....{^}");
  VerifyChord("SKWHAR", "{}.{^}");
  VerifyChord("SKWHOR", "{^}.{}");
  VerifyChord("SKWHAOR", "{}.{}");
  VerifyChord("SKWHAO*R", "{}.{}{-|}");
  VerifyChord("SKWH-R", "{^}.{^}");
  VerifyChord("SKWHER", "{^}•{^}");
  VerifyChord("SKWHUR", "{^}·{^}");
  VerifyChord("SKWHEUR", "{^}…{^}");
  // spellchecker: enable
}
TEST_END

//---------------------------------------------------------------------------
