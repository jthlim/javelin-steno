//---------------------------------------------------------------------------

#include "jeff_phrasing_dictionary.h"
#include "../chord.h"
#include "../crc32.h"
#include "../pattern.h"
#include "../str.h"
#include "dictionary.h"
#include "jeff_phrasing_dictionary_data.h"
#include <assert.h>

//---------------------------------------------------------------------------

// spellchecker: disable
const StenoChord SIMPLE_STARTER_MASK(0x3fe);  // `STKPWHRAO`
const StenoChord SIMPLE_PRONOUN_MASK(0x1C00); // `*EU`
const StenoChord FULL_STARTER_MASK(0xfe);     // 'STKPWHR'
const StenoChord ENDER_MASK(0x7fc000);        // '-RPBLGTSDZ'

const StenoChord STRUCTURE_EXCEPTION1_MASK(0x3ffe); // STKPWHRAO*EUF
const StenoChord STRUCTURE_EXCEPTION2_MASK(0x3f00); // AO*EUF
// spellchecker: enable

const StenoJeffPhrasingDictionary StenoJeffPhrasingDictionary::instance;

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

bool JeffPhrasingDictionaryData::IsValidPhraseChord(StenoChord chord) const {
  for (size_t i = 0; i < nonPhraseStrokeCount; ++i) {
    if (nonPhraseStrokes[i] == chord) {
      return false;
    }
  }
  return true;
}

const JeffPhrasingMiddle *
JeffPhrasingDictionaryData::LookupSimpleStarter(StenoChord chord) const {
  for (size_t i = 0; i < simpleStarterCount; ++i) {
    if (simpleStarters[i].chord == chord) {
      return &simpleStarters[i].middle;
    }
  }
  return nullptr;
}

const JeffPhrasingPronoun *
JeffPhrasingDictionaryData::LookupFullStarter(StenoChord chord) const {
  for (size_t i = 0; i < fullStarterCount; ++i) {
    if (fullStarters[i].chord == chord) {
      return &fullStarters[i].pronoun;
    }
  }
  return nullptr;
}

const JeffPhrasingEnder *
JeffPhrasingDictionaryData::LookupEnder(StenoChord chord) const {
  size_t index = chord.Hash();
  for (;;) {
    const JeffPhrasingEnder *ender =
        enderHashMap[index & (enderHashMapSize - 1)];

    if (ender == nullptr) {
      return nullptr;
    }
    if (ender->chord == chord) {
      return ender;
    }

    ++index;
  }

  // for (size_t i = 0; i < endersCount; ++i) {
  //   if (enders[i].chord == chord) {
  //     return &enders[i];
  //   }
  // }
  // return nullptr;
}

const JeffPhrasingStructure *
JeffPhrasingDictionaryData::LookupUniqueStarter(StenoChord chord) const {
  for (size_t i = 0; i < uniqueStarterCount; ++i) {
    if (uniqueStarters[i].chord == chord) {
      return &uniqueStarters[i].structure;
    }
  }
  return nullptr;
}

const JeffPhrasingStructure *
JeffPhrasingDictionaryData::LookupStructureException(StenoChord chord) const {
  for (size_t i = 0; i < structureExceptionCount; ++i) {
    if (structureExceptions[i].chord == chord) {
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

PhrasingParts DetermineParts(StenoChord chord) {
  if ((chord & ChordMask::NUM).IsNotEmpty()) {
    return PhrasingParts::CreateInvalid();
  }

  if (!phrasingData.IsValidPhraseChord(chord)) {
    return PhrasingParts::CreateInvalid();
  }

  const JeffPhrasingEnder *ender = phrasingData.LookupEnder(chord & ENDER_MASK);
  if (!ender) {
    return PhrasingParts::CreateInvalid();
  }

  // Try simple starters first.
  if ((chord & SIMPLE_PRONOUN_MASK).IsNotEmpty()) {
    const JeffPhrasingMiddle *simpleStarter =
        phrasingData.LookupSimpleStarter(chord & SIMPLE_STARTER_MASK);

    if (simpleStarter) {
      size_t pronounIndex = (chord.GetKeyState() >> ChordBitIndex::STAR) & 7;
      const JeffPhrasingPronoun *pronoun =
          phrasingData.simplePronouns + pronounIndex;

      size_t structureIndex = (chord.GetKeyState() >> ChordBitIndex::FR) & 1;
      const JeffPhrasingStructure *structure =
          phrasingData.simpleStructures + structureIndex;

      return PhrasingParts(pronoun, simpleStarter, structure, ender);
    }
  }

  const JeffPhrasingPronoun *fullStarter =
      phrasingData.LookupFullStarter(chord & FULL_STARTER_MASK);

  if (fullStarter != nullptr) {
    size_t middleIndex = (chord.GetKeyState() >> ChordBitIndex::A) & 7;
    const JeffPhrasingMiddle *middle = phrasingData.fullMiddles + middleIndex;

    const JeffPhrasingStructure *structure =
        phrasingData.LookupUniqueStarter(chord & STRUCTURE_EXCEPTION1_MASK);

    if (!structure) {
      structure = phrasingData.LookupStructureException(
          chord & STRUCTURE_EXCEPTION2_MASK);
    }

    if (!structure) {
      size_t structureIndex = (chord.GetKeyState() >> ChordBitIndex::STAR) & 15;
      structure = phrasingData.fullStructures + structureIndex;
    }
    return PhrasingParts(fullStarter, middle, structure, ender);
  }

  return PhrasingParts::CreateInvalid();
}

StenoDictionaryLookupResult
StenoJeffPhrasingDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);

  PhrasingParts parts = DetermineParts(lookup.chords[0]);
  if (!parts.IsValid()) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  return StenoDictionaryLookupResult::CreateDynamicString(parts.CreatePhrase());
}

const StenoDictionary *StenoJeffPhrasingDictionary::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);

  PhrasingParts parts = DetermineParts(lookup.chords[0]);
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
  return "jeff_phrasing";
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

  List<StenoChord> testedChords;
  RecurseCheckReverseLookup(result, result.lookup, StenoChord(), 0, 0, 3,
                            testedChords);
}

void StenoJeffPhrasingDictionary::RecurseCheckReverseLookup(
    StenoReverseDictionaryLookup &result, const char *p, StenoChord stroke,
    uint32_t hash, uint8_t componentMask, uint8_t modeMask,
    List<StenoChord> &testedChords) const {
  while (*p == ' ' && *p != '\0') {
    ++p;
  }

  const char *pEnd = p;
  while (*pEnd != '\0' && *pEnd != ' ') {
    ++pEnd;
  }

  if (p == pEnd) {
    if ((componentMask & 1) == 0) {
      // There must be a starter for simple forms.
      if (modeMask & 2) {
        return;
      }

      componentMask |= 1;
      hash += 0x710e300b; // CRC for "\\0"

      for (const JeffPhrasingReverseHashMapEntry *entry =
               phrasingData.reverseEntries;
           entry->hash == 0; ++entry) {
        if (entry->componentMask == 1) {
          RecurseCheckReverseLookup(result, p, stroke | entry->stroke, hash,
                                    componentMask, modeMask, testedChords);
        }
      }
    }
    if ((componentMask & 2) == 0) {
      // Simple forms must have a middle.
      componentMask |= 2;
      if (modeMask & 2) {
        return;
      }

      // Recurse, once with no change, then proceed with \\1 appended.
      RecurseCheckReverseLookup(result, p, stroke, hash, componentMask,
                                modeMask, testedChords);
      hash += 0x0609009d; // CRC for "\\1"
    }

    if ((componentMask & 8) == 0) {
      hash += 0x9f005127; // CRC for "\\2"
      hash += 0xe80761b1; // CRC for "\\3"

      if (componentMask & 0x80) {
        stroke |= StenoChord(ChordMask::DR);
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
        StenoChord lookupChord = stroke | entry->stroke;
        if (testedChords.Contains(lookupChord)) {
          continue;
        }
        testedChords.Add(lookupChord);
        StenoDictionaryLookupResult lookup = Lookup(&lookupChord, 1);
        if (lookup.IsValid()) {
          const char *lookupText = lookup.GetText();
          if (*lookupText == ' ') {
            ++lookupText;
          }
          if (Str::Eq(lookupText, result.lookup)) {
            result.AddResult(&lookupChord, 1, this);
          }
          lookup.Destroy();
        }
      }
    }
    return;
  }

  uint32_t wordHash = Crc32(p, pEnd - p);
  const JeffPhrasingReverseHashMapEntry *entry =
      phrasingData.LookupReverseWord(wordHash);
  if (entry) {
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
        for (; entry2->hash == wordHash2; ++entry2) {
          uint8_t updatedModeMask2 = entry2->modeMask & modeMask;
          if (updatedModeMask2 == 0) {
            continue;
          }
          if (componentMask & entry2->componentMask) {
            continue;
          }

          if (entry2->replaceHash != 0) {
            RecurseCheckReverseLookup(result, pEnd2, stroke | entry2->stroke,
                                      hash + entry2->replaceHash,
                                      componentMask | entry2->componentMask,
                                      updatedModeMask2, testedChords);
          }
        }
      }
    }

    for (; entry->hash == wordHash; ++entry) {
      uint8_t updatedModeMask = entry->modeMask & modeMask;
      if (updatedModeMask == 0) {
        continue;
      }
      if (componentMask & entry->componentMask) {
        continue;
      }
      if (entry->replaceHash != 0) {
        RecurseCheckReverseLookup(result, pEnd, stroke | entry->stroke,
                                  hash + entry->replaceHash,
                                  componentMask | entry->componentMask,
                                  updatedModeMask, testedChords);
      }
    }
  }
}

