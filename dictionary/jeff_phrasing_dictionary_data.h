//---------------------------------------------------------------------------

#pragma once
#include "../container/sized_list.h"
#include "../container/static_list.h"
#include "../stroke.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

// Bits:
//  1 = First Person
//  2 = Second Person
//  4 = Third Person
//  8 = Singular
//  0x10 = Plural
//  0x20 = Infinitive
//  0x40 = Participle
//  0x80 = Past
//  0x100 = Present
//  0x200 = Blank pronoun
enum class WordForm : uint16_t {
  UNSPECIFIED = 0,
  ROOT = 0x20,
  PAST = 0x80,
  PRESENT = 0x100,
  BLANK_PRONOUN = 0x200,

  FIRST_PERSON_SINGULAR = 9,
  SECOND_PERSON = 0xa,
  THIRD_PERSON_SINGULAR = 0xc,
  FIRST_PERSON_PLURAL = 0x11,
  THIRD_PERSON_PLURAL = 0x14,
  PAST_PARTICIPLE = 0xc0,
  PRESENT_PARTICIPLE = 0x140,
  THIRD_PERSON_SINGULAR_BLANK_PRONOUN = 0x20c,
  THIRD_PERSON_PLURAL_BLANK_PRONOUN = 0x214,
};

struct JeffPhrasingValidEnders : public StaticList<StenoStroke> {};

struct JeffPhrasingPronoun {
  const char *word;
  WordForm wordForm;
  bool canUseAllEnders;
};

struct JeffPhrasingFullStarter {
  StenoStroke stroke;
  JeffPhrasingPronoun pronoun;
};

struct JeffPhrasingMap;
template <size_t N> struct JeffPhrasingMapData;

struct JeffPhrasingVariant {
  enum class Type : uint8_t {
    UNKNOWN,
    TEXT,
    MAP,
  };

  consteval JeffPhrasingVariant() : type(Type::UNKNOWN), text(nullptr) {}
  consteval JeffPhrasingVariant(const char *text,
                                WordForm wordForm = WordForm::UNSPECIFIED)
      : type(Type::TEXT), wordForm(wordForm), text(text) {}
  consteval JeffPhrasingVariant(const JeffPhrasingMap *map,
                                WordForm wordForm = WordForm::UNSPECIFIED)
      : type(Type::MAP), wordForm(wordForm), map(map) {}

  template <size_t N>
  consteval JeffPhrasingVariant(const JeffPhrasingMapData<N> *mapData,
                                WordForm wordForm = WordForm::UNSPECIFIED)
      : type(Type::MAP), wordForm(wordForm), mapData(mapData) {}

  const JeffPhrasingVariant *Lookup(uint32_t key) const;
  const JeffPhrasingVariant *LookupWithDefaultOrSelf(uint32_t key) const;

  const char *ToString() const {
    assert(type == Type::TEXT);
    return text;
  }

  Type type;

  // Used by middles to override wordForm
  WordForm wordForm;

  union {
    const char *text;
    const JeffPhrasingMap *map;
    const void *mapData;
  };
};

struct JeffPhrasingMapEntry {
  consteval JeffPhrasingMapEntry(WordForm wordForm,
                                 const JeffPhrasingVariant value)
      : key(uint32_t(wordForm)), value(value) {}
  consteval JeffPhrasingMapEntry(uint32_t key, const JeffPhrasingVariant value)
      : key(key), value(value) {}

  uint32_t key;
  const JeffPhrasingVariant value;
};

struct JeffPhrasingMap {
  StaticList<const JeffPhrasingMapEntry> entries;

  const JeffPhrasingVariant *Lookup(uint32_t key) const;
};

template <size_t N> struct JeffPhrasingMapData {
  size_t entryCount = N;
  const JeffPhrasingMapEntry entries[N];
};

struct JeffPhrasingMiddle {
  JeffPhrasingVariant word;
};

struct JeffPhrasingSimpleStarter {
  StenoStroke stroke;
  JeffPhrasingMiddle middle;
};

struct JeffPhrasingStructure {
  JeffPhrasingVariant format;
  bool useMiddleWordForm;
  WordForm updatedWordForm;
};

struct JeffPhrasingEnder {
  StenoStroke stroke;
  WordForm tense;
  bool canUseAllStarters;
  JeffPhrasingVariant ender;
  const char *suffix;
};

struct JeffPhrasingStructureException {
  StenoStroke stroke;
  JeffPhrasingStructure structure;
};

struct ComponentMask {
  static constexpr uint8_t STARTER = 1;
  static constexpr uint8_t MIDDLE = 2;
  static constexpr uint8_t STRUCTURE = 4;
  static constexpr uint8_t VERB = 8;
};

struct ModeMask {
  static constexpr uint8_t FULL = 1;
  static constexpr uint8_t SIMPLE = 2;
  static constexpr uint8_t PRESENT = 4;
  static constexpr uint8_t PAST = 8;
};

struct JeffPhrasingReverseHashMapEntry {
  uint32_t hash;
  uint32_t replaceHash;
  StenoStroke stroke;
  bool checkNext;

  // 1 = starter, 2 = middle, 4 structure, 8 verb.
  uint8_t componentMask;

  // 1 = full, 2 = simple, 4 = present, 8 = past.
  uint8_t modeMask;
};

struct JeffPhrasingReverseStructureEntry {
  uint32_t hash;
  StenoStroke stroke;
  uint32_t modeMask; // 1 = full, 2 = simple.
};

struct JeffPhrasingDictionaryData {
  SizedList<JeffPhrasingSimpleStarter> simpleStarters;

  const JeffPhrasingPronoun *simplePronouns;
  const JeffPhrasingStructure *simpleStructures;

  SizedList<JeffPhrasingFullStarter> fullStarters;

  const JeffPhrasingMiddle *fullMiddles;
  const JeffPhrasingStructure *fullStructures;

  SizedList<JeffPhrasingStructureException> structureExceptions;
  SizedList<JeffPhrasingEnder> enders;
  SizedList<StenoStroke> nonPhraseStrokes;
  SizedList<JeffPhrasingStructureException> uniqueStarters;

  size_t enderHashMapSize;
  const JeffPhrasingEnder *const *enderHashMap;

  size_t reverseHashMapSize;
  const JeffPhrasingReverseHashMapEntry *const *reverseHashMap;

  const JeffPhrasingReverseHashMapEntry *reverseEntries;

  size_t reverseStructureHashMapSize;
  const JeffPhrasingReverseStructureEntry *const *reverseStructureHashMap;

  static const JeffPhrasingDictionaryData instance;

  bool IsValidPhraseStroke(StenoStroke stroke) const;
  const JeffPhrasingMiddle *LookupSimpleStarter(StenoStroke stroke) const;
  const JeffPhrasingMiddle *LookupFullMiddle(StenoStroke stroke) const;
  const JeffPhrasingPronoun *LookupFullStarter(StenoStroke stroke) const;
  const JeffPhrasingEnder *LookupEnder(StenoStroke stroke) const;
  const JeffPhrasingStructure *LookupUniqueStarter(StenoStroke stroke) const;
  const JeffPhrasingStructure *
  LookupStructureException(StenoStroke stroke) const;
  const JeffPhrasingReverseHashMapEntry *LookupReverseWord(uint32_t hash) const;
  const JeffPhrasingReverseStructureEntry *
  LookupReverseStructure(uint32_t hash) const;
};

//---------------------------------------------------------------------------
