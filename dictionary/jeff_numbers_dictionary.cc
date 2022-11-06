//---------------------------------------------------------------------------

#include "jeff_numbers_dictionary.h"
#include "../chord.h"
#include "../console.h"
#include "../steno_key_code.h"
#include "../str.h"
#include <assert.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

const StenoJeffNumbersDictionary StenoJeffNumbersDictionary::instance;

//---------------------------------------------------------------------------

// Buffer must have at least 16 bytes
// Value is max 3999.
static void ToRoman(char *outBuffer, int value);
char *ToWords(char *p);

const StenoChord ALL_DIGITS_MASK(ChordMask::SL | ChordMask::TL | ChordMask::PL |
                                 ChordMask::HL | ChordMask::A | ChordMask::O |
                                 ChordMask::FR | ChordMask::PR | ChordMask::LR |
                                 ChordMask::TR);

const StenoChord CONTROL_MASK(ChordMask::KL | ChordMask::WL | ChordMask::RL |
                              ChordMask::STAR | ChordMask::RR | ChordMask::BR |
                              ChordMask::GR | ChordMask::SR | ChordMask::DR |
                              ChordMask::ZR);

constexpr StenoChord DIGIT_MASKS[] = {
    ChordMask::SL, ChordMask::TL, ChordMask::PL, ChordMask::HL, ChordMask::A,
    ChordMask::O,  ChordMask::FR, ChordMask::PR, ChordMask::LR, ChordMask::TR,
};
const char *const DIGIT_VALUES = "1234506789";

//---------------------------------------------------------------------------

static bool _EndsWith(char *p, size_t length, const char *suffix,
                      size_t suffixLength) {
  if (length < suffixLength)
    return false;
  return memcmp(p + length - suffixLength, suffix, suffixLength) == 0;
}

static char *ReplaceSuffix(char *original, size_t length, size_t offset,
                           const char *suffix) {
  assert(offset <= length);
  original[length - offset] = '\0';
  char *updatedResult = Str::Asprintf("%s%s", original, suffix);
  free(original);
  return updatedResult;
}

template <size_t N>
bool EndsWith(char *p, size_t length, const char (&suffix)[N]) {
  return _EndsWith(p, length, suffix, N - 1);
}

//---------------------------------------------------------------------------

