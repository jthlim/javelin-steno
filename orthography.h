//---------------------------------------------------------------------------

#pragma once
#include "chord.h"
#include "list.h"
#include "pattern.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

struct StenoOrthographyRule {
  const char *testPattern;
  const char *replacement;
};

struct StenoOrthographyAlias {
  const char *text;
  const char *alias;
};

struct StenoOrthographyAutoSuffix {
  StenoChord chord;
  const char *text;
};

struct StenoOrthographyReverseAutoSuffix {
  StenoOrthographyAutoSuffix *autoSuffix;
  StenoChord suppressMask;
  const char *testPattern;
  const char *replacement;
};

//---------------------------------------------------------------------------

struct StenoOrthography {
  size_t ruleCount;
  const StenoOrthographyRule *rules;

  size_t aliasCount;
  const StenoOrthographyAlias *aliases;

  StenoChord autoSuffixMask;
  size_t autoSuffixCount;
  const StenoOrthographyAutoSuffix *autoSuffixes;

  size_t reverseAutoSuffixCount;
  const StenoOrthographyReverseAutoSuffix *reverseAutoSuffixes;

  static const StenoOrthography emptyOrthography;

  void Print() const;
};

//---------------------------------------------------------------------------

class StenoCompiledOrthography {
public:
  StenoCompiledOrthography(const StenoOrthography &orthography);

  char *AddSuffix(const char *word, const char *suffix) const;

  void PrintInfo() const;

  const StenoOrthography &orthography;

private:
  struct SuffixEntry;

  const Pattern *patterns;

  void AddCandidates(List<SuffixEntry> &candidates, const char *word,
                     const char *suffix) const;

  static const Pattern *CreatePatterns(const StenoOrthography &orthography);
};

//---------------------------------------------------------------------------