inline bool IsValidPhraseCharacter(int c) {
  return ('a' <= c && c <= 'z') || c == 'I' || c == '\'' || c == ' ';
}

bool StenoJeffPhrasingDictionary::ContainsNonPhraseCharacter(const char *p) {
  while (*p) {
    int c = *p++;
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

void VerifyChord(const char *chord, const char *result) {
  StenoChord stenoChord;
  stenoChord.Set(chord);

  auto lookup = StenoJeffPhrasingDictionary::instance.Lookup(&stenoChord, 1);
  if (!lookup.IsValid()) {
    printf("Failed to lookup %s\n", chord);

    assert(lookup.IsValid());
  }
  if (!Str::Eq(lookup.GetText(), result)) {
    printf("Lookup for %s expected '%s', got '%s'\n", chord, result,
           lookup.GetText());

    assert(Str::Eq(lookup.GetText(), result));
  }
  lookup.Destroy();
}

TEST_BEGIN("JeffPhrasing: Starter tests") {
  // spellchecker: disable
  VerifyChord("SWR", "I");
  VerifyChord("KPWR", "you");

  VerifyChord("KWHR", "he");
  VerifyChord("SKWHR", "she");
  VerifyChord("KPWH", "it");
  VerifyChord("TWH", "they");
  VerifyChord("STKH", "this");
  VerifyChord("STWH", "that");
  //    VerifyChord("STHR", "there");
  //    VerifyChord("STPHR", "there");
  VerifyChord("STKPWHR", "");
  VerifyChord("STWR", "");

  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: \"There\" tests") {
  // spellchecker: disable
  VerifyChord("STHRB", "there is");
  VerifyChord("STPHRB", "there are");

  // There shouldn't be able to access all enders, eg. "ask"
  StenoChord chord("STHR-RB");
  auto lookup = StenoJeffPhrasingDictionary::instance.Lookup(&chord, 1);
  assert(!lookup.IsValid());

  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Unique starters tests") {
  // spellchecker: disable
  VerifyChord("STWRU", "to");
  VerifyChord("STWR*U", "not to");
  VerifyChord("STKPWHRU", "to");
  VerifyChord("STKPWHR*U", "not to");
  VerifyChord("STWRUF", "just");
  VerifyChord("STWR*UF", "just don't");
  VerifyChord("STKPWHRUF", "just");
  VerifyChord("STKPWHR*UF", "just doesn't");
  VerifyChord("STWREU", "still");
  VerifyChord("STWR*EU", "still don't");
  VerifyChord("STKPWHREU", "still");
  VerifyChord("STKPWHR*EU", "still doesn't");
  VerifyChord("STWREUF", "never");
  VerifyChord("STWR*EUF", "even");
  VerifyChord("STKPWHREUF", "never");
  VerifyChord("STKPWHR*EUF", "even");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Simple phrases tests") {
  // spellchecker: disable
  VerifyChord("SKPEU", "and I");
  VerifyChord("SKPEUF", "and I have");
  VerifyChord("SKPEUFD", "and I had");
  VerifyChord("SKPEURB", "and I ask");
  VerifyChord("SKPEURBD", "and I asked");
  VerifyChord("SKPEUFRB", "and I have asked");
  VerifyChord("SKPEUFRBD", "and I had asked");
  VerifyChord("SKPE", "and he");
  VerifyChord("SKPEF", "and he has");
  VerifyChord("SKPEFD", "and he had");
  VerifyChord("SKPERB", "and he asks");
  VerifyChord("SKPERBD", "and he asked");
  VerifyChord("SKPEFRB", "and he has asked");
  VerifyChord("SKPEFRBD", "and he had asked");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Middle tests") {
  // spellchecker: disable
  VerifyChord("SWRA", "I can");
  VerifyChord("KPWRA", "you can");
  VerifyChord("KWHRA", "he can");
  VerifyChord("SWRO", "I shall");
  VerifyChord("KPWRO", "you shall");
  VerifyChord("KWHRO", "he shall");
  VerifyChord("SWRAO", "I will");
  VerifyChord("KPWRAO", "you will");
  VerifyChord("KWHRAO", "he will");
  VerifyChord("SWR*", "I don't");
  VerifyChord("KPWR*", "you don't");
  VerifyChord("KWHR*", "he doesn't");
  VerifyChord("SWRA*", "I can't");
  VerifyChord("KPWRA*", "you can't");
  VerifyChord("KWHRA*", "he can't");
  VerifyChord("SWRO*", "I shall not");
  VerifyChord("KPWRO*", "you shall not");
  VerifyChord("KWHRO*", "he shall not");
  VerifyChord("SWRAO*", "I won't");
  VerifyChord("KPWRAO*", "you won't");
  VerifyChord("KWHRAO*", "he won't");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Swap middle tests") {
  // spellchecker: disable
  VerifyChord("SWRAU", " can I");
  VerifyChord("KPWRAU", " can you");
  VerifyChord("KWHRAU", " can he");
  VerifyChord("SWROU", " shall I");
  VerifyChord("KPWROU", " shall you");
  VerifyChord("KWHROU", " shall he");
  VerifyChord("SWRAOU", " will I");
  VerifyChord("KPWRAOU", " will you");
  VerifyChord("KWHRAOU", " will he");
  VerifyChord("SWR*U", " don't I");
  VerifyChord("KPWR*U", " don't you");
  VerifyChord("KWHR*U", " doesn't he");
  VerifyChord("SWRA*U", " can't I");
  VerifyChord("KPWRA*U", " can't you");
  VerifyChord("KWHRA*U", " can't he");
  VerifyChord("SWRO*U", " shall not I");
  VerifyChord("KPWRO*U", " shall not you");
  VerifyChord("KWHRO*U", " shall not he");
  VerifyChord("SWRAO*U", " won't I");
  VerifyChord("KPWRAO*U", " won't you");
  VerifyChord("KWHRAO*U", " won't he");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Basic structures tests") {
  // spellchecker: disable
  VerifyChord("SWREURB", "I still ask");
  VerifyChord("SWRAEURB", "I can still ask");
  VerifyChord("SWRA*EURB", "I still can't ask");

  VerifyChord("SWRUFRB", "I just ask");
  VerifyChord("SWRAUFURB", "I can just ask");
  VerifyChord("SWRA*UFRB", "I just can't ask");

  VerifyChord("SWREUFRB", "I never ask");
  VerifyChord("SWRAEUFURB", "I can never ask");
  VerifyChord("SWRA*EUFRB", "I can't even ask");

  VerifyChord("SWRA-G", "I can go");
  VerifyChord("SWRA*G", "I can't go");
  VerifyChord("SWRA-EG", "I can be going");
  VerifyChord("SWRA*EG", "I can't be going");
  VerifyChord("SWRA-UG", " can I go");
  VerifyChord("SWRA*UG", " can't I go");
  VerifyChord("SWRA-EUG", "I can still go");
  VerifyChord("SWRA*EUG", "I still can't go");

  VerifyChord("SWRA-FG", "I can have gone");
  VerifyChord("SWRA*FG", "I can't have gone");
  VerifyChord("SWRA-EFG", "I can have been going");
  VerifyChord("SWRA*EFG", "I can't have been going");
  VerifyChord("SWRA-UFG", "I can just go");
  VerifyChord("SWRA*UFG", "I just can't go");
  VerifyChord("SWRA-EUFG", "I can never go");
  VerifyChord("SWRA*EUFG", "I can't even go");

  VerifyChord("SWRA-GD", "I could go");
  VerifyChord("SWRA*GD", "I couldn't go");
  VerifyChord("SWRA-EGD", "I could be going");
  VerifyChord("SWRA*EGD", "I couldn't be going");
  VerifyChord("SWRA-UGD", " could I go");
  VerifyChord("SWRA*UGD", " couldn't I go");
  VerifyChord("SWRA-EUGD", "I could still go");
  VerifyChord("SWRA*EUGD", "I still couldn't go");

  VerifyChord("SWRA-FGD", "I could have gone");
  VerifyChord("SWRA*FGD", "I couldn't have gone");
  VerifyChord("SWRA-EFGD", "I could have been going");
  VerifyChord("SWRA*EFGD", "I couldn't have been going");
  VerifyChord("SWRA-UFGD", "I could just go");
  VerifyChord("SWRA*UFGD", "I just couldn't go");
  VerifyChord("SWRA-EUFGD", "I could never go");
  VerifyChord("SWRA*EUFGD", "I couldn't even go");

  VerifyChord("SWR-G", "I go");
  VerifyChord("SWR*G", "I don't go");
  VerifyChord("SWR-EG", "I am going");
  VerifyChord("SWR*EG", "I am not going");
  VerifyChord("SWR-UG", " do I go");
  VerifyChord("SWR*UG", " don't I go");
  VerifyChord("SWR-EUG", "I still go");
  VerifyChord("SWR*EUG", "I still don't go");

  VerifyChord("SWR-FG", "I have gone");
  VerifyChord("SWR*FG", "I haven't gone");
  VerifyChord("SWR-EFG", "I have been going");
  VerifyChord("SWR*EFG", "I haven't been going");
  VerifyChord("SWR-UFG", "I just go");
  VerifyChord("SWR*UFG", "I just don't go");
  VerifyChord("SWR-EUFG", "I never go");
  VerifyChord("SWR*EUFG", "I don't even go");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Non-conjugating enders tests") {
  // spellchecker: disable
  VerifyChord("SWR-BGS", "I can");
  VerifyChord("SWR-BGSZ", "I could");

  VerifyChord("SWR-RLG", "I really");
  VerifyChord("SWR-RLGD", "I really");

  VerifyChord("SWR-PL", "I may");
  VerifyChord("SWR-PLD", "I might");
  VerifyChord("SWR-PLT", "I may be");
  VerifyChord("SWR-PLTD", "I might be");

  VerifyChord("SWR-PBLGS", "I must");
  VerifyChord("SWR-PBLGTS", "I must be");

  VerifyChord("SWR-RBL", "I shall");
  VerifyChord("SWR-RBLD", "I should");

  VerifyChord("SWR-TZ", "I used to");
  VerifyChord("SWR-TDZ", "I used to");

  VerifyChord("SWR-RBGS", "I will");
  VerifyChord("SWR-RBGSZ", "I would");
  // spellchecker: enable
}
TEST_END

TEST_BEGIN("JeffPhrasing: Enders tests") {
  // spellchecker: disable
  VerifyChord("SWR-RB", "I ask");
  VerifyChord("SWR-ERB", "I am asking");
  VerifyChord("SWR-FRB", "I have asked");
  VerifyChord("KWHR-RB", "he asks");
  VerifyChord("SWR-RBD", "I asked");
  VerifyChord("SWR-ERBD", "I was asking");
  VerifyChord("SWR-FRBD", "I had asked");
  VerifyChord("KWHR-RBD", "he asked");

  VerifyChord("SWR-B", "I am");
  VerifyChord("SWR-EB", "I am being");
  VerifyChord("SWR-FB", "I have been");
  VerifyChord("KWHR-B", "he is");
  VerifyChord("TWH-B", "they are");
  VerifyChord("SWR-BD", "I was");
  VerifyChord("SWR-EBD", "I was being");
  VerifyChord("SWR-FBD", "I had been");
  VerifyChord("KWHR-BD", "he was");
  VerifyChord("TWH-BD", "they were");

  VerifyChord("SWR-BT", "I am a");
  VerifyChord("SWR-EBT", "I am being a");
  VerifyChord("SWR-FBT", "I have been a");
  VerifyChord("KWHR-BT", "he is a");
  VerifyChord("TWH-BT", "they are a");
  VerifyChord("SWR-BTD", "I was a");
  VerifyChord("SWR-EBTD", "I was being a");
  VerifyChord("SWR-FBTD", "I had been a");
  VerifyChord("KWHR-BTD", "he was a");
  VerifyChord("TWH-BTD", "they were a");

  VerifyChord("SWR-RPBG", "I become");
  VerifyChord("SWR-ERPBG", "I am becoming");
  VerifyChord("SWR-FRPBG", "I have become");
  VerifyChord("KWHR-RPBG", "he becomes");
  VerifyChord("SWR-RPBGD", "I became");
  VerifyChord("SWR-ERPBGD", "I was becoming");
  VerifyChord("SWR-FRPBGD", "I had become");
  VerifyChord("KWHR-RPBGD", "he became");

  VerifyChord("SWR-RPBGT", "I become a");
  VerifyChord("SWR-ERPBGT", "I am becoming a");
  VerifyChord("SWR-FRPBGT", "I have become a");
  VerifyChord("KWHR-RPBGT", "he becomes a");
  VerifyChord("SWR-RPBGTD", "I became a");
  VerifyChord("SWR-ERPBGTD", "I was becoming a");
  VerifyChord("SWR-FRPBGTD", "I had become a");
  VerifyChord("KWHR-RPBGTD", "he became a");

  VerifyChord("SWR-BL", "I believe");
  VerifyChord("SWR-EBL", "I am believing");
  VerifyChord("SWR-FBL", "I have believed");
  VerifyChord("KWHR-BL", "he believes");
  VerifyChord("SWR-BLD", "I believed");
  VerifyChord("SWR-EBLD", "I was believing");
  VerifyChord("SWR-FBLD", "I had believed");
  VerifyChord("KWHR-BLD", "he believed");

  VerifyChord("SWR-BLT", "I believe that");
  VerifyChord("SWR-EBLT", "I am believing that");
  VerifyChord("SWR-FBLT", "I have believed that");
  VerifyChord("KWHR-BLT", "he believes that");
  VerifyChord("SWR-BLTD", "I believed that");
  VerifyChord("SWR-EBLTD", "I was believing that");
  VerifyChord("SWR-FBLTD", "I had believed that");
  VerifyChord("KWHR-BLTD", "he believed that");

  VerifyChord("SWR-RBLG", "I call");
  VerifyChord("SWR-ERBLG", "I am calling");
  VerifyChord("SWR-FRBLG", "I have called");
  VerifyChord("KWHR-RBLG", "he calls");
  VerifyChord("SWR-RBLGD", "I called");
  VerifyChord("SWR-ERBLGD", "I was calling");
  VerifyChord("SWR-FRBLGD", "I had called");
  VerifyChord("KWHR-RBLGD", "he called");

  VerifyChord("SWR-RZ", "I care");
  VerifyChord("SWR-ERZ", "I am caring");
  VerifyChord("SWR-FRZ", "I have cared");
  VerifyChord("KWHR-RZ", "he cares");
  VerifyChord("SWR-RZD", "I cared");
  VerifyChord("SWR-ERZD", "I was caring");
  VerifyChord("SWR-FRZD", "I had cared");
  VerifyChord("KWHR-RZD", "he cared");

  VerifyChord("SWR-BG", "I come");
  VerifyChord("SWR-EBG", "I am coming");
  VerifyChord("SWR-FBG", "I have come");
  VerifyChord("KWHR-BG", "he comes");
  VerifyChord("SWR-BGD", "I came");
  VerifyChord("SWR-EBGD", "I was coming");
  VerifyChord("SWR-FBGD", "I had come");
  VerifyChord("KWHR-BGD", "he came");

  VerifyChord("SWR-BGT", "I come to");
  VerifyChord("SWR-EBGT", "I am coming to");
  VerifyChord("SWR-FBGT", "I have come to");
  VerifyChord("KWHR-BGT", "he comes to");
  VerifyChord("SWR-BGTD", "I came to");
  VerifyChord("SWR-EBGTD", "I was coming to");
  VerifyChord("SWR-FBGTD", "I had come to");
  VerifyChord("KWHR-BGTD", "he came to");

  VerifyChord("SWR-RP", "I do");
  VerifyChord("SWR-ERP", "I am doing");
  VerifyChord("SWR-FRP", "I have done");
  VerifyChord("KWHR-RP", "he does");
  VerifyChord("SWR-RPD", "I did");
  VerifyChord("SWR-ERPD", "I was doing");
  VerifyChord("SWR-FRPD", "I had done");
  VerifyChord("KWHR-RPD", "he did");

  VerifyChord("SWR-RPT", "I do it");
  VerifyChord("SWR-ERPT", "I am doing it");
  VerifyChord("SWR-FRPT", "I have done it");
  VerifyChord("KWHR-RPT", "he does it");
  VerifyChord("SWR-RPTD", "I did it");
  VerifyChord("SWR-ERPTD", "I was doing it");
  VerifyChord("SWR-FRPTD", "I had done it");
  VerifyChord("KWHR-RPTD", "he did it");

  VerifyChord("SWR-PGS", "I expect");
  VerifyChord("SWR-EPGS", "I am expecting");
  VerifyChord("SWR-FPGS", "I have expected");
  VerifyChord("KWHR-PGS", "he expects");
  VerifyChord("SWR-PGSZ", "I expected");
  VerifyChord("SWR-EPGSZ", "I was expecting");
  VerifyChord("SWR-FPGSZ", "I had expected");
  VerifyChord("KWHR-PGSZ", "he expected");

  VerifyChord("SWR-PGTS", "I expect that");
  VerifyChord("SWR-EPGTS", "I am expecting that");
  VerifyChord("SWR-FPGTS", "I have expected that");
  VerifyChord("KWHR-PGTS", "he expects that");
  VerifyChord("SWR-PGTSDZ", "I expected that");
  VerifyChord("SWR-EPGTSDZ", "I was expecting that");
  VerifyChord("SWR-FPGTSDZ", "I had expected that");
  VerifyChord("KWHR-PGTSDZ", "he expected that");

  VerifyChord("SWR-LT", "I feel");
  VerifyChord("SWR-ELT", "I am feeling");
  VerifyChord("SWR-FLT", "I have felt");
  VerifyChord("KWHR-LT", "he feels");
  VerifyChord("SWR-LTD", "I felt");
  VerifyChord("SWR-ELTD", "I was feeling");
  VerifyChord("SWR-FLTD", "I had felt");
  VerifyChord("KWHR-LTD", "he felt");

  VerifyChord("SWR-LTS", "I feel like");
  VerifyChord("SWR-ELTS", "I am feeling like");
  VerifyChord("SWR-FLTS", "I have felt like");
  VerifyChord("KWHR-LTS", "he feels like");
  VerifyChord("SWR-LTSDZ", "I felt like");
  VerifyChord("SWR-ELTSDZ", "I was feeling like");
  VerifyChord("SWR-FLTSDZ", "I had felt like");
  VerifyChord("KWHR-LTSDZ", "he felt like");

  VerifyChord("SWR-PBLG", "I find");
  VerifyChord("SWR-EPBLG", "I am finding");
  VerifyChord("SWR-FPBLG", "I have found");
  VerifyChord("KWHR-PBLG", "he finds");
  VerifyChord("SWR-PBLGD", "I found");
  VerifyChord("SWR-EPBLGD", "I was finding");
  VerifyChord("SWR-FPBLGD", "I had found");
  VerifyChord("KWHR-PBLGD", "he found");

  VerifyChord("SWR-PBLGT", "I find that");
  VerifyChord("SWR-EPBLGT", "I am finding that");
  VerifyChord("SWR-FPBLGT", "I have found that");
  VerifyChord("KWHR-PBLGT", "he finds that");
  VerifyChord("SWR-PBLGTD", "I found that");
  VerifyChord("SWR-EPBLGTD", "I was finding that");
  VerifyChord("SWR-FPBLGTD", "I had found that");
  VerifyChord("KWHR-PBLGTD", "he found that");

  VerifyChord("SWR-RG", "I forget");
  VerifyChord("SWR-ERG", "I am forgetting");
  VerifyChord("SWR-FRG", "I have forgotten");
  VerifyChord("KWHR-RG", "he forgets");
  VerifyChord("SWR-RGD", "I forgot");
  VerifyChord("SWR-ERGD", "I was forgetting");
  VerifyChord("SWR-FRGD", "I had forgotten");
  VerifyChord("KWHR-RGD", "he forgot");

  VerifyChord("SWR-RGT", "I forget to");
  VerifyChord("SWR-ERGT", "I am forgetting to");
  VerifyChord("SWR-FRGT", "I have forgotten to");
  VerifyChord("KWHR-RGT", "he forgets to");
  VerifyChord("SWR-RGTD", "I forgot to");
  VerifyChord("SWR-ERGTD", "I was forgetting to");
  VerifyChord("SWR-FRGTD", "I had forgotten to");
  VerifyChord("KWHR-RGTD", "he forgot to");

  VerifyChord("SWR-GS", "I get");
  VerifyChord("SWR-EGS", "I am getting");
  VerifyChord("SWR-FGS", "I have gotten");
  VerifyChord("KWHR-GS", "he gets");
  VerifyChord("SWR-GSZ", "I got");
  VerifyChord("SWR-EGSZ", "I was getting");
  VerifyChord("SWR-FGSZ", "I had gotten");
  VerifyChord("KWHR-GSZ", "he got");

  VerifyChord("SWR-GTS", "I get to");
  VerifyChord("SWR-EGTS", "I am getting to");
  VerifyChord("SWR-FGTS", "I have gotten to");
  VerifyChord("KWHR-GTS", "he gets to");
  VerifyChord("SWR-GTSDZ", "I got to");
  VerifyChord("SWR-EGTSDZ", "I was getting to");
  VerifyChord("SWR-FGTSDZ", "I had gotten to");
  VerifyChord("KWHR-GTSDZ", "he got to");

  VerifyChord("SWR-GZ", "I give");
  VerifyChord("SWR-EGZ", "I am giving");
  VerifyChord("SWR-FGZ", "I have given");
  VerifyChord("KWHR-GZ", "he gives");
  VerifyChord("SWR-GDZ", "I gave");
  VerifyChord("SWR-EGDZ", "I was giving");
  VerifyChord("SWR-FGDZ", "I had given");
  VerifyChord("KWHR-GDZ", "he gave");

  VerifyChord("SWR-G", "I go");
  VerifyChord("SWR-EG", "I am going");
  VerifyChord("SWR-FG", "I have gone");
  VerifyChord("KWHR-G", "he goes");
  VerifyChord("SWR-GD", "I went");
  VerifyChord("SWR-EGD", "I was going");
  VerifyChord("SWR-FGD", "I had gone");
  VerifyChord("KWHR-GD", "he went");

  VerifyChord("SWR-GT", "I go to");
  VerifyChord("SWR-EGT", "I am going to");
  VerifyChord("SWR-FGT", "I have gone to");
  VerifyChord("KWHR-GT", "he goes to");
  VerifyChord("SWR-GTD", "I went to");
  VerifyChord("SWR-EGTD", "I was going to");
  VerifyChord("SWR-FGTD", "I had gone to");
  VerifyChord("KWHR-GTD", "he went to");

  VerifyChord("SWR-T", "I have");
  VerifyChord("SWR-ET", "I am having");
  VerifyChord("SWR-FT", "I have had");
  VerifyChord("KWHR-T", "he has");
  VerifyChord("SWR-TD", "I had");
  VerifyChord("SWR-ETD", "I was having");
  VerifyChord("SWR-FTD", "I had had");
  VerifyChord("KWHR-TD", "he had");

  VerifyChord("SWR-TS", "I have to");
  VerifyChord("SWR-ETS", "I am having to");
  VerifyChord("SWR-FTS", "I have had to");
  VerifyChord("KWHR-TS", "he has to");
  VerifyChord("SWR-TSDZ", "I had to");
  VerifyChord("SWR-ETSDZ", "I was having to");
  VerifyChord("SWR-FTSDZ", "I had had to");
  VerifyChord("KWHR-TSDZ", "he had to");

  VerifyChord("SWR-PZ", "I happen");
  VerifyChord("SWR-EPZ", "I am happening");
  VerifyChord("SWR-FPZ", "I have happened");
  VerifyChord("KWHR-PZ", "he happens");
  VerifyChord("SWR-PDZ", "I happened");
  VerifyChord("SWR-EPDZ", "I was happening");
  VerifyChord("SWR-FPDZ", "I had happened");
  VerifyChord("KWHR-PDZ", "he happened");

  VerifyChord("SWR-PG", "I hear");
  VerifyChord("SWR-EPG", "I am hearing");
  VerifyChord("SWR-FPG", "I have heard");
  VerifyChord("KWHR-PG", "he hears");
  VerifyChord("SWR-PGD", "I heard");
  VerifyChord("SWR-EPGD", "I was hearing");
  VerifyChord("SWR-FPGD", "I had heard");
  VerifyChord("KWHR-PGD", "he heard");

  VerifyChord("SWR-PGT", "I hear that");
  VerifyChord("SWR-EPGT", "I am hearing that");
  VerifyChord("SWR-FPGT", "I have heard that");
  VerifyChord("KWHR-PGT", "he hears that");
  VerifyChord("SWR-PGTD", "I heard that");
  VerifyChord("SWR-EPGTD", "I was hearing that");
  VerifyChord("SWR-FPGTD", "I had heard that");
  VerifyChord("KWHR-PGTD", "he heard that");

  VerifyChord("SWR-RPS", "I hope");
  VerifyChord("SWR-ERPS", "I am hoping");
  VerifyChord("SWR-FRPS", "I have hoped");
  VerifyChord("KWHR-RPS", "he hopes");
  VerifyChord("SWR-RPSZ", "I hoped");
  VerifyChord("SWR-ERPSZ", "I was hoping");
  VerifyChord("SWR-FRPSZ", "I had hoped");
  VerifyChord("KWHR-RPSZ", "he hoped");

  VerifyChord("SWR-RPTS", "I hope to");
  VerifyChord("SWR-ERPTS", "I am hoping to");
  VerifyChord("SWR-FRPTS", "I have hoped to");
  VerifyChord("KWHR-RPTS", "he hopes to");
  VerifyChord("SWR-RPTSDZ", "I hoped to");
  VerifyChord("SWR-ERPTSDZ", "I was hoping to");
  VerifyChord("SWR-FRPTSDZ", "I had hoped to");
  VerifyChord("KWHR-RPTSDZ", "he hoped to");

  VerifyChord("SWR-PLG", "I imagine");
  VerifyChord("SWR-EPLG", "I am imagining");
  VerifyChord("SWR-FPLG", "I have imagined");
  VerifyChord("KWHR-PLG", "he imagines");
  VerifyChord("SWR-PLGD", "I imagined");
  VerifyChord("SWR-EPLGD", "I was imagining");
  VerifyChord("SWR-FPLGD", "I had imagined");
  VerifyChord("KWHR-PLGD", "he imagined");

  VerifyChord("SWR-PLGT", "I imagine that");
  VerifyChord("SWR-EPLGT", "I am imagining that");
  VerifyChord("SWR-FPLGT", "I have imagined that");
  VerifyChord("KWHR-PLGT", "he imagines that");
  VerifyChord("SWR-PLGTD", "I imagined that");
  VerifyChord("SWR-EPLGTD", "I was imagining that");
  VerifyChord("SWR-FPLGTD", "I had imagined that");
  VerifyChord("KWHR-PLGTD", "he imagined that");

  VerifyChord("SWR-PBGS", "I keep");
  VerifyChord("SWR-EPBGS", "I am keeping");
  VerifyChord("SWR-FPBGS", "I have kept");
  VerifyChord("KWHR-PBGS", "he keeps");
  VerifyChord("SWR-PBGSZ", "I kept");
  VerifyChord("SWR-EPBGSZ", "I was keeping");
  VerifyChord("SWR-FPBGSZ", "I had kept");
  VerifyChord("KWHR-PBGSZ", "he kept");

  VerifyChord("SWR-PB", "I know");
  VerifyChord("SWR-EPB", "I am knowing");
  VerifyChord("SWR-FPB", "I have known");
  VerifyChord("KWHR-PB", "he knows");
  VerifyChord("SWR-PBD", "I knew");
  VerifyChord("SWR-EPBD", "I was knowing");
  VerifyChord("SWR-FPBD", "I had known");
  VerifyChord("KWHR-PBD", "he knew");

  VerifyChord("SWR-PBT", "I know that");
  VerifyChord("SWR-EPBT", "I am knowing that");
  VerifyChord("SWR-FPBT", "I have known that");
  VerifyChord("KWHR-PBT", "he knows that");
  VerifyChord("SWR-PBTD", "I knew that");
  VerifyChord("SWR-EPBTD", "I was knowing that");
  VerifyChord("SWR-FPBTD", "I had known that");
  VerifyChord("KWHR-PBTD", "he knew that");

  VerifyChord("SWR-RPBS", "I learn");
  VerifyChord("SWR-ERPBS", "I am learning");
  VerifyChord("SWR-FRPBS", "I have learnt");
  VerifyChord("KWHR-RPBS", "he learns");
  VerifyChord("SWR-RPBSZ", "I learnt");
  VerifyChord("SWR-ERPBSZ", "I was learning");
  VerifyChord("SWR-FRPBSZ", "I had learnt");
  VerifyChord("KWHR-RPBSZ", "he learnt");

  VerifyChord("SWR-RPBTS", "I learn to");
  VerifyChord("SWR-ERPBTS", "I am learning to");
  VerifyChord("SWR-FRPBTS", "I have learnt to");
  VerifyChord("KWHR-RPBTS", "he learns to");
  VerifyChord("SWR-RPBTSDZ", "I learnt to");
  VerifyChord("SWR-ERPBTSDZ", "I was learning to");
  VerifyChord("SWR-FRPBTSDZ", "I had learnt to");
  VerifyChord("KWHR-RPBTSDZ", "he learnt to");

  VerifyChord("SWR-LGZ", "I leave");
  VerifyChord("SWR-ELGZ", "I am leaving");
  VerifyChord("SWR-FLGZ", "I have left");
  VerifyChord("KWHR-LGZ", "he leaves");
  VerifyChord("SWR-LGDZ", "I left");
  VerifyChord("SWR-ELGDZ", "I was leaving");
  VerifyChord("SWR-FLGDZ", "I had left");
  VerifyChord("KWHR-LGDZ", "he left");

  VerifyChord("SWR-LS", "I let");
  VerifyChord("SWR-ELS", "I am letting");
  VerifyChord("SWR-FLS", "I have let");
  VerifyChord("KWHR-LS", "he lets");
  VerifyChord("SWR-LSZ", "I let");
  VerifyChord("SWR-ELSZ", "I was letting");
  VerifyChord("SWR-FLSZ", "I had let");
  VerifyChord("KWHR-LSZ", "he let");

  VerifyChord("SWR-BLG", "I like");
  VerifyChord("SWR-EBLG", "I am liking");
  VerifyChord("SWR-FBLG", "I have liked");
  VerifyChord("KWHR-BLG", "he likes");
  VerifyChord("SWR-BLGD", "I liked");
  VerifyChord("SWR-EBLGD", "I was liking");
  VerifyChord("SWR-FBLGD", "I had liked");
  VerifyChord("KWHR-BLGD", "he liked");

  VerifyChord("SWR-LZ", "I live");
  VerifyChord("SWR-ELZ", "I am living");
  VerifyChord("SWR-FLZ", "I have lived");
  VerifyChord("KWHR-LZ", "he lives");
  VerifyChord("SWR-LDZ", "I lived");
  VerifyChord("SWR-ELDZ", "I was living");
  VerifyChord("SWR-FLDZ", "I had lived");
  VerifyChord("KWHR-LDZ", "he lived");

  VerifyChord("SWR-L", "I look");
  VerifyChord("SWR-EL", "I am looking");
  VerifyChord("SWR-FL", "I have looked");
  VerifyChord("KWHR-L", "he looks");
  VerifyChord("SWR-LD", "I looked");
  VerifyChord("SWR-ELD", "I was looking");
  VerifyChord("SWR-FLD", "I had looked");
  VerifyChord("KWHR-LD", "he looked");

  VerifyChord("SWR-LG", "I love");
  VerifyChord("SWR-ELG", "I am loving");
  VerifyChord("SWR-FLG", "I have loved");
  VerifyChord("KWHR-LG", "he loves");
  VerifyChord("SWR-LGD", "I loved");
  VerifyChord("SWR-ELGD", "I was loving");
  VerifyChord("SWR-FLGD", "I had loved");
  VerifyChord("KWHR-LGD", "he loved");

  VerifyChord("SWR-LGT", "I love to");
  VerifyChord("SWR-ELGT", "I am loving to");
  VerifyChord("SWR-FLGT", "I have loved to");
  VerifyChord("KWHR-LGT", "he loves to");
  VerifyChord("SWR-LGTD", "I loved to");
  VerifyChord("SWR-ELGTD", "I was loving to");
  VerifyChord("SWR-FLGTD", "I had loved to");
  VerifyChord("KWHR-LGTD", "he loved to");

  VerifyChord("SWR-RPBL", "I make");
  VerifyChord("SWR-ERPBL", "I am making");
  VerifyChord("SWR-FRPBL", "I have made");
  VerifyChord("KWHR-RPBL", "he makes");
  VerifyChord("SWR-RPBLD", "I made");
  VerifyChord("SWR-ERPBLD", "I was making");
  VerifyChord("SWR-FRPBLD", "I had made");
  VerifyChord("KWHR-RPBLD", "he made");

  VerifyChord("SWR-RPBLT", "I make the");
  VerifyChord("SWR-ERPBLT", "I am making the");
  VerifyChord("SWR-FRPBLT", "I have made the");
  VerifyChord("KWHR-RPBLT", "he makes the");
  VerifyChord("SWR-RPBLTD", "I made the");
  VerifyChord("SWR-ERPBLTD", "I was making the");
  VerifyChord("SWR-FRPBLTD", "I had made the");
  VerifyChord("KWHR-RPBLTD", "he made the");

  VerifyChord("SWR-PBL", "I mean");
  VerifyChord("SWR-EPBL", "I am meaning");
  VerifyChord("SWR-FPBL", "I have meant");
  VerifyChord("KWHR-PBL", "he means");
  VerifyChord("SWR-PBLD", "I meant");
  VerifyChord("SWR-EPBLD", "I was meaning");
  VerifyChord("SWR-FPBLD", "I had meant");
  VerifyChord("KWHR-PBLD", "he meant");

  VerifyChord("SWR-PBLT", "I mean to");
  VerifyChord("SWR-EPBLT", "I am meaning to");
  VerifyChord("SWR-FPBLT", "I have meant to");
  VerifyChord("KWHR-PBLT", "he means to");
  VerifyChord("SWR-PBLTD", "I meant to");
  VerifyChord("SWR-EPBLTD", "I was meaning to");
  VerifyChord("SWR-FPBLTD", "I had meant to");
  VerifyChord("KWHR-PBLTD", "he meant to");

  VerifyChord("SWR-PLZ", "I move");
  VerifyChord("SWR-EPLZ", "I am moving");
  VerifyChord("SWR-FPLZ", "I have moved");
  VerifyChord("KWHR-PLZ", "he moves");
  VerifyChord("SWR-PLDZ", "I moved");
  VerifyChord("SWR-EPLDZ", "I was moving");
  VerifyChord("SWR-FPLDZ", "I had moved");
  VerifyChord("KWHR-PLDZ", "he moved");

  VerifyChord("SWR-RPG", "I need");
  VerifyChord("SWR-ERPG", "I am needing");
  VerifyChord("SWR-FRPG", "I have needed");
  VerifyChord("KWHR-RPG", "he needs");
  VerifyChord("SWR-RPGD", "I needed");
  VerifyChord("SWR-ERPGD", "I was needing");
  VerifyChord("SWR-FRPGD", "I had needed");
  VerifyChord("KWHR-RPGD", "he needed");

  VerifyChord("SWR-RPGT", "I need to");
  VerifyChord("SWR-ERPGT", "I am needing to");
  VerifyChord("SWR-FRPGT", "I have needed to");
  VerifyChord("KWHR-RPGT", "he needs to");
  VerifyChord("SWR-RPGTD", "I needed to");
  VerifyChord("SWR-ERPGTD", "I was needing to");
  VerifyChord("SWR-FRPGTD", "I had needed to");
  VerifyChord("KWHR-RPGTD", "he needed to");

  VerifyChord("SWR-PS", "I put");
  VerifyChord("SWR-EPS", "I am putting");
  VerifyChord("SWR-FPS", "I have put");
  VerifyChord("KWHR-PS", "he puts");
  VerifyChord("SWR-PSZ", "I put");
  VerifyChord("SWR-EPSZ", "I was putting");
  VerifyChord("SWR-FPSZ", "I had put");
  VerifyChord("KWHR-PSZ", "he put");

  VerifyChord("SWR-PST", "I put it");
  VerifyChord("SWR-EPST", "I am putting it");
  VerifyChord("SWR-FPST", "I have put it");
  VerifyChord("KWHR-PST", "he puts it");
  VerifyChord("SWR-PSTDZ", "I put it");
  VerifyChord("SWR-EPSTDZ", "I was putting it");
  VerifyChord("SWR-FPSTDZ", "I had put it");
  VerifyChord("KWHR-PSTDZ", "he put it");

  VerifyChord("SWR-RS", "I read");
  VerifyChord("SWR-ERS", "I am reading");
  VerifyChord("SWR-FRS", "I have read");
  VerifyChord("KWHR-RS", "he reads");
  VerifyChord("SWR-RSZ", "I read");
  VerifyChord("SWR-ERSZ", "I was reading");
  VerifyChord("SWR-FRSZ", "I had read");
  VerifyChord("KWHR-RSZ", "he read");

  VerifyChord("SWR-RL", "I recall");
  VerifyChord("SWR-ERL", "I am recalling");
  VerifyChord("SWR-FRL", "I have recalled");
  VerifyChord("KWHR-RL", "he recalls");
  VerifyChord("SWR-RLD", "I recalled");
  VerifyChord("SWR-ERLD", "I was recalling");
  VerifyChord("SWR-FRLD", "I had recalled");
  VerifyChord("KWHR-RLD", "he recalled");

  VerifyChord("SWR-RLS", "I realize");
  VerifyChord("SWR-ERLS", "I am realizing");
  VerifyChord("SWR-FRLS", "I have realized");
  VerifyChord("KWHR-RLS", "he realizes");
  VerifyChord("SWR-RLSZ", "I realized");
  VerifyChord("SWR-ERLSZ", "I was realizing");
  VerifyChord("SWR-FRLSZ", "I had realized");
  VerifyChord("KWHR-RLSZ", "he realized");

  VerifyChord("SWR-RLTS", "I realize that");
  VerifyChord("SWR-ERLTS", "I am realizing that");
  VerifyChord("SWR-FRLTS", "I have realized that");
  VerifyChord("KWHR-RLTS", "he realizes that");
  VerifyChord("SWR-RLTSDZ", "I realized that");
  VerifyChord("SWR-ERLTSDZ", "I was realizing that");
  VerifyChord("SWR-FRLTSDZ", "I had realized that");
  VerifyChord("KWHR-RLTSDZ", "he realized that");

  VerifyChord("SWR-RPL", "I remember");
  VerifyChord("SWR-ERPL", "I am remembering");
  VerifyChord("SWR-FRPL", "I have remembered");
  VerifyChord("KWHR-RPL", "he remembers");
  VerifyChord("SWR-RPLD", "I remembered");
  VerifyChord("SWR-ERPLD", "I was remembering");
  VerifyChord("SWR-FRPLD", "I had remembered");
  VerifyChord("KWHR-RPLD", "he remembered");

  VerifyChord("SWR-RPLT", "I remember that");
  VerifyChord("SWR-ERPLT", "I am remembering that");
  VerifyChord("SWR-FRPLT", "I have remembered that");
  VerifyChord("KWHR-RPLT", "he remembers that");
  VerifyChord("SWR-RPLTD", "I remembered that");
  VerifyChord("SWR-ERPLTD", "I was remembering that");
  VerifyChord("SWR-FRPLTD", "I had remembered that");
  VerifyChord("KWHR-RPLTD", "he remembered that");

  VerifyChord("SWR-RPLS", "I remain");
  VerifyChord("SWR-ERPLS", "I am remaining");
  VerifyChord("SWR-FRPLS", "I have remained");
  VerifyChord("KWHR-RPLS", "he remains");
  VerifyChord("SWR-RPLSZ", "I remained");
  VerifyChord("SWR-ERPLSZ", "I was remaining");
  VerifyChord("SWR-FRPLSZ", "I had remained");
  VerifyChord("KWHR-RPLSZ", "he remained");

  VerifyChord("SWR-R", "I run");
  VerifyChord("SWR-ER", "I am running");
  VerifyChord("SWR-FR", "I have run");
  VerifyChord("KWHR-R", "he runs");
  VerifyChord("SWR-RD", "I ran");
  VerifyChord("SWR-ERD", "I was running");
  VerifyChord("SWR-FRD", "I had run");
  VerifyChord("KWHR-RD", "he ran");

  VerifyChord("SWR-BS", "I say");
  VerifyChord("SWR-EBS", "I am saying");
  VerifyChord("SWR-FBS", "I have said");
  VerifyChord("KWHR-BS", "he says");
  VerifyChord("SWR-BSZ", "I said");
  VerifyChord("SWR-EBSZ", "I was saying");
  VerifyChord("SWR-FBSZ", "I had said");
  VerifyChord("KWHR-BSZ", "he said");

  VerifyChord("SWR-BTS", "I say that");
  VerifyChord("SWR-EBTS", "I am saying that");
  VerifyChord("SWR-FBTS", "I have said that");
  VerifyChord("KWHR-BTS", "he says that");
  VerifyChord("SWR-BTSDZ", "I said that");
  VerifyChord("SWR-EBTSDZ", "I was saying that");
  VerifyChord("SWR-FBTSDZ", "I had said that");
  VerifyChord("KWHR-BTSDZ", "he said that");

  VerifyChord("SWR-S", "I see");
  VerifyChord("SWR-ES", "I am seeing");
  VerifyChord("SWR-FS", "I have seen");
  VerifyChord("KWHR-S", "he sees");
  VerifyChord("SWR-SZ", "I saw");
  VerifyChord("SWR-ESZ", "I was seeing");
  VerifyChord("SWR-FSZ", "I had seen");
  VerifyChord("KWHR-SZ", "he saw");

  VerifyChord("SWR-BLS", "I set");
  VerifyChord("SWR-EBLS", "I am setting");
  VerifyChord("SWR-FBLS", "I have set");
  VerifyChord("KWHR-BLS", "he sets");
  VerifyChord("SWR-BLSZ", "I set");
  VerifyChord("SWR-EBLSZ", "I was setting");
  VerifyChord("SWR-FBLSZ", "I had set");
  VerifyChord("KWHR-BLSZ", "he set");

  VerifyChord("SWR-PLS", "I seem");
  VerifyChord("SWR-EPLS", "I am seeming");
  VerifyChord("SWR-FPLS", "I have seemed");
  VerifyChord("KWHR-PLS", "he seems");
  VerifyChord("SWR-PLSZ", "I seemed");
  VerifyChord("SWR-EPLSZ", "I was seeming");
  VerifyChord("SWR-FPLSZ", "I had seemed");
  VerifyChord("KWHR-PLSZ", "he seemed");

  VerifyChord("SWR-PLTS", "I seem to");
  VerifyChord("SWR-EPLTS", "I am seeming to");
  VerifyChord("SWR-FPLTS", "I have seemed to");
  VerifyChord("KWHR-PLTS", "he seems to");
  VerifyChord("SWR-PLTSDZ", "I seemed to");
  VerifyChord("SWR-EPLTSDZ", "I was seeming to");
  VerifyChord("SWR-FPLTSDZ", "I had seemed to");
  VerifyChord("KWHR-PLTSDZ", "he seemed to");

  VerifyChord("SWR-RBZ", "I show");
  VerifyChord("SWR-ERBZ", "I am showing");
  VerifyChord("SWR-FRBZ", "I have shown");
  VerifyChord("KWHR-RBZ", "he shows");
  VerifyChord("SWR-RBDZ", "I showed");
  VerifyChord("SWR-ERBDZ", "I was showing");
  VerifyChord("SWR-FRBDZ", "I had shown");
  VerifyChord("KWHR-RBDZ", "he showed");

  VerifyChord("SWR-RBT", "I take");
  VerifyChord("SWR-ERBT", "I am taking");
  VerifyChord("SWR-FRBT", "I have taken");
  VerifyChord("KWHR-RBT", "he takes");
  VerifyChord("SWR-RBTD", "I took");
  VerifyChord("SWR-ERBTD", "I was taking");
  VerifyChord("SWR-FRBTD", "I had taken");
  VerifyChord("KWHR-RBTD", "he took");

  // VerifyChord("SWR-BLGT", "I talk");
  // VerifyChord("SWR-EBLGT", "I am talking");
  // VerifyChord("SWR-FBLGT", "I have talked");
  // VerifyChord("KWHR-BLGT", "he talks");
  // VerifyChord("SWR-BLGTD", "I talked");
  // VerifyChord("SWR-EBLGTD", "I was talking");
  // VerifyChord("SWR-FBLGTD", "I had talked");
  // VerifyChord("KWHR-BLGTD", "he talked");

  VerifyChord("SWR-RLT", "I tell");
  VerifyChord("SWR-ERLT", "I am telling");
  VerifyChord("SWR-FRLT", "I have told");
  VerifyChord("KWHR-RLT", "he tells");
  VerifyChord("SWR-RLTD", "I told");
  VerifyChord("SWR-ERLTD", "I was telling");
  VerifyChord("SWR-FRLTD", "I had told");
  VerifyChord("KWHR-RLTD", "he told");

  VerifyChord("SWR-PBG", "I think");
  VerifyChord("SWR-EPBG", "I am thinking");
  VerifyChord("SWR-FPBG", "I have thought");
  VerifyChord("KWHR-PBG", "he thinks");
  VerifyChord("SWR-PBGD", "I thought");
  VerifyChord("SWR-EPBGD", "I was thinking");
  VerifyChord("SWR-FPBGD", "I had thought");
  VerifyChord("KWHR-PBGD", "he thought");

  VerifyChord("SWR-PBGT", "I think that");
  VerifyChord("SWR-EPBGT", "I am thinking that");
  VerifyChord("SWR-FPBGT", "I have thought that");
  VerifyChord("KWHR-PBGT", "he thinks that");
  VerifyChord("SWR-PBGTD", "I thought that");
  VerifyChord("SWR-EPBGTD", "I was thinking that");
  VerifyChord("SWR-FPBGTD", "I had thought that");
  VerifyChord("KWHR-PBGTD", "he thought that");

  VerifyChord("SWR-RT", "I try");
  VerifyChord("SWR-ERT", "I am trying");
  VerifyChord("SWR-FRT", "I have tried");
  VerifyChord("KWHR-RT", "he tries");
  VerifyChord("SWR-RTD", "I tried");
  VerifyChord("SWR-ERTD", "I was trying");
  VerifyChord("SWR-FRTD", "I had tried");
  VerifyChord("KWHR-RTD", "he tried");

  VerifyChord("SWR-RTS", "I try to");
  VerifyChord("SWR-ERTS", "I am trying to");
  VerifyChord("SWR-FRTS", "I have tried to");
  VerifyChord("KWHR-RTS", "he tries to");
  VerifyChord("SWR-RTSDZ", "I tried to");
  VerifyChord("SWR-ERTSDZ", "I was trying to");
  VerifyChord("SWR-FRTSDZ", "I had tried to");
  VerifyChord("KWHR-RTSDZ", "he tried to");

  VerifyChord("SWR-RPB", "I understand");
  VerifyChord("SWR-ERPB", "I am understanding");
  VerifyChord("SWR-FRPB", "I have understood");
  VerifyChord("KWHR-RPB", "he understands");
  VerifyChord("SWR-RPBD", "I understood");
  VerifyChord("SWR-ERPBD", "I was understanding");
  VerifyChord("SWR-FRPBD", "I had understood");
  VerifyChord("KWHR-RPBD", "he understood");

  VerifyChord("SWR-RPBT", "I understand the");
  VerifyChord("SWR-ERPBT", "I am understanding the");
  VerifyChord("SWR-FRPBT", "I have understood the");
  VerifyChord("KWHR-RPBT", "he understands the");
  VerifyChord("SWR-RPBTD", "I understood the");
  VerifyChord("SWR-ERPBTD", "I was understanding the");
  VerifyChord("SWR-FRPBTD", "I had understood the");
  VerifyChord("KWHR-RPBTD", "he understood the");

  VerifyChord("SWR-Z", "I use");
  VerifyChord("SWR-EZ", "I am using");
  VerifyChord("SWR-FZ", "I have used");
  VerifyChord("KWHR-Z", "he uses");
  VerifyChord("SWR-DZ", "I used");
  VerifyChord("SWR-EDZ", "I was using");
  VerifyChord("SWR-FDZ", "I had used");
  VerifyChord("KWHR-DZ", "he used");

  VerifyChord("SWR-P", "I want");
  VerifyChord("SWR-EP", "I am wanting");
  VerifyChord("SWR-FP", "I have wanted");
  VerifyChord("KWHR-P", "he wants");
  VerifyChord("SWR-PD", "I wanted");
  VerifyChord("SWR-EPD", "I was wanting");
  VerifyChord("SWR-FPD", "I had wanted");
  VerifyChord("KWHR-PD", "he wanted");

  VerifyChord("SWR-PT", "I want to");
  VerifyChord("SWR-EPT", "I am wanting to");
  VerifyChord("SWR-FPT", "I have wanted to");
  VerifyChord("KWHR-PT", "he wants to");
  VerifyChord("SWR-PTD", "I wanted to");
  VerifyChord("SWR-EPTD", "I was wanting to");
  VerifyChord("SWR-FPTD", "I had wanted to");
  VerifyChord("KWHR-PTD", "he wanted to");

  VerifyChord("SWR-RBS", "I wish");
  VerifyChord("SWR-ERBS", "I am wishing");
  VerifyChord("SWR-FRBS", "I have wished");
  VerifyChord("KWHR-RBS", "he wishes");
  VerifyChord("SWR-RBSZ", "I wished");
  VerifyChord("SWR-ERBSZ", "I was wishing");
  VerifyChord("SWR-FRBSZ", "I had wished");
  VerifyChord("KWHR-RBSZ", "he wished");

  VerifyChord("SWR-RBTS", "I wish to");
  VerifyChord("SWR-ERBTS", "I am wishing to");
  VerifyChord("SWR-FRBTS", "I have wished to");
  VerifyChord("KWHR-RBTS", "he wishes to");
  VerifyChord("SWR-RBTSDZ", "I wished to");
  VerifyChord("SWR-ERBTSDZ", "I was wishing to");
  VerifyChord("SWR-FRBTSDZ", "I had wished to");
  VerifyChord("KWHR-RBTSDZ", "he wished to");

  VerifyChord("SWR-RBG", "I work");
  VerifyChord("SWR-ERBG", "I am working");
  VerifyChord("SWR-FRBG", "I have worked");
  VerifyChord("KWHR-RBG", "he works");
  VerifyChord("SWR-RBGD", "I worked");
  VerifyChord("SWR-ERBGD", "I was working");
  VerifyChord("SWR-FRBGD", "I had worked");
  VerifyChord("KWHR-RBGD", "he worked");

  VerifyChord("SWR-RBGT", "I work on");
  VerifyChord("SWR-ERBGT", "I am working on");
  VerifyChord("SWR-FRBGT", "I have worked on");
  VerifyChord("KWHR-RBGT", "he works on");
  VerifyChord("SWR-RBGTD", "I worked on");
  VerifyChord("SWR-ERBGTD", "I was working on");
  VerifyChord("SWR-FRBGTD", "I had worked on");
  VerifyChord("KWHR-RBGTD", "he worked on");
  // spellchecker: enable
}
TEST_END

void VerifyReverseLookup(const char *text, StenoChord expected) {
  StenoReverseDictionaryLookup lookup(2, text);
  StenoJeffPhrasingDictionary::instance.ReverseLookup(lookup);
  assert(lookup.resultCount > 0);
  for (size_t i = 0; i < lookup.resultCount; ++i) {
    assert(lookup.resultLengths[i] == 1);
    if (lookup.chords[i] == expected) {
      return;
    }
  }
  assert(false);
}

TEST_BEGIN("JeffPhrasing: Single word reverse lookup") {
  // spellchecker: disable
  VerifyReverseLookup("to go to", StenoChord("STWRUGT"));
  VerifyReverseLookup("there are", StenoChord("STPHR-B"));
  VerifyReverseLookup("can I", StenoChord("SWRAU"));
  VerifyReverseLookup("I", StenoChord("SWR"));
  VerifyReverseLookup("I can", StenoChord("SWRA"));
  VerifyReverseLookup("I can't", StenoChord("SWRA*"));
  VerifyReverseLookup("I need to", StenoChord("SWR-RPGT"));
  VerifyReverseLookup("I am going to", StenoChord("SWREGT"));
  VerifyReverseLookup("if you go", StenoChord("STPAUG"));
  // spellchecker: enable
}
TEST_END

//---------------------------------------------------------------------------