StenoDictionaryLookup
StenoJeffNumbersDictionary::Lookup(const StenoChord *chords,
                                   size_t length) const {
  char *result = nullptr;
  char scratch[32];

  for (size_t i = 0; i < length; ++i) {
    if ((chords[i] & ChordMask::NUM).IsEmpty()) {
      free(result);
      return StenoDictionaryLookup::CreateInvalid();
    }

    StenoChord control = GetDigits(scratch, chords[i]);
    if (result) {
      char *updated = Str::Asprintf("%s%s", result, scratch);
      free(result);
      result = updated;
    } else {
      result = Str::Dup(scratch);
    }

    const StenoChord RB = ChordMask::RR | ChordMask::BR;
    const StenoChord WR = ChordMask::WL | ChordMask::RL;
    const StenoChord KR = ChordMask::KL | ChordMask::RL;
    const StenoChord RG = ChordMask::RR | ChordMask::GR;
    const StenoChord DZ = ChordMask::DR | ChordMask::ZR;
    const StenoChord BG = ChordMask::BR | ChordMask::GR;
    if ((control & RB) == RB) {
      // Dollars
      if (i + 1 != length) {
        free(result);
        return StenoDictionaryLookup::CreateInvalid();
      }
      control &= ~RB;
      char *updated = Str::Asprintf("$%s", result);
      free(result);
      result = updated;
    } else if ((control & WR) == WR) {
      // Dollars
      if (i + 1 != length) {
        free(result);
        return StenoDictionaryLookup::CreateInvalid();
      }
      control &= ~WR;
      char *updated = Str::Asprintf("$%s", result);
      free(result);
      result = updated;
    } else if ((control & KR) == KR) {
      // Percent
      if (i + 1 != length) {
        free(result);
        return StenoDictionaryLookup::CreateInvalid();
      }
      control &= ~KR;
      char *updated = Str::Asprintf("%s%%", result);
      free(result);
      result = updated;
    } else if ((control & RG) == RG) {
      // Percent
      if (i + 1 != length) {
        free(result);
        return StenoDictionaryLookup::CreateInvalid();
      }
      control &= ~RG;
      char *updated = Str::Asprintf("%s%%", result);
      free(result);
      result = updated;
    } else if ((control & DZ) == DZ) {
      // Hundreds of dollars.
      if (i + 1 != length) {
        free(result);
        return StenoDictionaryLookup::CreateInvalid();
      }
      control &= ~DZ;
      char *updated = Str::Asprintf("$%s00", result);
      free(result);
      result = updated;
    } else if ((control & ChordMask::KL).IsNotEmpty() ||
               ((control & BG) == BG)) {
      // Time
      if (i + 1 != length) {
        free(result);
        return StenoDictionaryLookup::CreateInvalid();
      }
      const char *minutes;
      if ((control & ChordMask::KL).IsEmpty()) {
        minutes = ":00";
      } else if ((control & BG) == BG) {
        minutes = ":45";
      } else if ((control & ChordMask::GR).IsNotEmpty()) {
        minutes = ":15";
      } else if ((control & ChordMask::BR).IsNotEmpty()) {
        minutes = ":30";
      } else {
        minutes = ":00";
      }

      const char *suffix = "";
      if ((control & ChordMask::SR).IsNotEmpty()) {
        suffix = (control & ChordMask::STAR).IsNotEmpty() ? " p.m." : " a.m.";
      }

      char *updated = Str::Asprintf("%s%s%s", result, minutes, suffix);
      free(result);
      result = updated;
      control &=
          ~(ChordMask::KL | ChordMask::BR | ChordMask::GR | ChordMask::SR);
    } else if ((control & ChordMask::GR).IsNotEmpty()) {
      if (i + 1 != length) {
        free(result);
        return StenoDictionaryLookup::CreateInvalid();
      }

      result = ToWords(result);
      if ((control & ChordMask::WL).IsNotEmpty()) {
        // Ordinal words
        size_t length = strlen(result);
        if (EndsWith(result, length, "ty")) {
          // cSpell: disable-next-line
          result = ReplaceSuffix(result, length, 1, "ieth");
        } else if (EndsWith(result, length, "one")) {
          result = ReplaceSuffix(result, length, 3, "first");
        } else if (EndsWith(result, length, "two")) {
          result = ReplaceSuffix(result, length, 3, "second");
        } else if (EndsWith(result, length, "three")) {
          result = ReplaceSuffix(result, length, 5, "third");
        } else if (EndsWith(result, length, "ve")) {
          result = ReplaceSuffix(result, length, 2, "fth");
        } else if (EndsWith(result, length, "eight")) {
          result = ReplaceSuffix(result, length, 0, "h");
        } else if (EndsWith(result, length, "nine")) {
          result = ReplaceSuffix(result, length, 1, "th");
        } else {
          result = ReplaceSuffix(result, length, 0, "th");
        }
      }

      control &= ~(ChordMask::GR | ChordMask::WL);
    } else if ((control & (ChordMask::WL | ChordMask::BR)).IsNotEmpty()) {
      // Ordinals
      if (i + 1 != length) {
        free(result);
        return StenoDictionaryLookup::CreateInvalid();
      }

      char secondLastDigit = '\0';
      char lastDigit = '\0';
      const char *p = result;
      while (*p) {
        if ('0' <= *p && *p <= '9') {
          secondLastDigit = lastDigit;
          lastDigit = *p;
        }
        ++p;
      }
      const char *suffix = "th";
      if (secondLastDigit != '1') {
        switch (lastDigit) {
        case '1':
          suffix = "st";
          break;
        case '2':
          suffix = "nd";
          break;
        case '3':
          suffix = "rd";
          break;
        }
      }

      char *updated = Str::Asprintf("%s%s", result, suffix);
      free(result);
      result = updated;

      control &= ~(ChordMask::WL | ChordMask::BR);
    } else if ((control & (ChordMask::RL | ChordMask::RR)).IsNotEmpty()) {
      // Roman numerals
      if (i + 1 != length) {
        free(result);
        return StenoDictionaryLookup::CreateInvalid();
      }

      int value = 0;
      const char *p = result;
      while (*p) {
        if ('0' <= *p && *p <= '9') {
          value = value * 10 + *p - '0';
        }
        ++p;
        if (value > 3999) {
          break;
        }
      }
      free(result);
      if (value <= 0 || value >= 3999) {
        return StenoDictionaryLookup::CreateInvalid();
      }
      ToRoman(scratch, value);
      result = Str::Dup(scratch);
      control &= ~(ChordMask::RL | ChordMask::RR);
    }

    control &= ~(ChordMask::STAR | ChordMask::DR | ChordMask::ZR);
    if (control.IsNotEmpty()) {
      free(result);
      return StenoDictionaryLookup::CreateInvalid();
    }
  }

  return StenoDictionaryLookup::CreateDynamicString(result);
}

