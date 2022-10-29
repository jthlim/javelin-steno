//---------------------------------------------------------------------------

#include "dictionary_list.h"

//---------------------------------------------------------------------------

StenoDictionaryLookup StenoDictionaryList::Lookup(const StenoChord *chords,
                                                  size_t length) const {

  for (size_t i = 0; i < count; ++i) {
    const StenoDictionary *dictionary = dictionaries[i];
    if (dictionary->GetMaximumMatchLength() < length) {
      continue;
    }

    StenoDictionaryLookup lookup = dictionary->Lookup(chords, length);
    if (lookup.IsValid()) {
      return lookup;
    }
  }
  return StenoDictionaryLookup::CreateInvalid();
}

unsigned int StenoDictionaryList::GetMaximumMatchLength() const {
  unsigned int max = 0;
  for (size_t i = 0; i < count; ++i) {
    unsigned int m = dictionaries[i]->GetMaximumMatchLength();
    if (m > max) {
      max = m;
    }
  }
  return max;
}

void StenoDictionaryList::PrintInfo() const {
  for (size_t i = 0; i < count; ++i) {
    dictionaries[i]->PrintInfo();
  }
}

bool StenoDictionaryList::PrintDictionary(bool hasData) const {
  for (size_t i = 0; i < count; ++i) {
    if (dictionaries[i]->PrintDictionary(hasData)) {
      hasData = true;
    }
  }
  return hasData;
}

//---------------------------------------------------------------------------
