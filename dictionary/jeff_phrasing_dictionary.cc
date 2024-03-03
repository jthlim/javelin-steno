//---------------------------------------------------------------------------

#include "jeff_phrasing_dictionary.h"
#include "../crc.h"
#include "../pattern.h"
#include "../str.h"
#include "../stroke.h"
#include "dictionary.h"
#include "jeff_phrasing_dictionary_data.h"
#include "jeff_phrasing_dictionary_generated.h"
#include <assert.h>

//---------------------------------------------------------------------------

// spellchecker: disable
const StenoStroke SIMPLE_STARTER_MASK(0x3fe);  // `STKPWHRAO`
const StenoStroke SIMPLE_PRONOUN_MASK(0x1C00); // `*EU`
const StenoStroke FULL_STARTER_MASK(0xfe);     // 'STKPWHR'
const StenoStroke ENDER_MASK(0x7fc000);        // '-RPBLGTSDZ'

const StenoStroke STRUCTURE_EXCEPTION1_MASK(0x3ffe); // STKPWHRAO*EUF
const StenoStroke STRUCTURE_EXCEPTION2_MASK(0x3f00); // AO*EUF
// spellchecker: enable

//---------------------------------------------------------------------------

StenoJeffPhrasingDictionary StenoJeffPhrasingDictionary::instance;

//---------------------------------------------------------------------------

struct StenoJeffPhrasingDictionary::ReverseLookupContext {
  ReverseLookupContext(StenoReverseDictionaryLookup &result) : result(result) {}

  bool hasPresentTenseResult = false;
  StenoReverseDictionaryLookup &result;
  List<StenoStroke> testedStrokes;

  bool HasTestedStroke(const StenoStroke &stroke) const {
    return testedStrokes.Contains(stroke);
  }
  void AddTestedStroke(const StenoStroke &stroke) { testedStrokes.Add(stroke); }

  void AddResult(const StenoStroke &stroke, const StenoDictionary *provider) {
    result.AddResult(&stroke, 1, provider);
  }
};

//---------------------------------------------------------------------------

struct PhrasingParts {
  PhrasingParts() = default;
  PhrasingParts(const JeffPhrasingPronoun *pronoun,
                const JeffPhrasingMiddle *middle,
                const JeffPhrasingStructure *structure,
                const JeffPhrasingEnder *ender)
      : pronoun(pronoun), middle(middle), structure(structure), ender(ender) {
    assert(pronoun != nullptr);
    assert(middle != nullptr);
    assert(structure != nullptr);
    assert(ender != nullptr);
  }

  static PhrasingParts CreateInvalid() {
    PhrasingParts result;
    result.pronoun = nullptr;
    return result;
  }

  bool IsValid() const {
    return pronoun != nullptr &&
           (pronoun->canUseAllEnders || ender->canUseAllStarters);
  }

  const JeffPhrasingPronoun *pronoun;
  const JeffPhrasingMiddle *middle;
  const JeffPhrasingStructure *structure;
  const JeffPhrasingEnder *ender;

  char *CreatePhrase() const;
};

//---------------------------------------------------------------------------

const JeffPhrasingVariant *JeffPhrasingMap::Lookup(uint32_t key) const {
  for (size_t i = 0; i < entryCount; ++i) {
    if (entries[i].key == key) {
      return &entries[i].value;
    }
  }
  return nullptr;
}

const JeffPhrasingVariant *JeffPhrasingVariant::Lookup(uint32_t key) const {
  if (type != Type::MAP) {
    return this;
  }
  return map->Lookup(key);
}

const JeffPhrasingVariant *
JeffPhrasingVariant::LookupWithDefaultOrSelf(uint32_t key) const {
  const JeffPhrasingVariant *result = Lookup(key);
  if (result) {
    return result;
  }
  result = Lookup(0);
  return result ? result : this;
}

bool JeffPhrasingDictionaryData::IsValidPhraseStroke(StenoStroke stroke) const {
  for (size_t i = 0; i < nonPhraseStrokeCount; ++i) {
    if (nonPhraseStrokes[i] == stroke) {
      return false;
    }
  }
  return true;
}

const JeffPhrasingMiddle *
JeffPhrasingDictionaryData::LookupSimpleStarter(StenoStroke stroke) const {
  for (size_t i = 0; i < simpleStarterCount; ++i) {
    if (simpleStarters[i].stroke == stroke) {
      return &simpleStarters[i].middle;
    }
  }
  return nullptr;
}

const JeffPhrasingPronoun *
JeffPhrasingDictionaryData::LookupFullStarter(StenoStroke stroke) const {
  for (size_t i = 0; i < fullStarterCount; ++i) {
    if (fullStarters[i].stroke == stroke) {
      return &fullStarters[i].pronoun;
    }
  }
  return nullptr;
}

const JeffPhrasingEnder *
JeffPhrasingDictionaryData::LookupEnder(StenoStroke stroke) const {
  size_t index = stroke.Hash();
  for (;;) {
    const JeffPhrasingEnder *ender =
        enderHashMap[index & (enderHashMapSize - 1)];

    if (ender == nullptr) {
      return nullptr;
    }
    if (ender->stroke == stroke) {
      return ender;
    }

    ++index;
  }

  // for (size_t i = 0; i < endersCount; ++i) {
  //   if (enders[i].stroke == stroke) {
  //     return &enders[i];
  //   }
  // }
  // return nullptr;
}

const JeffPhrasingStructure *
JeffPhrasingDictionaryData::LookupUniqueStarter(StenoStroke stroke) const {
  for (size_t i = 0; i < uniqueStarterCount; ++i) {
    if (uniqueStarters[i].stroke == stroke) {
      return &uniqueStarters[i].structure;
    }
  }
  return nullptr;
}

const JeffPhrasingStructure *
JeffPhrasingDictionaryData::LookupStructureException(StenoStroke stroke) const {
  for (size_t i = 0; i < structureExceptionCount; ++i) {
    if (structureExceptions[i].stroke == stroke) {
      return &structureExceptions[i].structure;
    }
  }
  return nullptr;
}

const JeffPhrasingReverseHashMapEntry *
JeffPhrasingDictionaryData::LookupReverseWord(uint32_t hash) const {
  size_t index = hash;
  for (;;) {
    const JeffPhrasingReverseHashMapEntry *entry =
        reverseHashMap[index & (reverseHashMapSize - 1)];
    if (entry == nullptr) {
      return nullptr;
    }
    if (entry->hash == hash) {
      return entry;
    }

    ++index;
  }
}

const JeffPhrasingReverseStructureEntry *
JeffPhrasingDictionaryData::LookupReverseStructure(uint32_t hash) const {
  size_t index = hash;
  for (;;) {
    const JeffPhrasingReverseStructureEntry *entry =
        reverseStructureHashMap[index & (reverseStructureHashMapSize - 1)];
    if (entry == nullptr) {
      return nullptr;
    }
    if (entry->hash == hash) {
      return entry;
    }
    ++index;
  }
}

//---------------------------------------------------------------------------

#define phrasingData JeffPhrasingDictionaryData::instance

PhrasingParts DetermineParts(StenoStroke stroke) {
  if ((stroke & SIMPLE_STARTER_MASK).IsEmpty()) {
    return PhrasingParts::CreateInvalid();
  }

  if ((stroke & (StrokeMask::NUM | StrokeMask::UNICODE)).IsNotEmpty()) {
    return PhrasingParts::CreateInvalid();
  }

  const JeffPhrasingEnder *ender =
      phrasingData.LookupEnder(stroke & ENDER_MASK);
  if (!ender) {
    return PhrasingParts::CreateInvalid();
  }

  if (!phrasingData.IsValidPhraseStroke(stroke)) {
    return PhrasingParts::CreateInvalid();
  }

  // Try simple starters first.
  if ((stroke & SIMPLE_PRONOUN_MASK).IsNotEmpty()) {
    const JeffPhrasingMiddle *simpleStarter =
        phrasingData.LookupSimpleStarter(stroke & SIMPLE_STARTER_MASK);

    if (simpleStarter) {
      size_t pronounIndex = (stroke.GetKeyState() >> StrokeBitIndex::STAR) & 7;
      const JeffPhrasingPronoun *pronoun =
          phrasingData.simplePronouns + pronounIndex;

      size_t structureIndex = (stroke.GetKeyState() >> StrokeBitIndex::FR) & 1;
      const JeffPhrasingStructure *structure =
          phrasingData.simpleStructures + structureIndex;

      return PhrasingParts(pronoun, simpleStarter, structure, ender);
    }
  }

  const JeffPhrasingPronoun *fullStarter =
      phrasingData.LookupFullStarter(stroke & FULL_STARTER_MASK);

  if (fullStarter != nullptr) {
    size_t middleIndex = (stroke.GetKeyState() >> StrokeBitIndex::A) & 7;
    const JeffPhrasingMiddle *middle = phrasingData.fullMiddles + middleIndex;

    const JeffPhrasingStructure *structure =
        phrasingData.LookupUniqueStarter(stroke & STRUCTURE_EXCEPTION1_MASK);

    if (!structure) {
      structure = phrasingData.LookupStructureException(
          stroke & STRUCTURE_EXCEPTION2_MASK);
    }

    if (!structure) {
      size_t structureIndex =
          (stroke.GetKeyState() >> StrokeBitIndex::STAR) & 15;
      structure = phrasingData.fullStructures + structureIndex;
    }
    return PhrasingParts(fullStarter, middle, structure, ender);
  }

  return PhrasingParts::CreateInvalid();
}

