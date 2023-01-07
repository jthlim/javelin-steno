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
  Console::Printf("\n\t],");
  Console::Printf("\n\t\"auto-suffix\": [");
  for (size_t i = 0; i < autoSuffixCount; ++i) {
    if (i != 0) {
      Console::Write(",", 1);
    }
    Console::Printf("\n\t\t{\n\t\t\t\"key\": \"");
    char *p = autoSuffixes[i].stroke.ToString(buffer);
    Console::Write(buffer, p - buffer);
    Console::Printf("\",\n\t\t\t\"suffix\": \"");
    p = Str::WriteJson(buffer, autoSuffixes[i].text + 1);
    Console::Write(buffer, p - buffer);
    Console::Printf("\"\n\t\t}");
  }
  Console::Printf("\n\t],");
  Console::Printf("\n\t\"reverse-auto-suffix\": [");
  for (size_t i = 0; i < reverseAutoSuffixCount; ++i) {
    if (i != 0) {
      Console::Write(",", 1);
    }
    Console::Printf("\n\t\t{\n\t\t\t\"key\": \"");
    char *p = reverseAutoSuffixes[i].autoSuffix->stroke.ToString(buffer);
    Console::Write(buffer, p - buffer);
    Console::Printf("\",\n\t\t\t\"suppressMask\": \"");
    p = reverseAutoSuffixes[i].suppressMask.ToString(buffer);
    Console::Write(buffer, p - buffer);
    Console::Printf("\",\n\t\t\t\"pattern\": \"");
    p = Str::WriteJson(buffer, reverseAutoSuffixes[i].testPattern);
    Console::Write(buffer, p - buffer);
    Console::Printf("\",\n\t\t\t\"replacement\": \"");
    p = Str::WriteJson(buffer, reverseAutoSuffixes[i].replacement);
    Console::Write(buffer, p - buffer);
    Console::Printf("\"\n\t\t}");
  }
  Console::Printf("\n\t]\n}\n\n");
}

//---------------------------------------------------------------------------

StenoCompiledOrthography::StenoCompiledOrthography(
    const StenoOrthography &orthography)
    : data(orthography), patterns(CreatePatterns(orthography)) {}

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

  for (size_t i = 0; i < data.aliasCount; ++i) {
    if (Str::Eq(word, data.aliases[i].text)) {
      AddCandidates(candidates, word, data.aliases[i].alias);
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
  for (size_t i = 0; i < data.ruleCount; ++i) {
    const PatternMatch match = patterns[i].Match(text);
    if (!match.match) {
      continue;
    }

    char *candidate = match.Replace(data.rules[i].replacement);
    free(text);
    return candidate;
  }

  free(text);
  return Str::Asprintf("%s%s", word, suffix);
}

void StenoCompiledOrthography::AddCandidates(List<SuffixEntry> &candidates,
                                             const char *word,
                                             const char *suffix) const {
  const size_t MAXIMUM_PREFIX_LENGTH = 8;
  size_t wordLength = strlen(word);
  size_t offset = wordLength > MAXIMUM_PREFIX_LENGTH
                      ? wordLength - MAXIMUM_PREFIX_LENGTH
                      : 0;
  char *text = Str::Asprintf("%s ^%s", word + offset, suffix);

  for (size_t i = 0; i < data.ruleCount; ++i) {
    const PatternMatch match = patterns[i].Match(text);
    if (!match.match) {
      continue;
    }

    char *candidate = match.Replace(data.rules[i].replacement);
    if (offset != 0) {
      char *fullCandidate = (char *)malloc(wordLength + strlen(candidate));
      memcpy(fullCandidate, word, offset);
      strcpy(fullCandidate + offset, candidate);
      free(candidate);
      candidate = fullCandidate;
    }
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
  Console::Printf("      Rules: %zu\n", data.ruleCount);
  Console::Printf("      Aliases: %zu\n", data.aliasCount);
}

//---------------------------------------------------------------------------
