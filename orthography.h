//---------------------------------------------------------------------------

#pragma once
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

//---------------------------------------------------------------------------

struct StenoOrthography {
  size_t ruleCount;
  const StenoOrthographyRule *rules;

  size_t aliasCount;
  const StenoOrthographyAlias *aliases;

  static const StenoOrthography emptyOrthography;
};

//---------------------------------------------------------------------------