StenoDictionaryLookupResult
StenoJeffPhrasingDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);

  PhrasingParts parts = DetermineParts(lookup.strokes[0]);
  if (!parts.IsValid()) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  return StenoDictionaryLookupResult::CreateDynamicString(parts.CreatePhrase());
}

const StenoDictionary *StenoJeffPhrasingDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);

  PhrasingParts parts = DetermineParts(lookup.strokes[0]);
  return parts.IsValid() ? this : nullptr;
}

char *PhrasingParts::CreatePhrase() const {
  VerbForm verbForm = pronoun->verbForm;
  const char *middleText =
      middle->word.LookupWithDefaultOrSelf((uint32_t)ender->tense)
          ->LookupWithDefaultOrSelf((uint32_t)verbForm)
          ->ToString();

  if (structure->useMiddleVerbForm &&
      middle->verbForm != VerbForm::UNSPECIFIED) {
    verbForm = middle->verbForm;
  }

  const char *structureText =
      structure->format.LookupWithDefaultOrSelf((uint32_t)ender->tense)
          ->LookupWithDefaultOrSelf((uint32_t)verbForm)
          ->ToString();

  if (structure->updatedVerbForm != VerbForm::UNSPECIFIED) {
    verbForm = structure->updatedVerbForm;
  }

  const char *verbText =
      ender->ender.LookupWithDefaultOrSelf((uint32_t)verbForm)->ToString();

  // Abuse pattern replace code to substitute text.
  PatternMatch match;
  match.match = true;
  match.captures[0] = pronoun->word;
  match.captures[1] = pronoun->word + strlen(pronoun->word);
  match.captures[2] = middleText;
  match.captures[3] = middleText + strlen(middleText);
  match.captures[4] = verbText;
  match.captures[5] = verbText + strlen(verbText);
  match.captures[6] = ender->suffix;
  match.captures[7] = ender->suffix + strlen(ender->suffix);
  return match.Replace(structureText);
}

const char *StenoJeffPhrasingDictionary::GetName() const {
  return "jeff-phrasing";
}

void StenoJeffPhrasingDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  // Maximum phrase is 7 words (6 spaces).
  if (CountNumberOfSpaces(result.lookup) > 6) {
    return;
  }

  if (ContainsNonPhraseCharacter(result.lookup)) {
    return;
  }

  ReverseLookupContext context(result);
  RecurseCheckReverseLookup(context, result.lookup, StenoStroke(), 0, 0,
                            ModeMask::FULL | ModeMask::SIMPLE |
                                ModeMask::PRESENT | ModeMask::PAST);
}

void StenoJeffPhrasingDictionary::RecurseCheckReverseLookup(
    ReverseLookupContext &context, const char *p, StenoStroke stroke,
    uint32_t hash, uint8_t componentMask, uint8_t modeMask) const {
  while (*p == ' ' && *p != '\0') {
    ++p;
  }

  const char *pEnd = p;
  while (*pEnd != '\0' && *pEnd != ' ') {
    ++pEnd;
  }

  if (p == pEnd) {
    if ((componentMask & ComponentMask::STARTER) == 0) {
      // There must be a starter for simple forms.
      if ((modeMask & ~ModeMask::SIMPLE) == 0) {
        return;
      }

      componentMask |= ComponentMask::STARTER;
      hash += STARTER_REPLACEMENT_CRC; // CRC for "\\0"

      // Reverse empty starter check.
      for (const JeffPhrasingReverseHashMapEntry *entry =
               phrasingData.reverseEntries;
           entry->hash == 0; ++entry) {
        if (entry->componentMask == ComponentMask::STARTER) {
          RecurseCheckReverseLookup(context, p, stroke | entry->stroke, hash,
                                    componentMask, modeMask);
        }
      }
    }
    if ((componentMask & ComponentMask::MIDDLE) == 0) {
      // Simple forms must have a middle.
      componentMask |= ComponentMask::MIDDLE;
      if ((modeMask & ~ModeMask::SIMPLE) == 0) {
        return;
      }

      // Recurse, once with no change, then proceed with \\1 appended.
      RecurseCheckReverseLookup(context, p, stroke, hash, componentMask,
                                modeMask);
      hash += MIDDLE_REPLACEMENT_CRC; // CRC for "\\1"
    }

    if ((componentMask & ComponentMask::VERB) == 0) {
      hash += VERB_REPLACEMENT_CRC;   // CRC for "\\2"
      hash += SUFFIX_REPLACEMENT_CRC; // CRC for "\\3"

      if ((modeMask & (ModeMask::PRESENT | ModeMask::PAST)) == ModeMask::PAST) {
        stroke |= StenoStroke(StrokeMask::DR);
      }
    }

    // End of string. Do lookup & check.
    const JeffPhrasingReverseStructureEntry *entry =
        phrasingData.LookupReverseStructure(hash);
    if (!entry) {
      return;
    }
    for (; entry->hash == hash; ++entry) {
      // Try lookup.
      if (entry->modeMask & modeMask) {
        StenoStroke lookupStroke = stroke | entry->stroke;
        if (context.HasTestedStroke(lookupStroke)) {
          continue;
        }
        context.AddTestedStroke(lookupStroke);
        StenoDictionaryLookupResult lookup = Lookup(&lookupStroke, 1);
        if (lookup.IsValid()) {
          const char *lookupText = lookup.GetText();
          if (*lookupText == ' ') {
            ++lookupText;
          }
          if (Str::Eq(lookupText, context.result.lookup)) {
            context.AddResult(lookupStroke, this);
            if (modeMask & ModeMask::PRESENT) {
              context.hasPresentTenseResult = true;
            }
          }
          lookup.Destroy();
        }
      }
    }
  }

  uint32_t wordHash = Crc32(p, pEnd - p);
  const JeffPhrasingReverseHashMapEntry *entry =
      phrasingData.LookupReverseWord(wordHash);
  if (!entry) {
    return;
  }

  if (entry->checkNext && *pEnd != '\0') {
    const char *pEnd2 = pEnd;
    while (*pEnd2 == ' ' && *pEnd2 != '\0') {
      ++pEnd2;
    }

    while (*pEnd2 != '\0' && *pEnd2 != ' ') {
      ++pEnd2;
    }

    uint32_t wordHash2 = Crc32(p, pEnd2 - p);
    const JeffPhrasingReverseHashMapEntry *entry2 =
        phrasingData.LookupReverseWord(wordHash2);
    if (entry2) {
      ProcessEntries(entry2, wordHash2, context, pEnd2, stroke, hash,
                     componentMask, modeMask);
    }
  }

  ProcessEntries(entry, wordHash, context, pEnd, stroke, hash, componentMask,
                 modeMask);
}

void StenoJeffPhrasingDictionary::ProcessEntries(
    const JeffPhrasingReverseHashMapEntry *entry, uint32_t wordHash,
    ReverseLookupContext &context, const char *p, StenoStroke stroke,
    uint32_t hash, uint32_t componentMask, uint32_t modeMask) const {
  for (; entry->hash == wordHash; ++entry) {
    // Don't proceed if the phrase component has already been used.
    if (componentMask & entry->componentMask) {
      continue;
    }

    uint8_t updatedModeMask = entry->modeMask & modeMask;

    uint8_t formModeMask =
        updatedModeMask & (ModeMask::FULL | ModeMask::SIMPLE);
    if (formModeMask == 0) {
      continue;
    }

    uint8_t tenseModeMask =
        updatedModeMask & (ModeMask::PRESENT | ModeMask::PAST);
    if (tenseModeMask == 0) {
      continue;
    }

    // Prevent suggesting past forms when present forms are already suggested.
    // e.g. "to read" should suggest `STWRURS` and `STKPWHRURS`, but not with
    // `-Z` attached.
    if (context.hasPresentTenseResult && tenseModeMask == ModeMask::PAST) {
      continue;
    }

    if (entry->replaceHash != 0) {
      RecurseCheckReverseLookup(
          context, p, stroke | entry->stroke, hash + entry->replaceHash,
          componentMask | entry->componentMask, updatedModeMask);
    }
  }
}

inline bool IsValidPhraseCharacter(int c) {
  //  return ('a' <= c && c < 'q')
  //         || ('q' < c && c <= 'z')
  //         || c == 'I'
  //         || c == '\''
  //         || c == ' ';
  return (VALID_CHARACTERS[c / 8] & (1 << c % 8)) != 0;
}

bool StenoJeffPhrasingDictionary::ContainsNonPhraseCharacter(const char *p) {
  while (*p) {
    int c = *(uint8_t *)p++;
    if (!IsValidPhraseCharacter(c)) {
      return true;
    }
  }
  return false;
}