void StenoJeffNumbersDictionary::PrintInfo() const {
  Console::Printf("      Jeff's Numbers\n");
}

StenoChord StenoJeffNumbersDictionary::GetDigits(char *p,
                                                 StenoChord chord) const {

  if ((chord & (ChordMask::E | ChordMask::U)).IsEmpty()) {
    // Normal direction
    for (int i = 0; i < 10; ++i) {
      if ((chord & DIGIT_MASKS[i]).IsNotEmpty()) {
        *p++ = DIGIT_VALUES[i];
      }
    }
  } else {
    // Reverse direction
    for (int i = 9; i >= 0; --i) {
      if ((chord & DIGIT_MASKS[i]).IsNotEmpty()) {
        *p++ = DIGIT_VALUES[i];
      }
    }
  }

  const StenoChord DZ = (ChordMask::DR | ChordMask::ZR);
  StenoChord control = chord & CONTROL_MASK;
  if ((control & DZ) != DZ) {
    if ((control & ChordMask::ZR).IsNotEmpty()) {
      if ((control & ChordMask::STAR).IsNotEmpty()) {
        *p++ = ',';
        *p++ = '0';
        control &= ~(ChordMask::STAR | ChordMask::ZR);
      }
      *p++ = '0';
      *p++ = '0';
    }

    if ((control & ChordMask::DR).IsNotEmpty() &&
        (chord & ALL_DIGITS_MASK).IsNotEmpty()) {
      char previousDigit = p[-1];
      *p++ = previousDigit;
      control &= ~ChordMask::DR;
    }
  }

  if (control == (ChordMask::STAR | ChordMask::SR)) {
    *p++ = ',';
    control &= ~(ChordMask::STAR | ChordMask::SR);
  } else if ((control & ChordMask::STAR).IsNotEmpty() &&
             (control & (ChordMask::RR | ChordMask::SR | ChordMask::ZR))
                 .IsEmpty()) {
    *p++ = '.';
    control &= ~ChordMask::STAR;
  }
  *p = '\0';
  return control;
}

struct RomanNumeralData {
  int value;
  const char *symbol;
};

