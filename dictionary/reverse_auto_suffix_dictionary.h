//---------------------------------------------------------------------------

#pragma once
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class Pattern;
class StenoCompiledOrthography;
class StenoOrthography;
struct StenoOrthographyReverseAutoSuffix;

//---------------------------------------------------------------------------

class StenoReverseAutoSuffixDictionary final : public StenoWrappedDictionary {
public:
  StenoReverseAutoSuffixDictionary(StenoDictionary *dictionary,
                                   const StenoCompiledOrthography &orthography);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  virtual const char *GetName() const;

private:
  const StenoCompiledOrthography &orthography;
  const Pattern *reversePatterns;

  void ProcessReverseAutoSuffix(
      StenoReverseDictionaryLookup &result,
      const StenoOrthographyReverseAutoSuffix &reverseAutoSuffix,
      const Pattern &reversePattern) const;

  bool CanAutoSuffixLookup(const StenoChord *chords, size_t length) const;
  bool HasPrefixLookup(const StenoChord *chords, size_t length) const;
  bool HasSuffixLookup(const StenoChord *chords, size_t length) const;

  static const Pattern *
  CreateReversePatterns(const StenoOrthography &orthography);
};

//---------------------------------------------------------------------------
