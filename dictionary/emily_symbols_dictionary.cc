//---------------------------------------------------------------------------

#include "emily_symbols_dictionary.h"
#include "../chord.h"
#include "../console.h"
#include "../string_util.h"
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

constexpr StenoEmilySymbolsDictionary StenoEmilySymbolsDictionary::instance;

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
        .text = {"\\", "Δ", "√", "∞"},
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

StenoDictionaryLookup
StenoEmilySymbolsDictionary::Lookup(const StenoChord *chords,
                                    size_t length) const {
  assert(length == 1);
  const StenoChord c = chords[0];
  if ((c & ACTIVATION_MASK) != ACTIVATION_MATCH) {
    return StenoDictionaryLookup::CreateInvalid();
  }

  const EmilySymbolData *data = LookupDataChord(c & DATA_MASK);
  if (data == nullptr) {
    return StenoDictionaryLookup::CreateInvalid();
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

  if (streq(text, "{*!}") || streq(text, "{*?}")) {
    leftSpace = "";
    rightSpace = "";
  }

  const char *r1 = (c & REPEAT_EXTRA_1).IsNotEmpty() ? text : "";
  const char *r2 = (c & REPEAT_EXTRA_2).IsNotEmpty() ? text : "";

  return StenoDictionaryLookup::CreateDynamicString(rasprintf(
      "%s%s%s%s%s%s%s", leftSpace, text, r1, r2, r2, rightSpace, capitalize));
}

void StenoEmilySymbolsDictionary::PrintInfo() const {
  Console::Printf("  Emily's Symbols\n");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"

static void VerifyChord(const char *chord, const char *result) {
  StenoChord stenoChord(chord);

  auto lookup = StenoEmilySymbolsDictionary::instance.Lookup(&stenoChord, 1);
  assert(lookup.IsValid());
  assert(streq(lookup.GetText(), result));
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
