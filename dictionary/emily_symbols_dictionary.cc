//---------------------------------------------------------------------------

#include "emily_symbols_dictionary.h"
#include "../console.h"
#include "../str.h"
#include "../stroke.h"
#include <assert.h>

//---------------------------------------------------------------------------

const StenoStroke ACTIVATION_MASK(StrokeMask::SL | StrokeMask::TL |
                                  StrokeMask::KL | StrokeMask::PL |
                                  StrokeMask::WL | StrokeMask::HL |
                                  StrokeMask::RL | StrokeMask::DR |
                                  StrokeMask::ZR | StrokeMask::UNICODE);

const StenoStroke ACTIVATION_MATCH(StrokeMask::SL | StrokeMask::KL |
                                   StrokeMask::WL | StrokeMask::HL);

const StenoStroke CAPITALIZE_MASK(StrokeMask::STAR);

const StenoStroke LEFT_SPACE_MASK(StrokeMask::A);
const StenoStroke RIGHT_SPACE_MASK(StrokeMask::O);

const StenoStroke VARIANT_1(StrokeMask::E);
const StenoStroke VARIANT_2(StrokeMask::U);

const StenoStroke REPEAT_EXTRA_1(StrokeMask::SR);
const StenoStroke REPEAT_EXTRA_2(StrokeMask::TR);

const StenoStroke DATA_MASK(StrokeMask::FR | StrokeMask::RR | StrokeMask::PR |
                            StrokeMask::BR | StrokeMask::LR | StrokeMask::GR);

//---------------------------------------------------------------------------

StenoEmilySymbolsDictionary StenoEmilySymbolsDictionary::instance;

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
  const StenoStroke c = lookup.strokes[0];
  if ((c & ACTIVATION_MASK) != ACTIVATION_MATCH) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  const EmilySymbolData *data = LookupDataStroke(c & DATA_MASK);
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

  return StenoDictionaryLookupResult::CreateDynamicString(
      Str::Join(leftSpace, text, r1, r2, r2, rightSpace, capitalize, nullptr));
}

const StenoDictionary *StenoEmilySymbolsDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);
  const StenoStroke c = lookup.strokes[0];
  if ((c & ACTIVATION_MASK) != ACTIVATION_MATCH) {
    return nullptr;
  }

  const EmilySymbolData *data = LookupDataStroke(c & DATA_MASK);
  return data != nullptr ? this : nullptr;
}

const char *StenoEmilySymbolsDictionary::GetName() const {
  return "emily-symbols";
}

bool StenoEmilySymbolsDictionary::PrintDictionary(const char *name,
                                                  bool hasData) const {
  for (size_t i = 0; i < sizeof(DATA) / sizeof(*DATA); ++i) {
    StenoStroke stroke = ACTIVATION_MATCH | DATA[i].trigger;

    for (int j = 0; j < 4; ++j) {
      StenoStroke localStroke = stroke;
      if (j & 1) {
        localStroke |= VARIANT_1;
      }
      if (j & 2) {
        localStroke |= VARIANT_2;
      }

      if (!hasData) {
        hasData = true;
        Console::Printf("\n\t");
      } else {
        Console::Printf(",\n\t");
      }

      Console::Printf("\"%t\": \"%J\"", &localStroke, DATA[i].text[j]);
    }
  }
  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"

static void VerifyStroke(const char *stroke, const char *result) {
  StenoStroke stenoStroke;
  stenoStroke.Set(stroke);

  auto lookup = StenoEmilySymbolsDictionary::instance.Lookup(&stenoStroke, 1);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), result));
  lookup.Destroy();
}

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

//---------------------------------------------------------------------------
