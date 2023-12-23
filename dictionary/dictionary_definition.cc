//---------------------------------------------------------------------------

#include "dictionary_definition.h"
#include "compact_map_dictionary.h"
#include "corrupted_dictionary.h"
#include "dictionary_list.h"
#include "emily_symbols_dictionary.h"
#include "full_map_dictionary.h"
#include "jeff_numbers_dictionary.h"
#include "jeff_phrasing_dictionary.h"
#include "jeff_show_stroke_dictionary.h"

//---------------------------------------------------------------------------

StenoDictionary *StenoDictionaryDefinition::Create() const {
#if defined(JAVELIN_PLATFORM_NRF5_SDK)
  // Avoid XIP anomaly 216.
  asm volatile("dsb");
#endif

  switch (type) {
  case StenoDictionaryType::COMPACT_MAP:
    return new StenoCompactMapDictionary(*this);

  case StenoDictionaryType::FULL_MAP:
#if defined(JAVELIN_PLATFORM_NRF5_SDK)
    return new StenoFullMapDictionary(*this);
#else
    return nullptr;
#endif

  case StenoDictionaryType::JEFF_SHOW_STROKE:
    return &StenoJeffShowStrokeDictionary::instance;

  case StenoDictionaryType::JEFF_NUMBERS:
    return &StenoJeffNumbersDictionary::instance;

  case StenoDictionaryType::JEFF_PHRASING:
    return &StenoJeffPhrasingDictionary::instance;

  case StenoDictionaryType::EMILY_SYMBOLS:
    return &StenoEmilySymbolsDictionary::instance;
  }

  return nullptr;
}

//---------------------------------------------------------------------------

void StenoDictionaryCollection::AddDictionariesToList(
    List<StenoDictionaryListEntry> &list) const {
  if (magic != STENO_MAP_DICTIONARY_COLLECTION_MAGIC) {
    list.Add(
        StenoDictionaryListEntry(&StenoCorruptedDictionary::instance, true));
    return;
  }

  for (size_t i = 0; i < dictionaryCount; ++i) {
    const StenoDictionaryDefinition *definition = dictionaries[i];

    StenoDictionary *dictionary = definition->Create();
    if (dictionary) {
      list.Add(
          StenoDictionaryListEntry(dictionary, definition->defaultEnabled));
    }
  }
}

//---------------------------------------------------------------------------