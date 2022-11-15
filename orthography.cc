//---------------------------------------------------------------------------

#include "orthography.h"
#include "console.h"
#include "str.h"
#include "word_list.h"

//---------------------------------------------------------------------------

constexpr StenoOrthography StenoOrthography::emptyOrthography = {
    .ruleCount = 0,
    .rules = nullptr,
    .aliasCount = 0,
    .aliases = nullptr,
    .autoSuffixCount = 0,
    .autoSuffixes = nullptr,
    .reverseAutoSuffixCount = 0,
    .reverseAutoSuffixes = nullptr,
};

//---------------------------------------------------------------------------

struct StenoCompiledOrthography::SuffixEntry {
  SuffixEntry();
  SuffixEntry(char *text, int score) : text(text), score(score) {}

  char *text;
  int score;
};

//---------------------------------------------------------------------------

void StenoOrthography::Print() const {
  char buffer[256];

  Console::Printf("{\n");
  Console::Printf("\t\"rules\": [");
  for (size_t i = 0; i < ruleCount; ++i) {
    if (i != 0) {
      Console::Write(",", 1);
    }
    Console::Printf("\n\t\t{\n\t\t\t\"pattern\": \"");
    char *p = Str::WriteJson(buffer, rules[i].testPattern);
    Console::Write(buffer, p - buffer);
    Console::Printf("\",\n\t\t\t\"replacement\": \"");
    p = Str::WriteJson(buffer, rules[i].replacement);
    Console::Write(buffer, p - buffer);
    Console::Printf("\"\n\t\t}");
  }
  Console::Printf("\n\t],");
  Console::Printf("\n\t\"aliases\": [");
  for (size_t i = 0; i < aliasCount; ++i) {
    if (i != 0) {
      Console::Write(",", 1);
    }
    Console::Printf("\n\t\t{\n\t\t\t\"suffix\": \"");
    char *p = Str::WriteJson(buffer, aliases[i].text);
    Console::Write(buffer, p - buffer);
    Console::Printf("\",\n\t\t\t\"alias\": \"");
    p = Str::WriteJson(buffer, aliases[i].alias);
    Console::Write(buffer, p - buffer);
    Console::Printf("\"\n\t\t}");
  }
  Console::Printf("\n\t]\n}\n\n");
}

//---------------------------------------------------------------------------

StenoCompiledOrthography::StenoCompiledOrthography(
    const StenoOrthography &orthography)
    : orthography(orthography), patterns(CreatePatterns(orthography)) {}

const Pattern *
StenoCompiledOrthography::CreatePatterns(const StenoOrthography &orthography) {
  Pattern *patterns =
      (Pattern *)malloc(sizeof(Pattern) * orthography.ruleCount);
  for (size_t i = 0; i < orthography.ruleCount; ++i) {
    patterns[i] = Pattern::Compile(orthography.rules[i].testPattern);
  }
  return patterns;
}

char *StenoCompiledOrthography::AddSuffix(const char *word,
                                          const char *suffix) const {
  List<SuffixEntry> candidates;

  for (size_t i = 0; i < orthography.aliasCount; ++i) {
    if (Str::Eq(word, orthography.aliases[i].text)) {
      AddCandidates(candidates, word, orthography.aliases[i].alias);
    }
  }

  char *simple = Str::Asprintf("%s%s", word, suffix);
  int score = WordList::GetWordRank(simple);
  if (score >= 0) {
    candidates.Add(SuffixEntry(simple, score));
  } else {
    free(simple);
  }

  AddCandidates(candidates, word, suffix);

  if (candidates.IsNotEmpty()) {
    candidates.Sort([](const void *pa, const void *pb) -> int {
      const SuffixEntry *a = (const SuffixEntry *)pa;
      const SuffixEntry *b = (const SuffixEntry *)pb;
      if (a->score != b->score) {
        return a->score - b->score;
      }
      return (int)(a - b);
    });
    for (size_t i = 1; i < candidates.GetCount(); ++i) {
      free(candidates[i].text);
    }
    return candidates[0].text;
  }
  for (size_t i = 0; i < candidates.GetCount(); ++i) {
    free(candidates[i].text);
  }

  char *text = Str::Asprintf("%s ^%s", word, suffix);
  for (size_t i = 0; i < orthography.ruleCount; ++i) {
    const PatternMatch match = patterns[i].Match(text);
    if (!match.match) {
      continue;
    }

    char *candidate = match.Replace(orthography.rules[i].replacement);
    free(text);
    return candidate;
  }

  free(text);
  return Str::Asprintf("%s%s", word, suffix);
}

void StenoCompiledOrthography::AddCandidates(List<SuffixEntry> &candidates,
                                             const char *word,
                                             const char *suffix) const {
  char *text = Str::Asprintf("%s ^%s", word, suffix);

  for (size_t i = 0; i < orthography.ruleCount; ++i) {
    const PatternMatch match = patterns[i].Match(text);
    if (!match.match) {
      continue;
    }

    char *candidate = match.Replace(orthography.rules[i].replacement);
    int score = WordList::GetWordRank(candidate);
    if (score < 0) {
      free(candidate);
      continue;
    }
    candidates.Add(SuffixEntry(candidate, score));
  }
  free(text);
}

//---------------------------------------------------------------------------

void StenoCompiledOrthography::PrintInfo() const {
  Console::Printf("    Orthography\n");
  Console::Printf("      Rules: %zu\n", orthography.ruleCount);
  Console::Printf("      Aliases: %zu\n", orthography.aliasCount);
}

//---------------------------------------------------------------------------