size_t StenoJeffPhrasingDictionary::CountNumberOfSpaces(const char *p) {
  size_t count = 0;
  while (*p) {
    if (*p == ' ') {
      ++count;
    }
    ++p;
  }
  return count;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"
#include <stdio.h>

static void VerifyStroke(const char *stroke, const char *result) {
  StenoStroke stenoStroke;
  stenoStroke.Set(stroke);

  auto lookup = StenoJeffPhrasingDictionary::instance.Lookup(&stenoStroke, 1);
  if (!lookup.IsValid()) {
    printf("Failed to lookup %s\n", stroke);

    assert(lookup.IsValid());
  }
  if (!Str::Eq(lookup.GetText(), result)) {
    printf("Lookup for %s expected '%s', got '%s'\n", stroke, result,
           lookup.GetText());

    assert(Str::Eq(lookup.GetText(), result));
  }
  lookup.Destroy();
}

TEST_BEGIN("JeffPhrasing: Starter tests") {
  // spellchecker: disable
  VerifyStroke("SWR", "I");
  VerifyStroke("KPWR", "you");

  VerifyStroke("KWHR", "he");
  VerifyStroke("SKWHR", "she");
  VerifyStroke("KPWH", "it");
  VerifyStroke("TWH", "they");
  VerifyStroke("STKH", "this");
  VerifyStroke("STWH", "that");
  //    VerifyStroke("STHR", "there");
  //    VerifyStroke("STPHR", "there");
  VerifyStroke("STKPWHR", "");
  VerifyStroke("STWR", "");

  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: \"There\" tests") {
  // spellchecker: disable
  VerifyStroke("STHRB", "there is");
  VerifyStroke("STPHRB", "there are");

  // There shouldn't be able to access all enders, eg. "ask"
  StenoStroke stroke("STHR-RB");
  auto lookup = StenoJeffPhrasingDictionary::instance.Lookup(&stroke, 1);
  assert(!lookup.IsValid());

  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Unique starters tests") {
  // spellchecker: disable
  VerifyStroke("STWRU", "to");
  VerifyStroke("STWR*U", "not to");
  VerifyStroke("STKPWHRU", "to");
  VerifyStroke("STKPWHR*U", "not to");
  VerifyStroke("STWRUF", " just");
  VerifyStroke("STWR*UF", " just don't");
  VerifyStroke("STKPWHRUF", " just");
  VerifyStroke("STKPWHR*UF", " just doesn't");
  VerifyStroke("STWREU", " still");
  VerifyStroke("STWR*EU", " still don't");
  VerifyStroke("STKPWHREU", " still");
  VerifyStroke("STKPWHR*EU", " still doesn't");
  VerifyStroke("STWREUF", " never");
  VerifyStroke("STWR*EUF", " don't even");
  VerifyStroke("STKPWHREUF", " never");
  VerifyStroke("STKPWHR*EUF", " doesn't even");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Simple phrases tests") {
  // spellchecker: disable
  VerifyStroke("SKPEU", "and I");
  VerifyStroke("SKPEUF", "and I have");
  VerifyStroke("SKPEUFD", "and I had");
  VerifyStroke("SKPEURB", "and I ask");
  VerifyStroke("SKPEURBD", "and I asked");
  VerifyStroke("SKPEUFRB", "and I have asked");
  VerifyStroke("SKPEUFRBD", "and I had asked");
  VerifyStroke("SKPE", "and he");
  VerifyStroke("SKPEF", "and he has");
  VerifyStroke("SKPEFD", "and he had");
  VerifyStroke("SKPERB", "and he asks");
  VerifyStroke("SKPERBD", "and he asked");
  VerifyStroke("SKPEFRB", "and he has asked");
  VerifyStroke("SKPEFRBD", "and he had asked");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Middle tests") {
  // spellchecker: disable
  VerifyStroke("SWRA", "I can");
  VerifyStroke("KPWRA", "you can");
  VerifyStroke("KWHRA", "he can");
  VerifyStroke("SWRO", "I shall");
  VerifyStroke("KPWRO", "you shall");
  VerifyStroke("KWHRO", "he shall");
  VerifyStroke("SWRAO", "I will");
  VerifyStroke("KPWRAO", "you will");
  VerifyStroke("KWHRAO", "he will");
  VerifyStroke("SWR*", "I don't");
  VerifyStroke("KPWR*", "you don't");
  VerifyStroke("KWHR*", "he doesn't");
  VerifyStroke("SWRA*", "I can't");
  VerifyStroke("KPWRA*", "you can't");
  VerifyStroke("KWHRA*", "he can't");
  VerifyStroke("SWRO*", "I shall not");
  VerifyStroke("KPWRO*", "you shall not");
  VerifyStroke("KWHRO*", "he shall not");
  VerifyStroke("SWRAO*", "I won't");
  VerifyStroke("KPWRAO*", "you won't");
  VerifyStroke("KWHRAO*", "he won't");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Swap middle tests") {
  // spellchecker: disable
  VerifyStroke("SWRAU", " can I");
  VerifyStroke("KPWRAU", " can you");
  VerifyStroke("KWHRAU", " can he");
  VerifyStroke("SWROU", " shall I");
  VerifyStroke("KPWROU", " shall you");
  VerifyStroke("KWHROU", " shall he");
  VerifyStroke("SWRAOU", " will I");
  VerifyStroke("KPWRAOU", " will you");
  VerifyStroke("KWHRAOU", " will he");
  VerifyStroke("SWR*U", " don't I");
  VerifyStroke("KPWR*U", " don't you");
  VerifyStroke("KWHR*U", " doesn't he");
  VerifyStroke("SWRA*U", " can't I");
  VerifyStroke("KPWRA*U", " can't you");
  VerifyStroke("KWHRA*U", " can't he");
  VerifyStroke("SWRO*U", " shall not I");
  VerifyStroke("KPWRO*U", " shall not you");
  VerifyStroke("KWHRO*U", " shall not he");
  VerifyStroke("SWRAO*U", " won't I");
  VerifyStroke("KPWRAO*U", " won't you");
  VerifyStroke("KWHRAO*U", " won't he");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Basic structures tests") {
  // spellchecker: disable
  VerifyStroke("SWREURB", "I still ask");
  VerifyStroke("SWRAEURB", "I can still ask");
  VerifyStroke("SWRA*EURB", "I still can't ask");

  VerifyStroke("SWRUFRB", "I just ask");
  VerifyStroke("SWRAUFRB", "I can just ask");
  VerifyStroke("SWRA*UFRB", "I just can't ask");

  VerifyStroke("SWREUFRB", "I never ask");
  VerifyStroke("SWRAEUFRB", "I can never ask");
  VerifyStroke("SWRA*EUFRB", "I can't even ask");

  VerifyStroke("SWRA-G", "I can go");
  VerifyStroke("SWRA*G", "I can't go");
  VerifyStroke("SWRA-EG", "I can be going");
  VerifyStroke("SWRA*EG", "I can't be going");
  VerifyStroke("SWRA-UG", " can I go");
  VerifyStroke("SWRA*UG", " can't I go");
  VerifyStroke("SWRA-EUG", "I can still go");
  VerifyStroke("SWRA*EUG", "I still can't go");

  VerifyStroke("SWRA-FG", "I can have gone");
  VerifyStroke("SWRA*FG", "I can't have gone");
  VerifyStroke("SWRA-EFG", "I can have been going");
  VerifyStroke("SWRA*EFG", "I can't have been going");
  VerifyStroke("SWRA-UFG", "I can just go");
  VerifyStroke("SWRA*UFG", "I just can't go");
  VerifyStroke("SWRA-EUFG", "I can never go");
  VerifyStroke("SWRA*EUFG", "I can't even go");

  VerifyStroke("SWRA-GD", "I could go");
  VerifyStroke("SWRA*GD", "I couldn't go");
  VerifyStroke("SWRA-EGD", "I could be going");
  VerifyStroke("SWRA*EGD", "I couldn't be going");
  VerifyStroke("SWRA-UGD", " could I go");
  VerifyStroke("SWRA*UGD", " couldn't I go");
  VerifyStroke("SWRA-EUGD", "I could still go");
  VerifyStroke("SWRA*EUGD", "I still couldn't go");

  VerifyStroke("SWRA-FGD", "I could have gone");
  VerifyStroke("SWRA*FGD", "I couldn't have gone");
  VerifyStroke("SWRA-EFGD", "I could have been going");
  VerifyStroke("SWRA*EFGD", "I couldn't have been going");
  VerifyStroke("SWRA-UFGD", "I could just go");
  VerifyStroke("SWRA*UFGD", "I just couldn't go");
  VerifyStroke("SWRA-EUFGD", "I could never go");
  VerifyStroke("SWRA*EUFGD", "I couldn't even go");

  VerifyStroke("SWR-G", "I go");
  VerifyStroke("SWR*G", "I don't go");
  VerifyStroke("SWR-EG", "I am going");
  VerifyStroke("SWR*EG", "I am not going");
  VerifyStroke("SWR-UG", " do I go");
  VerifyStroke("SWR*UG", " don't I go");
  VerifyStroke("SWR-EUG", "I still go");
  VerifyStroke("SWR*EUG", "I still don't go");

  VerifyStroke("SWR-FG", "I have gone");
  VerifyStroke("SWR*FG", "I haven't gone");
  VerifyStroke("SWR-EFG", "I have been going");
  VerifyStroke("SWR*EFG", "I haven't been going");
  VerifyStroke("SWR-UFG", "I just go");
  VerifyStroke("SWR*UFG", "I just don't go");
  VerifyStroke("SWR-EUFG", "I never go");
  VerifyStroke("SWR*EUFG", "I don't even go");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Non-conjugating enders tests") {
  // spellchecker: disable
  VerifyStroke("SWR-BGS", "I can");
  VerifyStroke("SWR-BGSZ", "I could");

  VerifyStroke("SWR-RLG", "I really");
  VerifyStroke("SWR-RLGD", "I really");

  VerifyStroke("SWR-PL", "I may");
  VerifyStroke("SWR-PLD", "I might");
  VerifyStroke("SWR-PLT", "I may be");
  VerifyStroke("SWR-PLTD", "I might be");

  VerifyStroke("SWR-PBLGS", "I must");
  VerifyStroke("SWR-PBLGTS", "I must be");

  VerifyStroke("SWR-RBL", "I shall");
  VerifyStroke("SWR-RBLD", "I should");

  VerifyStroke("SWR-TZ", "I used to");
  VerifyStroke("SWR-TDZ", "I used to");

  VerifyStroke("SWR-RBGS", "I will");
  VerifyStroke("SWR-RBGSZ", "I would");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Enders tests") {
  // spellchecker: disable
  VerifyStroke("SWR-RB", "I ask");
  VerifyStroke("SWR-ERB", "I am asking");
  VerifyStroke("SWR-FRB", "I have asked");
  VerifyStroke("KWHR-RB", "he asks");
  VerifyStroke("SWR-RBD", "I asked");
  VerifyStroke("SWR-ERBD", "I was asking");
  VerifyStroke("SWR-FRBD", "I had asked");
  VerifyStroke("KWHR-RBD", "he asked");

  VerifyStroke("SWR-B", "I am");
  VerifyStroke("SWR-EB", "I am being");
  VerifyStroke("SWR-FB", "I have been");
  VerifyStroke("KWHR-B", "he is");
  VerifyStroke("TWH-B", "they are");
  VerifyStroke("SWR-BD", "I was");
  VerifyStroke("SWR-EBD", "I was being");
  VerifyStroke("SWR-FBD", "I had been");
  VerifyStroke("KWHR-BD", "he was");
  VerifyStroke("TWH-BD", "they were");

  VerifyStroke("SWR-BT", "I am a");
  VerifyStroke("SWR-EBT", "I am being a");
  VerifyStroke("SWR-FBT", "I have been a");
  VerifyStroke("KWHR-BT", "he is a");
  VerifyStroke("TWH-BT", "they are a");
  VerifyStroke("SWR-BTD", "I was a");
  VerifyStroke("SWR-EBTD", "I was being a");
  VerifyStroke("SWR-FBTD", "I had been a");
  VerifyStroke("KWHR-BTD", "he was a");
  VerifyStroke("TWH-BTD", "they were a");

  VerifyStroke("SWR-RPBG", "I become");
  VerifyStroke("SWR-ERPBG", "I am becoming");
  VerifyStroke("SWR-FRPBG", "I have become");
  VerifyStroke("KWHR-RPBG", "he becomes");
  VerifyStroke("SWR-RPBGD", "I became");
  VerifyStroke("SWR-ERPBGD", "I was becoming");
  VerifyStroke("SWR-FRPBGD", "I had become");
  VerifyStroke("KWHR-RPBGD", "he became");

  VerifyStroke("SWR-RPBGT", "I become a");
  VerifyStroke("SWR-ERPBGT", "I am becoming a");
  VerifyStroke("SWR-FRPBGT", "I have become a");
  VerifyStroke("KWHR-RPBGT", "he becomes a");
  VerifyStroke("SWR-RPBGTD", "I became a");
  VerifyStroke("SWR-ERPBGTD", "I was becoming a");
  VerifyStroke("SWR-FRPBGTD", "I had become a");
  VerifyStroke("KWHR-RPBGTD", "he became a");

  VerifyStroke("SWR-BL", "I believe");
  VerifyStroke("SWR-EBL", "I am believing");
  VerifyStroke("SWR-FBL", "I have believed");
  VerifyStroke("KWHR-BL", "he believes");
  VerifyStroke("SWR-BLD", "I believed");
  VerifyStroke("SWR-EBLD", "I was believing");
  VerifyStroke("SWR-FBLD", "I had believed");
  VerifyStroke("KWHR-BLD", "he believed");

  VerifyStroke("SWR-BLT", "I believe that");
  VerifyStroke("SWR-EBLT", "I am believing that");
  VerifyStroke("SWR-FBLT", "I have believed that");
  VerifyStroke("KWHR-BLT", "he believes that");
  VerifyStroke("SWR-BLTD", "I believed that");
  VerifyStroke("SWR-EBLTD", "I was believing that");
  VerifyStroke("SWR-FBLTD", "I had believed that");
  VerifyStroke("KWHR-BLTD", "he believed that");

  VerifyStroke("SWR-RBLG", "I call");
  VerifyStroke("SWR-ERBLG", "I am calling");
  VerifyStroke("SWR-FRBLG", "I have called");
  VerifyStroke("KWHR-RBLG", "he calls");
  VerifyStroke("SWR-RBLGD", "I called");
  VerifyStroke("SWR-ERBLGD", "I was calling");
  VerifyStroke("SWR-FRBLGD", "I had called");
  VerifyStroke("KWHR-RBLGD", "he called");

  VerifyStroke("SWR-RZ", "I care");
  VerifyStroke("SWR-ERZ", "I am caring");
  VerifyStroke("SWR-FRZ", "I have cared");
  VerifyStroke("KWHR-RZ", "he cares");
  VerifyStroke("SWR-RZD", "I cared");
  VerifyStroke("SWR-ERZD", "I was caring");
  VerifyStroke("SWR-FRZD", "I had cared");
  VerifyStroke("KWHR-RZD", "he cared");

  VerifyStroke("SWR-PBGZ", "I change");
  VerifyStroke("SWR-EPBGZ", "I am changing");
  VerifyStroke("SWR-FPBGZ", "I have changed");
  VerifyStroke("KWHR-PBGZ", "he changes");
  VerifyStroke("SWR-PBGDZ", "I changed");
  VerifyStroke("SWR-EPBGDZ", "I was changing");
  VerifyStroke("SWR-FPBGDZ", "I had changed");
  VerifyStroke("KWHR-PBGDZ", "he changed");

  VerifyStroke("SWR-BG", "I come");
  VerifyStroke("SWR-EBG", "I am coming");
  VerifyStroke("SWR-FBG", "I have come");
  VerifyStroke("KWHR-BG", "he comes");
  VerifyStroke("SWR-BGD", "I came");
  VerifyStroke("SWR-EBGD", "I was coming");
  VerifyStroke("SWR-FBGD", "I had come");
  VerifyStroke("KWHR-BGD", "he came");

  VerifyStroke("SWR-BGT", "I come to");
  VerifyStroke("SWR-EBGT", "I am coming to");
  VerifyStroke("SWR-FBGT", "I have come to");
  VerifyStroke("KWHR-BGT", "he comes to");
  VerifyStroke("SWR-BGTD", "I came to");
  VerifyStroke("SWR-EBGTD", "I was coming to");
  VerifyStroke("SWR-FBGTD", "I had come to");
  VerifyStroke("KWHR-BGTD", "he came to");

  VerifyStroke("SWR-RBGZ", "I consider");
  VerifyStroke("SWR-ERBGZ", "I am considering");
  VerifyStroke("SWR-FRBGZ", "I have considered");
  VerifyStroke("KWHR-RBGZ", "he considers");
  VerifyStroke("SWR-RBGDZ", "I considered");
  VerifyStroke("SWR-ERBGDZ", "I was considering");
  VerifyStroke("SWR-FRBGDZ", "I had considered");
  VerifyStroke("KWHR-RBGDZ", "he considered");

  VerifyStroke("SWR-RP", "I do");
  VerifyStroke("SWR-ERP", "I am doing");
  VerifyStroke("SWR-FRP", "I have done");
  VerifyStroke("KWHR-RP", "he does");
  VerifyStroke("SWR-RPD", "I did");
  VerifyStroke("SWR-ERPD", "I was doing");
  VerifyStroke("SWR-FRPD", "I had done");
  VerifyStroke("KWHR-RPD", "he did");

  VerifyStroke("SWR-RPT", "I do it");
  VerifyStroke("SWR-ERPT", "I am doing it");
  VerifyStroke("SWR-FRPT", "I have done it");
  VerifyStroke("KWHR-RPT", "he does it");
  VerifyStroke("SWR-RPTD", "I did it");
  VerifyStroke("SWR-ERPTD", "I was doing it");
  VerifyStroke("SWR-FRPTD", "I had done it");
  VerifyStroke("KWHR-RPTD", "he did it");

  VerifyStroke("SWR-PGS", "I expect");
  VerifyStroke("SWR-EPGS", "I am expecting");
  VerifyStroke("SWR-FPGS", "I have expected");
  VerifyStroke("KWHR-PGS", "he expects");
  VerifyStroke("SWR-PGSZ", "I expected");
  VerifyStroke("SWR-EPGSZ", "I was expecting");
  VerifyStroke("SWR-FPGSZ", "I had expected");
  VerifyStroke("KWHR-PGSZ", "he expected");

  VerifyStroke("SWR-PGTS", "I expect that");
  VerifyStroke("SWR-EPGTS", "I am expecting that");
  VerifyStroke("SWR-FPGTS", "I have expected that");
  VerifyStroke("KWHR-PGTS", "he expects that");
  VerifyStroke("SWR-PGTSDZ", "I expected that");
  VerifyStroke("SWR-EPGTSDZ", "I was expecting that");
  VerifyStroke("SWR-FPGTSDZ", "I had expected that");
  VerifyStroke("KWHR-PGTSDZ", "he expected that");

  VerifyStroke("SWR-LT", "I feel");
  VerifyStroke("SWR-ELT", "I am feeling");
  VerifyStroke("SWR-FLT", "I have felt");
  VerifyStroke("KWHR-LT", "he feels");
  VerifyStroke("SWR-LTD", "I felt");
  VerifyStroke("SWR-ELTD", "I was feeling");
  VerifyStroke("SWR-FLTD", "I had felt");
  VerifyStroke("KWHR-LTD", "he felt");

  VerifyStroke("SWR-LTS", "I feel like");
  VerifyStroke("SWR-ELTS", "I am feeling like");
  VerifyStroke("SWR-FLTS", "I have felt like");
  VerifyStroke("KWHR-LTS", "he feels like");
  VerifyStroke("SWR-LTSDZ", "I felt like");
  VerifyStroke("SWR-ELTSDZ", "I was feeling like");
  VerifyStroke("SWR-FLTSDZ", "I had felt like");
  VerifyStroke("KWHR-LTSDZ", "he felt like");

  VerifyStroke("SWR-PBLG", "I find");
  VerifyStroke("SWR-EPBLG", "I am finding");
  VerifyStroke("SWR-FPBLG", "I have found");
  VerifyStroke("KWHR-PBLG", "he finds");
  VerifyStroke("SWR-PBLGD", "I found");
  VerifyStroke("SWR-EPBLGD", "I was finding");
  VerifyStroke("SWR-FPBLGD", "I had found");
  VerifyStroke("KWHR-PBLGD", "he found");

  VerifyStroke("SWR-PBLGT", "I find that");
  VerifyStroke("SWR-EPBLGT", "I am finding that");
  VerifyStroke("SWR-FPBLGT", "I have found that");
  VerifyStroke("KWHR-PBLGT", "he finds that");
  VerifyStroke("SWR-PBLGTD", "I found that");
  VerifyStroke("SWR-EPBLGTD", "I was finding that");
  VerifyStroke("SWR-FPBLGTD", "I had found that");
  VerifyStroke("KWHR-PBLGTD", "he found that");

  VerifyStroke("SWR-RG", "I forget");
  VerifyStroke("SWR-ERG", "I am forgetting");
  VerifyStroke("SWR-FRG", "I have forgotten");
  VerifyStroke("KWHR-RG", "he forgets");
  VerifyStroke("SWR-RGD", "I forgot");
  VerifyStroke("SWR-ERGD", "I was forgetting");
  VerifyStroke("SWR-FRGD", "I had forgotten");
  VerifyStroke("KWHR-RGD", "he forgot");

  VerifyStroke("SWR-RGT", "I forget to");
  VerifyStroke("SWR-ERGT", "I am forgetting to");
  VerifyStroke("SWR-FRGT", "I have forgotten to");
  VerifyStroke("KWHR-RGT", "he forgets to");
  VerifyStroke("SWR-RGTD", "I forgot to");
  VerifyStroke("SWR-ERGTD", "I was forgetting to");
  VerifyStroke("SWR-FRGTD", "I had forgotten to");
  VerifyStroke("KWHR-RGTD", "he forgot to");

  VerifyStroke("SWR-GS", "I get");
  VerifyStroke("SWR-EGS", "I am getting");
  VerifyStroke("SWR-FGS", "I have gotten");
  VerifyStroke("KWHR-GS", "he gets");
  VerifyStroke("SWR-GSZ", "I got");
  VerifyStroke("SWR-EGSZ", "I was getting");
  VerifyStroke("SWR-FGSZ", "I had gotten");
  VerifyStroke("KWHR-GSZ", "he got");

  VerifyStroke("SWR-GTS", "I get to");
  VerifyStroke("SWR-EGTS", "I am getting to");
  VerifyStroke("SWR-FGTS", "I have gotten to");
  VerifyStroke("KWHR-GTS", "he gets to");
  VerifyStroke("SWR-GTSDZ", "I got to");
  VerifyStroke("SWR-EGTSDZ", "I was getting to");
  VerifyStroke("SWR-FGTSDZ", "I had gotten to");
  VerifyStroke("KWHR-GTSDZ", "he got to");

  VerifyStroke("SWR-GZ", "I give");
  VerifyStroke("SWR-EGZ", "I am giving");
  VerifyStroke("SWR-FGZ", "I have given");
  VerifyStroke("KWHR-GZ", "he gives");
  VerifyStroke("SWR-GDZ", "I gave");
  VerifyStroke("SWR-EGDZ", "I was giving");
  VerifyStroke("SWR-FGDZ", "I had given");
  VerifyStroke("KWHR-GDZ", "he gave");

  VerifyStroke("SWR-G", "I go");
  VerifyStroke("SWR-EG", "I am going");
  VerifyStroke("SWR-FG", "I have gone");
  VerifyStroke("KWHR-G", "he goes");
  VerifyStroke("SWR-GD", "I went");
  VerifyStroke("SWR-EGD", "I was going");
  VerifyStroke("SWR-FGD", "I had gone");
  VerifyStroke("KWHR-GD", "he went");

  VerifyStroke("SWR-GT", "I go to");
  VerifyStroke("SWR-EGT", "I am going to");
  VerifyStroke("SWR-FGT", "I have gone to");
  VerifyStroke("KWHR-GT", "he goes to");
  VerifyStroke("SWR-GTD", "I went to");
  VerifyStroke("SWR-EGTD", "I was going to");
  VerifyStroke("SWR-FGTD", "I had gone to");
  VerifyStroke("KWHR-GTD", "he went to");

  VerifyStroke("SWR-T", "I have");
  VerifyStroke("SWR-ET", "I am having");
  VerifyStroke("SWR-FT", "I have had");
  VerifyStroke("KWHR-T", "he has");
  VerifyStroke("SWR-TD", "I had");
  VerifyStroke("SWR-ETD", "I was having");
  VerifyStroke("SWR-FTD", "I had had");
  VerifyStroke("KWHR-TD", "he had");

  VerifyStroke("SWR-TS", "I have to");
  VerifyStroke("SWR-ETS", "I am having to");
  VerifyStroke("SWR-FTS", "I have had to");
  VerifyStroke("KWHR-TS", "he has to");
  VerifyStroke("SWR-TSDZ", "I had to");
  VerifyStroke("SWR-ETSDZ", "I was having to");
  VerifyStroke("SWR-FTSDZ", "I had had to");
  VerifyStroke("KWHR-TSDZ", "he had to");

  VerifyStroke("SWR-PZ", "I happen");
  VerifyStroke("SWR-EPZ", "I am happening");
  VerifyStroke("SWR-FPZ", "I have happened");
  VerifyStroke("KWHR-PZ", "he happens");
  VerifyStroke("SWR-PDZ", "I happened");
  VerifyStroke("SWR-EPDZ", "I was happening");
  VerifyStroke("SWR-FPDZ", "I had happened");
  VerifyStroke("KWHR-PDZ", "he happened");

  VerifyStroke("SWR-PG", "I hear");
  VerifyStroke("SWR-EPG", "I am hearing");
  VerifyStroke("SWR-FPG", "I have heard");
  VerifyStroke("KWHR-PG", "he hears");
  VerifyStroke("SWR-PGD", "I heard");
  VerifyStroke("SWR-EPGD", "I was hearing");
  VerifyStroke("SWR-FPGD", "I had heard");
  VerifyStroke("KWHR-PGD", "he heard");

  VerifyStroke("SWR-PGT", "I hear that");
  VerifyStroke("SWR-EPGT", "I am hearing that");
  VerifyStroke("SWR-FPGT", "I have heard that");
  VerifyStroke("KWHR-PGT", "he hears that");
  VerifyStroke("SWR-PGTD", "I heard that");
  VerifyStroke("SWR-EPGTD", "I was hearing that");
  VerifyStroke("SWR-FPGTD", "I had heard that");
  VerifyStroke("KWHR-PGTD", "he heard that");

  VerifyStroke("SWR-RPS", "I hope");
  VerifyStroke("SWR-ERPS", "I am hoping");
  VerifyStroke("SWR-FRPS", "I have hoped");
  VerifyStroke("KWHR-RPS", "he hopes");
  VerifyStroke("SWR-RPSZ", "I hoped");
  VerifyStroke("SWR-ERPSZ", "I was hoping");
  VerifyStroke("SWR-FRPSZ", "I had hoped");
  VerifyStroke("KWHR-RPSZ", "he hoped");

  VerifyStroke("SWR-RPTS", "I hope to");
  VerifyStroke("SWR-ERPTS", "I am hoping to");
  VerifyStroke("SWR-FRPTS", "I have hoped to");
  VerifyStroke("KWHR-RPTS", "he hopes to");
  VerifyStroke("SWR-RPTSDZ", "I hoped to");
  VerifyStroke("SWR-ERPTSDZ", "I was hoping to");
  VerifyStroke("SWR-FRPTSDZ", "I had hoped to");
  VerifyStroke("KWHR-RPTSDZ", "he hoped to");

  VerifyStroke("SWR-PLG", "I imagine");
  VerifyStroke("SWR-EPLG", "I am imagining");
  VerifyStroke("SWR-FPLG", "I have imagined");
  VerifyStroke("KWHR-PLG", "he imagines");
  VerifyStroke("SWR-PLGD", "I imagined");
  VerifyStroke("SWR-EPLGD", "I was imagining");
  VerifyStroke("SWR-FPLGD", "I had imagined");
  VerifyStroke("KWHR-PLGD", "he imagined");

  VerifyStroke("SWR-PLGT", "I imagine that");
  VerifyStroke("SWR-EPLGT", "I am imagining that");
  VerifyStroke("SWR-FPLGT", "I have imagined that");
  VerifyStroke("KWHR-PLGT", "he imagines that");
  VerifyStroke("SWR-PLGTD", "I imagined that");
  VerifyStroke("SWR-EPLGTD", "I was imagining that");
  VerifyStroke("SWR-FPLGTD", "I had imagined that");
  VerifyStroke("KWHR-PLGTD", "he imagined that");

  VerifyStroke("SWR-PBGS", "I keep");
  VerifyStroke("SWR-EPBGS", "I am keeping");
  VerifyStroke("SWR-FPBGS", "I have kept");
  VerifyStroke("KWHR-PBGS", "he keeps");
  VerifyStroke("SWR-PBGSZ", "I kept");
  VerifyStroke("SWR-EPBGSZ", "I was keeping");
  VerifyStroke("SWR-FPBGSZ", "I had kept");
  VerifyStroke("KWHR-PBGSZ", "he kept");

  VerifyStroke("SWR-PB", "I know");
  VerifyStroke("SWR-EPB", "I am knowing");
  VerifyStroke("SWR-FPB", "I have known");
  VerifyStroke("KWHR-PB", "he knows");
  VerifyStroke("SWR-PBD", "I knew");
  VerifyStroke("SWR-EPBD", "I was knowing");
  VerifyStroke("SWR-FPBD", "I had known");
  VerifyStroke("KWHR-PBD", "he knew");

  VerifyStroke("SWR-PBT", "I know that");
  VerifyStroke("SWR-EPBT", "I am knowing that");
  VerifyStroke("SWR-FPBT", "I have known that");
  VerifyStroke("KWHR-PBT", "he knows that");
  VerifyStroke("SWR-PBTD", "I knew that");
  VerifyStroke("SWR-EPBTD", "I was knowing that");
  VerifyStroke("SWR-FPBTD", "I had known that");
  VerifyStroke("KWHR-PBTD", "he knew that");

  VerifyStroke("SWR-RPBS", "I learn");
  VerifyStroke("SWR-ERPBS", "I am learning");
  VerifyStroke("SWR-FRPBS", "I have learned");
  VerifyStroke("KWHR-RPBS", "he learns");
  VerifyStroke("SWR-RPBSZ", "I learned");
  VerifyStroke("SWR-ERPBSZ", "I was learning");
  VerifyStroke("SWR-FRPBSZ", "I had learned");
  VerifyStroke("KWHR-RPBSZ", "he learned");

  VerifyStroke("SWR-RPBTS", "I learn to");
  VerifyStroke("SWR-ERPBTS", "I am learning to");
  VerifyStroke("SWR-FRPBTS", "I have learned to");
  VerifyStroke("KWHR-RPBTS", "he learns to");
  VerifyStroke("SWR-RPBTSDZ", "I learned to");
  VerifyStroke("SWR-ERPBTSDZ", "I was learning to");
  VerifyStroke("SWR-FRPBTSDZ", "I had learned to");
  VerifyStroke("KWHR-RPBTSDZ", "he learned to");

  VerifyStroke("SWR-LGZ", "I leave");
  VerifyStroke("SWR-ELGZ", "I am leaving");
  VerifyStroke("SWR-FLGZ", "I have left");
  VerifyStroke("KWHR-LGZ", "he leaves");
  VerifyStroke("SWR-LGDZ", "I left");
  VerifyStroke("SWR-ELGDZ", "I was leaving");
  VerifyStroke("SWR-FLGDZ", "I had left");
  VerifyStroke("KWHR-LGDZ", "he left");

  VerifyStroke("SWR-LS", "I let");
  VerifyStroke("SWR-ELS", "I am letting");
  VerifyStroke("SWR-FLS", "I have let");
  VerifyStroke("KWHR-LS", "he lets");
  VerifyStroke("SWR-LSZ", "I let");
  VerifyStroke("SWR-ELSZ", "I was letting");
  VerifyStroke("SWR-FLSZ", "I had let");
  VerifyStroke("KWHR-LSZ", "he let");

  VerifyStroke("SWR-BLG", "I like");
  VerifyStroke("SWR-EBLG", "I am liking");
  VerifyStroke("SWR-FBLG", "I have liked");
  VerifyStroke("KWHR-BLG", "he likes");
  VerifyStroke("SWR-BLGD", "I liked");
  VerifyStroke("SWR-EBLGD", "I was liking");
  VerifyStroke("SWR-FBLGD", "I had liked");
  VerifyStroke("KWHR-BLGD", "he liked");

  VerifyStroke("SWR-LZ", "I live");
  VerifyStroke("SWR-ELZ", "I am living");
  VerifyStroke("SWR-FLZ", "I have lived");
  VerifyStroke("KWHR-LZ", "he lives");
  VerifyStroke("SWR-LDZ", "I lived");
  VerifyStroke("SWR-ELDZ", "I was living");
  VerifyStroke("SWR-FLDZ", "I had lived");
  VerifyStroke("KWHR-LDZ", "he lived");

  VerifyStroke("SWR-L", "I look");
  VerifyStroke("SWR-EL", "I am looking");
  VerifyStroke("SWR-FL", "I have looked");
  VerifyStroke("KWHR-L", "he looks");
  VerifyStroke("SWR-LD", "I looked");
  VerifyStroke("SWR-ELD", "I was looking");
  VerifyStroke("SWR-FLD", "I had looked");
  VerifyStroke("KWHR-LD", "he looked");

  VerifyStroke("SWR-LG", "I love");
  VerifyStroke("SWR-ELG", "I am loving");
  VerifyStroke("SWR-FLG", "I have loved");
  VerifyStroke("KWHR-LG", "he loves");
  VerifyStroke("SWR-LGD", "I loved");
  VerifyStroke("SWR-ELGD", "I was loving");
  VerifyStroke("SWR-FLGD", "I had loved");
  VerifyStroke("KWHR-LGD", "he loved");

  VerifyStroke("SWR-LGT", "I love to");
  VerifyStroke("SWR-ELGT", "I am loving to");
  VerifyStroke("SWR-FLGT", "I have loved to");
  VerifyStroke("KWHR-LGT", "he loves to");
  VerifyStroke("SWR-LGTD", "I loved to");
  VerifyStroke("SWR-ELGTD", "I was loving to");
  VerifyStroke("SWR-FLGTD", "I had loved to");
  VerifyStroke("KWHR-LGTD", "he loved to");

  VerifyStroke("SWR-RPBL", "I make");
  VerifyStroke("SWR-ERPBL", "I am making");
  VerifyStroke("SWR-FRPBL", "I have made");
  VerifyStroke("KWHR-RPBL", "he makes");
  VerifyStroke("SWR-RPBLD", "I made");
  VerifyStroke("SWR-ERPBLD", "I was making");
  VerifyStroke("SWR-FRPBLD", "I had made");
  VerifyStroke("KWHR-RPBLD", "he made");

  VerifyStroke("SWR-RPBLT", "I make a");
  VerifyStroke("SWR-ERPBLT", "I am making a");
  VerifyStroke("SWR-FRPBLT", "I have made a");
  VerifyStroke("KWHR-RPBLT", "he makes a");
  VerifyStroke("SWR-RPBLTD", "I made a");
  VerifyStroke("SWR-ERPBLTD", "I was making a");
  VerifyStroke("SWR-FRPBLTD", "I had made a");
  VerifyStroke("KWHR-RPBLTD", "he made a");

  VerifyStroke("SWR-PBL", "I mean");
  VerifyStroke("SWR-EPBL", "I am meaning");
  VerifyStroke("SWR-FPBL", "I have meant");
  VerifyStroke("KWHR-PBL", "he means");
  VerifyStroke("SWR-PBLD", "I meant");
  VerifyStroke("SWR-EPBLD", "I was meaning");
  VerifyStroke("SWR-FPBLD", "I had meant");
  VerifyStroke("KWHR-PBLD", "he meant");

  VerifyStroke("SWR-PBLT", "I mean to");
  VerifyStroke("SWR-EPBLT", "I am meaning to");
  VerifyStroke("SWR-FPBLT", "I have meant to");
  VerifyStroke("KWHR-PBLT", "he means to");
  VerifyStroke("SWR-PBLTD", "I meant to");
  VerifyStroke("SWR-EPBLTD", "I was meaning to");
  VerifyStroke("SWR-FPBLTD", "I had meant to");
  VerifyStroke("KWHR-PBLTD", "he meant to");

  VerifyStroke("SWR-PBLS", "I mind");
  VerifyStroke("SWREPBLS", "I am minding");
  VerifyStroke("SWR-FPBLS", "I have minded");
  VerifyStroke("KWHR-PBLS", "he minds");
  VerifyStroke("SWR-PBLSZ", "I minded");
  VerifyStroke("SWREPBLSZ", "I was minding");
  VerifyStroke("SWR-FPBLSZ", "I had minded");
  VerifyStroke("KWHR-PBLSZ", "he minded");

  VerifyStroke("SWR-PLZ", "I move");
  VerifyStroke("SWR-EPLZ", "I am moving");
  VerifyStroke("SWR-FPLZ", "I have moved");
  VerifyStroke("KWHR-PLZ", "he moves");
  VerifyStroke("SWR-PLDZ", "I moved");
  VerifyStroke("SWR-EPLDZ", "I was moving");
  VerifyStroke("SWR-FPLDZ", "I had moved");
  VerifyStroke("KWHR-PLDZ", "he moved");

  VerifyStroke("SWR-RPG", "I need");
  VerifyStroke("SWR-ERPG", "I am needing");
  VerifyStroke("SWR-FRPG", "I have needed");
  VerifyStroke("KWHR-RPG", "he needs");
  VerifyStroke("SWR-RPGD", "I needed");
  VerifyStroke("SWR-ERPGD", "I was needing");
  VerifyStroke("SWR-FRPGD", "I had needed");
  VerifyStroke("KWHR-RPGD", "he needed");

  VerifyStroke("SWR-RPGT", "I need to");
  VerifyStroke("SWR-ERPGT", "I am needing to");
  VerifyStroke("SWR-FRPGT", "I have needed to");
  VerifyStroke("KWHR-RPGT", "he needs to");
  VerifyStroke("SWR-RPGTD", "I needed to");
  VerifyStroke("SWR-ERPGTD", "I was needing to");
  VerifyStroke("SWR-FRPGTD", "I had needed to");
  VerifyStroke("KWHR-RPGTD", "he needed to");

  VerifyStroke("SWR-PS", "I put");
  VerifyStroke("SWR-EPS", "I am putting");
  VerifyStroke("SWR-FPS", "I have put");
  VerifyStroke("KWHR-PS", "he puts");
  VerifyStroke("SWR-PSZ", "I put");
  VerifyStroke("SWR-EPSZ", "I was putting");
  VerifyStroke("SWR-FPSZ", "I had put");
  VerifyStroke("KWHR-PSZ", "he put");

  VerifyStroke("SWR-PST", "I put it");
  VerifyStroke("SWR-EPST", "I am putting it");
  VerifyStroke("SWR-FPST", "I have put it");
  VerifyStroke("KWHR-PST", "he puts it");
  VerifyStroke("SWR-PSTDZ", "I put it");
  VerifyStroke("SWR-EPSTDZ", "I was putting it");
  VerifyStroke("SWR-FPSTDZ", "I had put it");
  VerifyStroke("KWHR-PSTDZ", "he put it");

  VerifyStroke("SWR-RS", "I read");
  VerifyStroke("SWR-ERS", "I am reading");
  VerifyStroke("SWR-FRS", "I have read");
  VerifyStroke("KWHR-RS", "he reads");
  VerifyStroke("SWR-RSZ", "I read");
  VerifyStroke("SWR-ERSZ", "I was reading");
  VerifyStroke("SWR-FRSZ", "I had read");
  VerifyStroke("KWHR-RSZ", "he read");

  VerifyStroke("SWR-RL", "I recall");
  VerifyStroke("SWR-ERL", "I am recalling");
  VerifyStroke("SWR-FRL", "I have recalled");
  VerifyStroke("KWHR-RL", "he recalls");
  VerifyStroke("SWR-RLD", "I recalled");
  VerifyStroke("SWR-ERLD", "I was recalling");
  VerifyStroke("SWR-FRLD", "I had recalled");
  VerifyStroke("KWHR-RLD", "he recalled");

  VerifyStroke("SWR-RLS", "I realize");
  VerifyStroke("SWR-ERLS", "I am realizing");
  VerifyStroke("SWR-FRLS", "I have realized");
  VerifyStroke("KWHR-RLS", "he realizes");
  VerifyStroke("SWR-RLSZ", "I realized");
  VerifyStroke("SWR-ERLSZ", "I was realizing");
  VerifyStroke("SWR-FRLSZ", "I had realized");
  VerifyStroke("KWHR-RLSZ", "he realized");

  VerifyStroke("SWR-RLTS", "I realize that");
  VerifyStroke("SWR-ERLTS", "I am realizing that");
  VerifyStroke("SWR-FRLTS", "I have realized that");
  VerifyStroke("KWHR-RLTS", "he realizes that");
  VerifyStroke("SWR-RLTSDZ", "I realized that");
  VerifyStroke("SWR-ERLTSDZ", "I was realizing that");
  VerifyStroke("SWR-FRLTSDZ", "I had realized that");
  VerifyStroke("KWHR-RLTSDZ", "he realized that");

  VerifyStroke("SWR-RPL", "I remember");
  VerifyStroke("SWR-ERPL", "I am remembering");
  VerifyStroke("SWR-FRPL", "I have remembered");
  VerifyStroke("KWHR-RPL", "he remembers");
  VerifyStroke("SWR-RPLD", "I remembered");
  VerifyStroke("SWR-ERPLD", "I was remembering");
  VerifyStroke("SWR-FRPLD", "I had remembered");
  VerifyStroke("KWHR-RPLD", "he remembered");

  VerifyStroke("SWR-RPLT", "I remember that");
  VerifyStroke("SWR-ERPLT", "I am remembering that");
  VerifyStroke("SWR-FRPLT", "I have remembered that");
  VerifyStroke("KWHR-RPLT", "he remembers that");
  VerifyStroke("SWR-RPLTD", "I remembered that");
  VerifyStroke("SWR-ERPLTD", "I was remembering that");
  VerifyStroke("SWR-FRPLTD", "I had remembered that");
  VerifyStroke("KWHR-RPLTD", "he remembered that");

  VerifyStroke("SWR-RPLS", "I remain");
  VerifyStroke("SWR-ERPLS", "I am remaining");
  VerifyStroke("SWR-FRPLS", "I have remained");
  VerifyStroke("KWHR-RPLS", "he remains");
  VerifyStroke("SWR-RPLSZ", "I remained");
  VerifyStroke("SWR-ERPLSZ", "I was remaining");
  VerifyStroke("SWR-FRPLSZ", "I had remained");
  VerifyStroke("KWHR-RPLSZ", "he remained");

  VerifyStroke("SWR-R", "I run");
  VerifyStroke("SWR-ER", "I am running");
  VerifyStroke("SWR-FR", "I have run");
  VerifyStroke("KWHR-R", "he runs");
  VerifyStroke("SWR-RD", "I ran");
  VerifyStroke("SWR-ERD", "I was running");
  VerifyStroke("SWR-FRD", "I had run");
  VerifyStroke("KWHR-RD", "he ran");

  VerifyStroke("SWR-BS", "I say");
  VerifyStroke("SWR-EBS", "I am saying");
  VerifyStroke("SWR-FBS", "I have said");
  VerifyStroke("KWHR-BS", "he says");
  VerifyStroke("SWR-BSZ", "I said");
  VerifyStroke("SWR-EBSZ", "I was saying");
  VerifyStroke("SWR-FBSZ", "I had said");
  VerifyStroke("KWHR-BSZ", "he said");

  VerifyStroke("SWR-BTS", "I say that");
  VerifyStroke("SWR-EBTS", "I am saying that");
  VerifyStroke("SWR-FBTS", "I have said that");
  VerifyStroke("KWHR-BTS", "he says that");
  VerifyStroke("SWR-BTSDZ", "I said that");
  VerifyStroke("SWR-EBTSDZ", "I was saying that");
  VerifyStroke("SWR-FBTSDZ", "I had said that");
  VerifyStroke("KWHR-BTSDZ", "he said that");

  VerifyStroke("SWR-S", "I see");
  VerifyStroke("SWR-ES", "I am seeing");
  VerifyStroke("SWR-FS", "I have seen");
  VerifyStroke("KWHR-S", "he sees");
  VerifyStroke("SWR-SZ", "I saw");
  VerifyStroke("SWR-ESZ", "I was seeing");
  VerifyStroke("SWR-FSZ", "I had seen");
  VerifyStroke("KWHR-SZ", "he saw");

  VerifyStroke("SWR-BLS", "I set");
  VerifyStroke("SWR-EBLS", "I am setting");
  VerifyStroke("SWR-FBLS", "I have set");
  VerifyStroke("KWHR-BLS", "he sets");
  VerifyStroke("SWR-BLSZ", "I set");
  VerifyStroke("SWR-EBLSZ", "I was setting");
  VerifyStroke("SWR-FBLSZ", "I had set");
  VerifyStroke("KWHR-BLSZ", "he set");

  VerifyStroke("SWR-PLS", "I seem");
  VerifyStroke("SWR-EPLS", "I am seeming");
  VerifyStroke("SWR-FPLS", "I have seemed");
  VerifyStroke("KWHR-PLS", "he seems");
  VerifyStroke("SWR-PLSZ", "I seemed");
  VerifyStroke("SWR-EPLSZ", "I was seeming");
  VerifyStroke("SWR-FPLSZ", "I had seemed");
  VerifyStroke("KWHR-PLSZ", "he seemed");

  VerifyStroke("SWR-PLTS", "I seem to");
  VerifyStroke("SWR-EPLTS", "I am seeming to");
  VerifyStroke("SWR-FPLTS", "I have seemed to");
  VerifyStroke("KWHR-PLTS", "he seems to");
  VerifyStroke("SWR-PLTSDZ", "I seemed to");
  VerifyStroke("SWR-EPLTSDZ", "I was seeming to");
  VerifyStroke("SWR-FPLTSDZ", "I had seemed to");
  VerifyStroke("KWHR-PLTSDZ", "he seemed to");

  VerifyStroke("SWR-RBZ", "I show");
  VerifyStroke("SWR-ERBZ", "I am showing");
  VerifyStroke("SWR-FRBZ", "I have shown");
  VerifyStroke("KWHR-RBZ", "he shows");
  VerifyStroke("SWR-RBDZ", "I showed");
  VerifyStroke("SWR-ERBDZ", "I was showing");
  VerifyStroke("SWR-FRBDZ", "I had shown");
  VerifyStroke("KWHR-RBDZ", "he showed");

  VerifyStroke("SWR-RBT", "I take");
  VerifyStroke("SWR-ERBT", "I am taking");
  VerifyStroke("SWR-FRBT", "I have taken");
  VerifyStroke("KWHR-RBT", "he takes");
  VerifyStroke("SWR-RBTD", "I took");
  VerifyStroke("SWR-ERBTD", "I was taking");
  VerifyStroke("SWR-FRBTD", "I had taken");
  VerifyStroke("KWHR-RBTD", "he took");

  // VerifyStroke("SWR-BLGT", "I talk");
  // VerifyStroke("SWR-EBLGT", "I am talking");
  // VerifyStroke("SWR-FBLGT", "I have talked");
  // VerifyStroke("KWHR-BLGT", "he talks");
  // VerifyStroke("SWR-BLGTD", "I talked");
  // VerifyStroke("SWR-EBLGTD", "I was talking");
  // VerifyStroke("SWR-FBLGTD", "I had talked");
  // VerifyStroke("KWHR-BLGTD", "he talked");

  VerifyStroke("SWR-RLT", "I tell");
  VerifyStroke("SWR-ERLT", "I am telling");
  VerifyStroke("SWR-FRLT", "I have told");
  VerifyStroke("KWHR-RLT", "he tells");
  VerifyStroke("SWR-RLTD", "I told");
  VerifyStroke("SWR-ERLTD", "I was telling");
  VerifyStroke("SWR-FRLTD", "I had told");
  VerifyStroke("KWHR-RLTD", "he told");

  VerifyStroke("SWR-PBG", "I think");
  VerifyStroke("SWR-EPBG", "I am thinking");
  VerifyStroke("SWR-FPBG", "I have thought");
  VerifyStroke("KWHR-PBG", "he thinks");
  VerifyStroke("SWR-PBGD", "I thought");
  VerifyStroke("SWR-EPBGD", "I was thinking");
  VerifyStroke("SWR-FPBGD", "I had thought");
  VerifyStroke("KWHR-PBGD", "he thought");

  VerifyStroke("SWR-PBGT", "I think that");
  VerifyStroke("SWR-EPBGT", "I am thinking that");
  VerifyStroke("SWR-FPBGT", "I have thought that");
  VerifyStroke("KWHR-PBGT", "he thinks that");
  VerifyStroke("SWR-PBGTD", "I thought that");
  VerifyStroke("SWR-EPBGTD", "I was thinking that");
  VerifyStroke("SWR-FPBGTD", "I had thought that");
  VerifyStroke("KWHR-PBGTD", "he thought that");

  VerifyStroke("SWR-RT", "I try");
  VerifyStroke("SWR-ERT", "I am trying");
  VerifyStroke("SWR-FRT", "I have tried");
  VerifyStroke("KWHR-RT", "he tries");
  VerifyStroke("SWR-RTD", "I tried");
  VerifyStroke("SWR-ERTD", "I was trying");
  VerifyStroke("SWR-FRTD", "I had tried");
  VerifyStroke("KWHR-RTD", "he tried");

  VerifyStroke("SWR-RTS", "I try to");
  VerifyStroke("SWR-ERTS", "I am trying to");
  VerifyStroke("SWR-FRTS", "I have tried to");
  VerifyStroke("KWHR-RTS", "he tries to");
  VerifyStroke("SWR-RTSDZ", "I tried to");
  VerifyStroke("SWR-ERTSDZ", "I was trying to");
  VerifyStroke("SWR-FRTSDZ", "I had tried to");
  VerifyStroke("KWHR-RTSDZ", "he tried to");

  VerifyStroke("SWR-RPB", "I understand");
  VerifyStroke("SWR-ERPB", "I am understanding");
  VerifyStroke("SWR-FRPB", "I have understood");
  VerifyStroke("KWHR-RPB", "he understands");
  VerifyStroke("SWR-RPBD", "I understood");
  VerifyStroke("SWR-ERPBD", "I was understanding");
  VerifyStroke("SWR-FRPBD", "I had understood");
  VerifyStroke("KWHR-RPBD", "he understood");

  VerifyStroke("SWR-RPBT", "I understand the");
  VerifyStroke("SWR-ERPBT", "I am understanding the");
  VerifyStroke("SWR-FRPBT", "I have understood the");
  VerifyStroke("KWHR-RPBT", "he understands the");
  VerifyStroke("SWR-RPBTD", "I understood the");
  VerifyStroke("SWR-ERPBTD", "I was understanding the");
  VerifyStroke("SWR-FRPBTD", "I had understood the");
  VerifyStroke("KWHR-RPBTD", "he understood the");

  VerifyStroke("SWR-Z", "I use");
  VerifyStroke("SWR-EZ", "I am using");
  VerifyStroke("SWR-FZ", "I have used");
  VerifyStroke("KWHR-Z", "he uses");
  VerifyStroke("SWR-DZ", "I used");
  VerifyStroke("SWR-EDZ", "I was using");
  VerifyStroke("SWR-FDZ", "I had used");
  VerifyStroke("KWHR-DZ", "he used");

  VerifyStroke("SWR-P", "I want");
  VerifyStroke("SWR-EP", "I am wanting");
  VerifyStroke("SWR-FP", "I have wanted");
  VerifyStroke("KWHR-P", "he wants");
  VerifyStroke("SWR-PD", "I wanted");
  VerifyStroke("SWR-EPD", "I was wanting");
  VerifyStroke("SWR-FPD", "I had wanted");
  VerifyStroke("KWHR-PD", "he wanted");

  VerifyStroke("SWR-PT", "I want to");
  VerifyStroke("SWR-EPT", "I am wanting to");
  VerifyStroke("SWR-FPT", "I have wanted to");
  VerifyStroke("KWHR-PT", "he wants to");
  VerifyStroke("SWR-PTD", "I wanted to");
  VerifyStroke("SWR-EPTD", "I was wanting to");
  VerifyStroke("SWR-FPTD", "I had wanted to");
  VerifyStroke("KWHR-PTD", "he wanted to");

  VerifyStroke("SWR-RBS", "I wish");
  VerifyStroke("SWR-ERBS", "I am wishing");
  VerifyStroke("SWR-FRBS", "I have wished");
  VerifyStroke("KWHR-RBS", "he wishes");
  VerifyStroke("SWR-RBSZ", "I wished");
  VerifyStroke("SWR-ERBSZ", "I was wishing");
  VerifyStroke("SWR-FRBSZ", "I had wished");
  VerifyStroke("KWHR-RBSZ", "he wished");

  VerifyStroke("SWR-RBTS", "I wish to");
  VerifyStroke("SWR-ERBTS", "I am wishing to");
  VerifyStroke("SWR-FRBTS", "I have wished to");
  VerifyStroke("KWHR-RBTS", "he wishes to");
  VerifyStroke("SWR-RBTSDZ", "I wished to");
  VerifyStroke("SWR-ERBTSDZ", "I was wishing to");
  VerifyStroke("SWR-FRBTSDZ", "I had wished to");
  VerifyStroke("KWHR-RBTSDZ", "he wished to");

  VerifyStroke("SWR-RBG", "I work");
  VerifyStroke("SWR-ERBG", "I am working");
  VerifyStroke("SWR-FRBG", "I have worked");
  VerifyStroke("KWHR-RBG", "he works");
  VerifyStroke("SWR-RBGD", "I worked");
  VerifyStroke("SWR-ERBGD", "I was working");
  VerifyStroke("SWR-FRBGD", "I had worked");
  VerifyStroke("KWHR-RBGD", "he worked");

  VerifyStroke("SWR-RBGT", "I work on");
  VerifyStroke("SWR-ERBGT", "I am working on");
  VerifyStroke("SWR-FRBGT", "I have worked on");
  VerifyStroke("KWHR-RBGT", "he works on");
  VerifyStroke("SWR-RBGTD", "I worked on");
  VerifyStroke("SWR-ERBGTD", "I was working on");
  VerifyStroke("SWR-FRBGTD", "I had worked on");
  VerifyStroke("KWHR-RBGTD", "he worked on");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Omit past tense lookup when present exists") {
  StenoReverseDictionaryLookup lookup(2, "to read");
  StenoJeffPhrasingDictionary::instance.ReverseLookup(lookup);
  assert(lookup.resultCount == 2);
}
TEST_END

TEST_BEGIN("JeffPhrasing: Include all past tense results when there's no "
           "present tense") {
  StenoReverseDictionaryLookup lookup(2, "you were");
  StenoJeffPhrasingDictionary::instance.ReverseLookup(lookup);
  assert(lookup.resultCount == 2);
}
TEST_END

static void VerifyReverseLookup(const char *text, StenoStroke expected) {
  StenoReverseDictionaryLookup lookup(2, text);
  StenoJeffPhrasingDictionary::instance.ReverseLookup(lookup);
  assert(lookup.resultCount > 0);
  for (size_t i = 0; i < lookup.resultCount; ++i) {
    assert(lookup.results[i].length == 1);
    if (lookup.strokes[i] == expected) {
      return;
    }
  }
  assert(false);
}

TEST_BEGIN("JeffPhrasing: Reverse lookups") {
  // spellchecker: disable
  VerifyReverseLookup("I have been going", StenoStroke("SWREFG"));
  VerifyReverseLookup("if I did it", StenoStroke("STPAEURPTD"));
  VerifyReverseLookup("I heard", StenoStroke("SWR-PGD"));
  VerifyReverseLookup("I didn't like", StenoStroke("SWR*BLGD"));
  VerifyReverseLookup("to go to", StenoStroke("STWRUGT"));
  VerifyReverseLookup("there are", StenoStroke("STPHR-B"));
  VerifyReverseLookup("can I", StenoStroke("SWRAU"));
  VerifyReverseLookup("I", StenoStroke("SWR"));
  VerifyReverseLookup("I can", StenoStroke("SWRA"));
  VerifyReverseLookup("I can't", StenoStroke("SWRA*"));
  VerifyReverseLookup("I need to", StenoStroke("SWR-RPGT"));
  VerifyReverseLookup("I am going to", StenoStroke("SWREGT"));
  VerifyReverseLookup("if you go", StenoStroke("STPAUG"));
  VerifyReverseLookup("but I considered", StenoStroke("SPWHEURBGDZ"));
  // spellchecker: enable
}
TEST_END

//---------------------------------------------------------------------------
