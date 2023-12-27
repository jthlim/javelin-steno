//---------------------------------------------------------------------------

#include "orthography.h"
#include "console.h"
#include "crc.h"
#include "str.h"
#include "word_list.h"

//---------------------------------------------------------------------------

#if USE_ORTHOGRAPHY_CACHE

#define RECORD_ORTHOGRAPHY_CACHE_STATS 0

#if RUN_TESTS

void StenoCompiledOrthography::LockCache() {}
void StenoCompiledOrthography::UnlockCache() {}

#endif

StenoCompiledOrthography::CacheEntry::CacheEntry(const char *word,
                                                 const char *suffix,
                                                 const char *result)
    : word(Str::Dup(word)), suffix(Str::Dup(suffix)), result(Str::Dup(result)) {
}

StenoCompiledOrthography::CacheEntry::~CacheEntry() {
  free(word);
  free(suffix);
  free(result);
}

bool StenoCompiledOrthography::CacheEntry::IsEqual(const char *word,
                                                   const char *suffix) const {
  return Str::Eq(word, this->word) && Str::Eq(suffix, this->suffix);
}

char *StenoCompiledOrthography::CacheBlock::Lookup(const char *word,
                                                   const char *suffix) const {
  LockCache();
  for (size_t i = 0; i < CACHE_ASSOCIATIVITY; ++i) {
    const CacheEntry *entry = entries[i];
    if (entry && entry->IsEqual(word, suffix)) {
      char *result = Str::Dup(entry->result);
      UnlockCache();
      return result;
    }
  }

  UnlockCache();
  return nullptr;
}

void StenoCompiledOrthography::CacheBlock::AddEntry(CacheEntry *entry) {
  LockCache();

  size_t entryIndex = index++ & (CACHE_ASSOCIATIVITY - 1);
  CacheEntry *oldEntry = entries[entryIndex];
  entries[entryIndex] = entry;

  UnlockCache();

  delete oldEntry;
}

#endif

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
  Console::Printf("{"
                  "\n\t\"rules\": [");
  for (size_t i = 0; i < ruleCount; ++i) {
    if (i != 0) {
      Console::Printf(",");
    }
    Console::Printf("\n\t\t{"
                    "\n\t\t\t\"pattern\": \"%J\","
                    "\n\t\t\t\"replacement\": \"%J\""
                    "\n\t\t}",
                    rules[i].testPattern, rules[i].replacement);
  }
  Console::Printf("\n\t],"
                  "\n\t\"aliases\": [");
  for (size_t i = 0; i < aliasCount; ++i) {
    if (i != 0) {
      Console::Printf(",");
    }
    Console::Printf("\n\t\t{"
                    "\n\t\t\t\"suffix\": \"%J\","
                    "\n\t\t\t\"alias\": \"%J\""
                    "\n\t\t}",
                    aliases[i].text, aliases[i].alias);
  }
  Console::Printf("\n\t],"
                  "\n\t\"auto-suffix\": [");
  for (size_t i = 0; i < autoSuffixCount; ++i) {
    if (i != 0) {
      Console::Printf(",");
    }
    Console::Printf("\n\t\t{"
                    "\n\t\t\t\"key\": \"%t\","
                    "\n\t\t\t\"suffix\": \"%J\""
                    "\n\t\t}",
                    &autoSuffixes[i].stroke, autoSuffixes[i].text + 1);
  }
  Console::Printf("\n\t],"
                  "\n\t\"reverse-auto-suffix\": [");
  for (size_t i = 0; i < reverseAutoSuffixCount; ++i) {
    if (i != 0) {
      Console::Printf(",");
    }
    Console::Printf("\n\t\t{"
                    "\n\t\t\t\"key\": \"%t\","
                    "\n\t\t\t\"suppressMask\": \"%t\","
                    "\n\t\t\t\"pattern\": \"%J\","
                    "\n\t\t\t\"replacement\": \"%J\""
                    "\n\t\t}",
                    &reverseAutoSuffixes[i].autoSuffix->stroke,
                    &reverseAutoSuffixes[i].suppressMask,
                    reverseAutoSuffixes[i].testPattern,
                    reverseAutoSuffixes[i].replacement);
  }
  Console::Printf("\n\t]\n}\n\n");
}

//---------------------------------------------------------------------------

StenoCompiledOrthography::StenoCompiledOrthography(
    const StenoOrthography &orthography)
    : data(orthography), patterns(CreatePatterns(orthography)) {
#if USE_ORTHOGRAPHY_CACHE
  memset(&cache, 0, sizeof(cache));
#endif
}

const Pattern *
StenoCompiledOrthography::CreatePatterns(const StenoOrthography &orthography) {
  Pattern *patterns =
      (Pattern *)malloc(sizeof(Pattern) * orthography.ruleCount);
  for (size_t i = 0; i < orthography.ruleCount; ++i) {
    patterns[i] = Pattern::Compile(orthography.rules[i].testPattern);
  }
  return patterns;
}

#if USE_ORTHOGRAPHY_CACHE

#if RECORD_ORTHOGRAPHY_CACHE_STATS
static size_t cacheHits;
static size_t cacheMisses;
#endif
char *StenoCompiledOrthography::AddSuffix(const char *word,
                                          const char *suffix) const {
  size_t blockIndex = Crc32(word, Str::Length(word)) & (CACHE_BLOCK_COUNT - 1);
  char *cachedResult = cache[blockIndex].Lookup(word, suffix);
  if (cachedResult) {
#if RECORD_ORTHOGRAPHY_CACHE_STATS
    cacheHits++;
#endif
    return cachedResult;
  }

#if RECORD_ORTHOGRAPHY_CACHE_STATS
  cacheMisses++;
#endif

  char *result = AddSuffixInternal(word, suffix);
  CacheEntry *entry = new CacheEntry(word, suffix, result);
  cache[blockIndex].AddEntry(entry);
  return result;
}

char *StenoCompiledOrthography::AddSuffixInternal(const char *word,
                                                  const char *suffix) const {
#else
char *StenoCompiledOrthography::AddSuffix(const char *word,
                                          const char *suffix) const {
#endif
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
#if RECORD_ORTHOGRAPHY_CACHE_STATS
  Console::Printf("      Cache hits: %zu/%zu\n", cacheHits,
                  cacheHits + cacheMisses);
#endif
}

//---------------------------------------------------------------------------
