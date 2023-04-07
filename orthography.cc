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
    Console::WriteAsJson(rules[i].testPattern, buffer);
    Console::Printf("\",\n\t\t\t\"replacement\": \"");
    Console::WriteAsJson(rules[i].replacement, buffer);
    Console::Printf("\"\n\t\t}");
  }
  Console::Printf("\n\t],");
  Console::Printf("\n\t\"aliases\": [");
  for (size_t i = 0; i < aliasCount; ++i) {
    if (i != 0) {
      Console::Write(",", 1);
    }
    Console::Printf("\n\t\t{\n\t\t\t\"suffix\": \"");
    Console::WriteAsJson(aliases[i].text, buffer);
    Console::Printf("\",\n\t\t\t\"alias\": \"");
    Console::WriteAsJson(aliases[i].alias, buffer);
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
    Console::WriteAsJson(autoSuffixes[i].text + 1, buffer);
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
    Console::WriteAsJson(reverseAutoSuffixes[i].testPattern, buffer);
    Console::Printf("\",\n\t\t\t\"replacement\": \"");
    Console::WriteAsJson(reverseAutoSuffixes[i].replacement, buffer);
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

  char *simple = Str::Join(word, suffix, nullptr);

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
  for (SuffixEntry &entry : candidates) {
    free(entry.text);
  }

  char *text = Str::Join(word, " ^", suffix, nullptr);
  for (size_t i = 0; i < data.ruleCount; ++i) {
    const PatternMatch &match = patterns[i].Match(text);
    if (!match.match) {
      continue;
    }

    char *candidate = match.Replace(data.rules[i].replacement);
    free(text);
    return candidate;
  }

  free(text);
  return Str::Join(word, suffix, nullptr);
}

void StenoCompiledOrthography::AddCandidates(List<SuffixEntry> &candidates,
                                             const char *word,
                                             const char *suffix) const {
  const size_t MAXIMUM_PREFIX_LENGTH = 8;
  size_t wordLength = strlen(word);
  size_t offset = wordLength > MAXIMUM_PREFIX_LENGTH
                      ? wordLength - MAXIMUM_PREFIX_LENGTH
                      : 0;
  char *text = Str::Join(word + offset, " ^", suffix, nullptr);

  PatternQuickReject inputQuickReject(text);

  for (size_t i = 0; i < data.ruleCount; ++i) {
    const Pattern &pattern = patterns[i];
    if (!pattern.IsPossibleMatch(inputQuickReject)) {
      continue;
    }

    const PatternMatch match = pattern.MatchBypassingQuickReject(text);
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
  Console::Printf("      Auto-suffixes: %zu\n", data.autoSuffixCount);
  Console::Printf("      Reverse auto-suffixes: %zu\n",
                  data.reverseAutoSuffixCount);
}

//---------------------------------------------------------------------------