constexpr RomanNumeralData ROMAN_NUMERAL_DATA[] = {
    {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
    {90, "XC"},  {50, "L"},   {40, "XL"}, {10, "X"},   {9, "IX"},
    {5, "V"},    {4, "IV"},   {1, "I"},
};

// Not the most efficient, but it's simple.
// outBuffer must have at least 16 bytes capacity.
static void ToRoman(char *outBuffer, int value) {
  assert(1 <= value && value <= 3999);
  outBuffer[0] = '\0';
  for (const RomanNumeralData &data : ROMAN_NUMERAL_DATA) {
    while (value >= data.value) {
      value -= data.value;
      strcat(outBuffer, data.symbol);
    }
  }
}

const char *const NUMBER_WORDS[] = {
    "zero",    "one",     "two",       "three",    "four",
    "five",    "six",     "seven",     "eight",    "nine",
    "ten",     "eleven",  "twelve",    "thirteen", "fourteen",
    "fifteen", "sixteen", "seventeen", "eighteen", "nineteen",
};

const char *const TENS[] = {
    "zero",  "ten",   "twenty",  "thirty", "forty",
    "fifty", "sixty", "seventy", "eighty", "ninety",
};

const char *HUNDRED = " hundred";

const char *const LARGE_SUM_WORDS[] = {
    "",
    " thousand",
    " million",
    " billion",
    " trillion",
    " quadrillion",
    " quintillion",
    " sextillion",
    " septillion",
    " octillion",
    " nonillion",
    " decillion",
    // spellchecker: disable
    " undecillion",
    " duodecillion",
    " tredecillion",
    " quattuordecillion",
    " quindecillion",
    " sexdecillion",
    " septendecillion",
    " octodecillion",
    " novemdecillion",
    " vigintillion",
    // spellchecker: enable
    " ???",
};

bool CanConvertToWords(char *digits) {
  for (char *p = digits; *p; ++p) {
    if ('0' <= *p && *p <= '9') {
      continue;
    }
    if (*p == ',') {
      continue;
    }
    return false;
  }
  return true;
}

// Remove commas and leading zeros.
void CanonicalizeInPlaceAndReverse(char *digits) {
  bool hasFoundValue = false;
  char *read = digits;
  char *write = digits;
  for (; *read; ++read) {
    if (*read == ',') {
      continue;
    }
    if (*read != '0') {
      hasFoundValue = true;
    }
    if (hasFoundValue) {
      *write++ = *read;
    }
  }
  *write = '\0';

  char *left = digits;
  char *right = write - 1;
  while (left < right) {
    char t = *left;
    *left = *right;
    *right = t;
    ++left;
    --right;
  }
}

char *ToWords(char *digits) {
  if (!CanConvertToWords(digits)) {
    return digits;
  }

  CanonicalizeInPlaceAndReverse(digits);

  char *p = digits;
  if (*p == '\0') {
    free(digits);
    return Str::Dup(NUMBER_WORDS[0]);
  }

  size_t length = strlen(p);
  char *result = Str::Dup("");
  char *end = p + length;
  bool needsAnd = false;
  bool needsComma = false;
  const char *const *largeSumWord = LARGE_SUM_WORDS;

  for (; p < end; largeSumWord++) {
    if (largeSumWord ==
        LARGE_SUM_WORDS + sizeof(LARGE_SUM_WORDS) / sizeof(*LARGE_SUM_WORDS)) {
      --largeSumWord;
    }

    int value = 0;
    int positionValue = 1;
    int digitValues[3] = {0, 0, 0};
    int *pDigitValues = &digitValues[2];
    while (p < end && positionValue < 1000) {
      int digitValue = *p++ - '0';
      *pDigitValues-- = digitValue;
      value += positionValue * digitValue;
      positionValue *= 10;
    }

    if (value == 0) {
      // Nothing in this group of 3.
      continue;
    }

    int twoDigitValue = digitValues[1] * 10 + digitValues[2];
    const char *twoDigitWord = "";
    bool freeTwoDigitWord = false;
    if (twoDigitValue != 0) {
      if (twoDigitValue < 20) {
        twoDigitWord = NUMBER_WORDS[twoDigitValue];
      } else if (digitValues[2] == 0) {
        twoDigitWord = TENS[digitValues[1]];
      } else {
        twoDigitWord = Str::Asprintf("%s-%s", TENS[digitValues[1]],
                                     NUMBER_WORDS[digitValues[2]]);
        freeTwoDigitWord = true;
      }
    }

    const char *separator = *result == '\0' ? ""
                            : needsAnd      ? " and "
                            : needsComma    ? ", "
                                            : " ";

    char *updatedResult;
    if (digitValues[0] != 0) {
      if (twoDigitValue == 0) {
        updatedResult =
            Str::Asprintf("%s%s%s%s%s", NUMBER_WORDS[digitValues[0]], HUNDRED,
                          *largeSumWord, separator, result);
      } else {
        updatedResult = Str::Asprintf(
            "%s%s and %s%s%s%s", NUMBER_WORDS[digitValues[0]], HUNDRED,
            twoDigitWord, *largeSumWord, separator, result);
      }
    } else {
      updatedResult = Str::Asprintf("%s%s%s%s", twoDigitWord, *largeSumWord,
                                    separator, result);
    }
    free(result);
    result = updatedResult;

    needsComma = largeSumWord != LARGE_SUM_WORDS;
    needsAnd = largeSumWord == LARGE_SUM_WORDS && digitValues[0] == 0;

    if (freeTwoDigitWord) {
      free((char *)twoDigitWord);
    }
  }

  free(digits);
  return result;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"
#include <assert.h>

static void TestLookup(const char *stroke, const char *expected) {
  StenoChord chord(stroke);
  StenoDictionaryLookup lookup =
      StenoJeffNumbersDictionary::instance.Lookup(&chord, 1);
  assert(lookup.IsValid());
  const char *text = lookup.GetText();
  assert(Str::Eq(text, expected));
  lookup.Destroy();
}

static void TestLookup(const StenoChord *chords, size_t length,
                       const char *expected) {
  StenoDictionaryLookup lookup =
      StenoJeffNumbersDictionary::instance.Lookup(chords, length);
  assert(lookup.IsValid());
  const char *text = lookup.GetText();
  assert(Str::Eq(text, expected));
  lookup.Destroy();
}

TEST_BEGIN("JeffNumbers: Test basic digits") {
  TestLookup("#S", "1");
  TestLookup("#T", "2");
  TestLookup("#P", "3");
  TestLookup("#H", "4");
  TestLookup("#A", "5");
  TestLookup("#O", "0");
  TestLookup("#-F", "6");
  TestLookup("#-P", "7");
  TestLookup("#-L", "8");
  TestLookup("#-T", "9");
}
TEST_END

TEST_BEGIN("JeffNumbers: Test multiple digits") {
  // spellchecker: disable
  TestLookup("#SD", "11");
  TestLookup("#ST", "12");
  TestLookup("#STE", "21");
  TestLookup("#STU", "21");
  TestLookup("#STEU", "21");

  TestLookup("#STP", "123");
  TestLookup("#STP-D", "1233");
  TestLookup("#STPED", "3211");

  TestLookup("#STPZ", "12300");
  TestLookup("#STP*Z", "123,000");

  TestLookup("#S*D", "11.");

  const StenoChord chords[] = {
      StenoChord("#ST*"),
      StenoChord("#PH"),
  };
  // spellchecker: enable
  TestLookup(chords, 2, "12.34");
}
TEST_END

TEST_BEGIN("JeffNumbers: Test money") {
  // spellchecker: disable
  TestLookup("#ST-RB", "$12");
  TestLookup("#STWR", "$12");

  const StenoChord chords[] = {
      StenoChord("#ST*"),
      StenoChord("#PWHR"),
  };
  // spellchecker: enable
  TestLookup(chords, 2, "$12.34");
}
TEST_END

TEST_BEGIN("JeffNumbers: Test percents") {
  // spellchecker: disable
  TestLookup("#ST-RG", "12%");
  TestLookup("#STKR", "12%");

  const StenoChord chords[] = {
      StenoChord("#ST*"),
      StenoChord("#PKHR"),
  };
  // spellchecker: enable
  TestLookup(chords, 2, "12.34%");
}
TEST_END

TEST_BEGIN("JeffNumbers: Test time") {
  // spellchecker: disable
  TestLookup("#SK", "1:00");
  TestLookup("#S-BG", "1:00");
  TestLookup("#SK-G", "1:15");
  TestLookup("#SK-B", "1:30");
  TestLookup("#SK-BG", "1:45");

  TestLookup("#SK-BGS", "1:45 a.m.");
  TestLookup("#SK*BGS", "1:45 p.m.");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffNumbers: Test ordinals") {
  // spellchecker: disable
  TestLookup("#SW", "1st");
  TestLookup("#S-B", "1st");
  TestLookup("#SW-D", "11th");
  TestLookup("#STWE", "21st");

  TestLookup("#TW", "2nd");
  TestLookup("#T-B", "2nd");
  TestLookup("#STW", "12th");
  TestLookup("#TW-D", "22nd");

  TestLookup("#PW", "3rd");
  TestLookup("#P-B", "3rd");
  TestLookup("#SPW", "13th");
  TestLookup("#TPW", "23rd");

  TestLookup("#HW", "4th");
  TestLookup("#H-B", "4th");
  TestLookup("#SHW", "14th");
  TestLookup("#THW", "24th");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffNumbers: Test roman numerals") {
  // spellchecker: disable
  char buffer[16];
  ToRoman(buffer, 1);
  assert(Str::Eq(buffer, "I"));

  ToRoman(buffer, 3);
  assert(Str::Eq(buffer, "III"));

  ToRoman(buffer, 4);
  assert(Str::Eq(buffer, "IV"));

  ToRoman(buffer, 47);
  assert(Str::Eq(buffer, "XLVII"));

  ToRoman(buffer, 1977);
  assert(Str::Eq(buffer, "MCMLXXVII"));

  ToRoman(buffer, 3888);
  assert(Str::Eq(buffer, "MMMDCCCLXXXVIII"));

  TestLookup("#STR", "XII");
  TestLookup("#ST-R", "XII");
  // spellchecker: enable
}
TEST_END

static void TestWord(const char *input, const char *expectedOutput) {
  char *dupedInput = Str::Dup(input);
  char *result = ToWords(dupedInput);

  if (!Str::Eq(result, expectedOutput)) {
    printf("'%s' gave '%s', expected '%s'\n", input, result, expectedOutput);
    assert(Str::Eq(result, expectedOutput));
  }

  free(result);
}

TEST_BEGIN("JeffNumbers: Test words") {
  TestWord("0", "zero");
  TestWord("1", "one");
  TestWord("2", "two");
  TestWord("3", "three");
  TestWord("4", "four");
  TestWord("5", "five");
  TestWord("6", "six");
  TestWord("7", "seven");
  TestWord("8", "eight");
  TestWord("9", "nine");
  TestWord("10", "ten");
  TestWord("11", "eleven");
  TestWord("12", "twelve");
  TestWord("13", "thirteen");
  TestWord("14", "fourteen");
  TestWord("15", "fifteen");
  TestWord("16", "sixteen");
  TestWord("17", "seventeen");
  TestWord("18", "eighteen");
  TestWord("19", "nineteen");
  TestWord("20", "twenty");
  TestWord("21", "twenty-one");
  TestWord("22", "twenty-two");
  TestWord("23", "twenty-three");
  TestWord("100", "one hundred");
  TestWord("101", "one hundred and one");
  TestWord("147", "one hundred and forty-seven");
  TestWord("1000", "one thousand");
  TestWord("1001", "one thousand and one");
  TestWord("1100", "one thousand one hundred");
  TestWord("123100", "one hundred and twenty-three thousand one hundred");
  TestWord("1000000", "one million");
  TestWord("1,000,000", "one million");
  TestWord("1,000,070", "one million and seventy");
  TestWord("1,001,070", "one million, one thousand and seventy");
  TestWord("1,023,040", "one million, twenty-three thousand and forty");

  // spellchecker: disable
  TestLookup("#STG", "twelve");

  TestLookup("#WOG", "zeroth");
  TestLookup("#SWG", "first");
  TestLookup("#TWG", "second");
  TestLookup("#PWG", "third");
  TestLookup("#WHG", "fourth");
  TestLookup("#WAG", "fifth");
  TestLookup("#W-FG", "sixth");
  TestLookup("#W-PG", "seventh");
  TestLookup("#W-LG", "eighth");
  TestLookup("#W-TG", "ninth");
  TestLookup("#SWOG", "tenth");
  TestLookup("#SWGD", "eleventh");
  TestLookup("#STWG", "twelfth");
  TestLookup("#SPWG", "thirteenth");

  TestLookup("#TWOG", "twentieth");
  // spellchecker: enable
}
TEST_END

//---------------------------------------------------------------------------
