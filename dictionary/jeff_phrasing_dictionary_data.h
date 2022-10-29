//---------------------------------------------------------------------------

#include "../chord.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

enum class VerbForm {
  UNSPECIFIED = 0,
  FIRST_PERSON_SINGULAR,
  SECOND_PERSON,
  THIRD_PERSON_SINGULAR,
  FIRST_PERSON_PLURAL,
  THIRD_PERSON_PLURAL,
  ROOT,
  PRESENT_PARTICIPLE,
  PAST_PARTICIPLE,
};

enum class Tense {
  // These values should have no overlap with VerbForm
  PRESENT = 0x8000,
  PAST,
};

struct JeffPhrasingValidEnders {
  size_t enderCount;
  const StenoChord *enders;
};

struct JeffPhrasingPronoun {
  const char *word;
  VerbForm verbForm;
  bool canUseAllEnders;
};

struct JeffPhrasingFullStarter {
  StenoChord chord;
  JeffPhrasingPronoun pronoun;
};

struct JeffPhrasingMap;
template <size_t N> struct JeffPhrasingMapData;

struct JeffPhrasingVariant {
  enum class Type {
    UNKNOWN,
    TEXT,
    MAP,
  };

  constexpr JeffPhrasingVariant() : type(Type::UNKNOWN), text(nullptr) {}
  constexpr JeffPhrasingVariant(const char *text)
      : type(Type::TEXT), text(text) {}
  constexpr JeffPhrasingVariant(const JeffPhrasingMap *map)
      : type(Type::MAP), map(map) {}

  template <size_t N>
  constexpr JeffPhrasingVariant(const JeffPhrasingMapData<N> *mapData)
      : type(Type::MAP), mapData(mapData) {}

  const JeffPhrasingVariant *Lookup(uint32_t key) const;

  const JeffPhrasingVariant *LookupWithDefaultOrSelf(uint32_t key) const;

  const char *ToString() const {
    assert(type == Type::TEXT);
    return text;
  }

  Type type;

  union {
    const char *text;
    const JeffPhrasingMap *map;
    const void *mapData;
  };
};

struct JeffPhrasingMapEntry {
  uint32_t key;
  const JeffPhrasingVariant value;
};

struct JeffPhrasingMap {
  size_t entryCount;
  const JeffPhrasingMapEntry entries[];

  const JeffPhrasingVariant *Lookup(uint32_t key) const;
};

template <size_t N> struct JeffPhrasingMapData {
  size_t entryCount = N;
  const JeffPhrasingMapEntry entries[N];
};

struct JeffPhrasingMiddle {
  VerbForm verbForm;
  JeffPhrasingVariant word;
};

struct JeffPhrasingSimpleStarter {
  StenoChord chord;
  JeffPhrasingMiddle middle;
};

struct JeffPhrasingStructure {
  JeffPhrasingVariant format;
  bool useMiddleVerbForm;
  VerbForm updatedVerbForm;
};

struct JeffPhrasingEnder {
  StenoChord chord;
  Tense tense;
  bool canUseAllStarters;
  JeffPhrasingVariant ender;
  const char *suffix;
};

struct JeffPhrasingStructureException {
  StenoChord chord;
  JeffPhrasingStructure structure;
};

struct JeffPhrasingDictionaryData {
  size_t simpleStarterCount;
  const JeffPhrasingSimpleStarter *simpleStarters;

  const JeffPhrasingPronoun *simplePronouns;
  const JeffPhrasingStructure *simpleStructures;

  size_t fullStarterCount;
  const JeffPhrasingFullStarter *fullStarters;

  const JeffPhrasingMiddle *fullMiddles;
  const JeffPhrasingStructure *fullStructures;

  size_t structureExceptionCount;
  const JeffPhrasingStructureException *structureExceptions;

  size_t endersCount;
  const JeffPhrasingEnder *enders;

  size_t nonPhraseStrokeCount;
  const StenoChord *nonPhraseStrokes;

  size_t uniqueStarterCount;
  const JeffPhrasingStructureException *uniqueStarters;

  size_t enderHashMapSize;
  const JeffPhrasingEnder *const *enderHashMap;

  static const JeffPhrasingDictionaryData instance;

  bool IsValidPhraseChord(StenoChord chord) const;
  const JeffPhrasingMiddle *LookupSimpleStarter(StenoChord chord) const;
  const JeffPhrasingMiddle *LookupFullMiddle(StenoChord chord) const;
  const JeffPhrasingPronoun *LookupFullStarter(StenoChord chord) const;
  const JeffPhrasingEnder *LookupEnder(StenoChord chord) const;
  const JeffPhrasingStructure *LookupUniqueStarter(StenoChord chord) const;
  const JeffPhrasingStructure *LookupStructureException(StenoChord chord) const;
};

//---------------------------------------------------------------------------
