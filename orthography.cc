//---------------------------------------------------------------------------

#include "orthography.h"

#include "console.h"
#include "crc32.h"
#include "hal/external_flash.h"
#include "mem.h"
#include "str.h"
#include "unicode.h"
#include "word_list.h"

//---------------------------------------------------------------------------

constexpr size_t MAXIMUM_CACHEABLE_WORD_LENGTH = 28;

//---------------------------------------------------------------------------

class StenoCompiledOrthography::BestCandidate {
public:
  void Add(char *newCandidate, int score);

  char *GetResult() const { return candidate; }

  bool IsEmpty() const { return candidate == nullptr; }
  bool IsNotEmpty() const { return candidate != nullptr; }

  static constexpr int NOT_IN_WORD_LIST_SCORE = WordList::MAX_SCORE + 1;
  static constexpr int FALLBACK_SCORE = WordList::MAX_SCORE + 2;
  static constexpr int EXCLUDE_WORD_SCORE = WordList::MAX_SCORE + 4;

private:
  char *candidate = nullptr;
  int score = WordList::MAX_SCORE + 3;
};

void StenoCompiledOrthography::BestCandidate::Add(char *newCandidate,
                                                  int newScore) {
  char *unused;
  if (newScore < score) {
    unused = candidate;
    candidate = newCandidate;
    score = newScore;
  } else {
    unused = newCandidate;
  }
  free(unused);
}

//---------------------------------------------------------------------------

#if USE_ORTHOGRAPHY_CACHE

#define RECORD_ORTHOGRAPHY_CACHE_STATS 0

#if RUN_TESTS

void StenoCompiledOrthography::LockCache() {}
void StenoCompiledOrthography::UnlockCache() {}

#endif

void StenoCompiledOrthography::CacheEntry::Reset() {
  crc = 0;
  suffixOffset = 0;
}

inline char *StenoCompiledOrthography::CacheEntry::DupResult() const {
  char *r = (char *)malloc(resultLength + 1);
  char *p = r;
  memcpy(p, GetWordPointer(), commonWordLength);
  p += commonWordLength;
  const size_t resultDataLength =
      resultLength - commonWordLength - commonSuffixLength;
  memcpy(p, GetResultPointer(), resultDataLength);
  p += resultDataLength;
  memcpy(p, GetSuffixPointer() + GetSuffixLength() - commonSuffixLength,
         commonSuffixLength);
  p += commonSuffixLength;
  *p = '\0';
  return r;
}

void StenoCompiledOrthography::CacheEntry::Set(uint32_t crc, const char *word,
                                               size_t wordLength,
                                               const char *suffix,
                                               size_t suffixLength,
                                               const char *result) {
  size_t commonWordLength = 0;
  while (word[commonWordLength] &&
         word[commonWordLength] == result[commonWordLength]) {
    ++commonWordLength;
  }
  const size_t resultLength = Str::Length(result);
  size_t commonSuffixLength = 0;
  while (commonSuffixLength + commonWordLength < resultLength &&
         result[resultLength - commonSuffixLength - 1] ==
             suffix[suffixLength - commonSuffixLength - 1]) {
    ++commonSuffixLength;
  }

  const size_t entryLength =
      wordLength + suffixLength + resultLength - commonWordLength;
  if (entryLength > MAXIMUM_DATA_LENGTH) {
    return;
  }

  this->crc = crc;
  this->resultLength = resultLength;
  this->commonWordLength = commonWordLength;
  this->commonSuffixLength = commonSuffixLength;

  char *p = data;
  memcpy(p, word, wordLength);
  p += wordLength;

  suffixOffset = p - data;
  memcpy(p, suffix, suffixLength);
  p += suffixLength;

  resultOffset = p - data;
  memcpy(p, result + commonWordLength,
         resultLength - commonWordLength - commonSuffixLength);
}

bool StenoCompiledOrthography::CacheEntry::IsEqual(uint32_t crc,
                                                   const char *word,
                                                   const char *suffix) const {
  if (crc != this->crc) [[likely]] {
    return false;
  }
  return Str::Eq(word, GetWordPointer(), GetWordLength()) &&
         Str::Eq(suffix, GetSuffixPointer(), GetSuffixLength());
}

size_t StenoCompiledOrthography::CacheEntry::GetMemoryUsage() const {
  return resultOffset + resultLength + 1;
}

char *StenoCompiledOrthography::CacheBlock::Lookup(uint32_t crc,
                                                   const char *word,
                                                   const char *suffix) const {
  LockCache();
  for (size_t i = 0; i < CACHE_ASSOCIATIVITY; ++i) {
    const CacheEntry &entry = entries[i];
    if (entry.IsEqual(crc, word, suffix)) {
      char *result = entry.DupResult();
      UnlockCache();
      return result;
    }
  }

  UnlockCache();
  return nullptr;
}

void StenoCompiledOrthography::CacheBlock::AddEntry(
    uint32_t crc, const char *word, size_t wordLength, const char *suffix,
    size_t suffixLength, const char *result) {
  LockCache();

  const size_t entryIndex = nextEntryIndex++ % CACHE_ASSOCIATIVITY;
  CacheEntry &entry = entries[entryIndex];
  entry.Set(crc, word, wordLength, suffix, suffixLength, result);

  UnlockCache();
}

void StenoCompiledOrthography::CacheBlock::Reset() {
  LockCache();

  for (CacheEntry &entry : entries) {
    entry.Reset();
  }

  UnlockCache();
}

size_t StenoCompiledOrthography::CacheBlock::GetMemoryUsage() const {
  size_t total = 0;
  for (const CacheEntry &entry : entries) {
    total += entry.GetMemoryUsage();
  }
  return total;
}

#endif

//---------------------------------------------------------------------------

constexpr StenoOrthography StenoOrthography::emptyOrthography = {
    .rules = {0, nullptr},
    .aliases = {0, nullptr},
    .autoSuffixes = {0, nullptr},
    .reverseAutoSuffixes = {0, nullptr},
};

//---------------------------------------------------------------------------

void StenoOrthography::Print() const {
  Console::Printf("orthography:"
                  "\n\trules:");
  if (rules.IsEmpty()) {
    Console::Printf(" []");
  } else {
    for (const StenoOrthographyRule &rule : rules) {
      Console::Printf("\n\t\t- pattern: \"%J\""
                      "\n\t\t  replacement: \"%J\"",
                      rule.testPattern, rule.replacement);
    }
  }
  Console::Printf("\n\taliases:");
  if (aliases.IsEmpty()) {
    Console::Printf(" []");
  } else {
    for (const StenoOrthographyAlias &alias : aliases) {
      Console::Printf("\n\t\t- suffix: \"%J\""
                      "\n\t\t  alias: \"%J\"",
                      alias.text, alias.alias);
    }
  }
  Console::Printf("\n\tauto-suffix:");
  if (autoSuffixes.IsEmpty()) {
    Console::Printf(" []");
  } else {
    for (const StenoOrthographyAutoSuffix &autoSuffix : autoSuffixes) {
      Console::Printf("\n\t\t- key: \"%t\""
                      "\n\t\t  suffix: \"%J\"",
                      &autoSuffix.stroke, autoSuffix.text + 1);
    }
  }
  Console::Printf("\n\treverse-suffix:");
  if (reverseSuffixes.IsEmpty()) {
    Console::Printf(" []");
  } else {
    for (const StenoOrthographyRule &reverseSuffix : reverseSuffixes) {
      Console::Printf("\n\t\t- pattern: \"%J\""
                      "\n\t\t  replacement: \"%J\"",
                      reverseSuffix.testPattern, reverseSuffix.replacement);
    }
  }
  Console::Printf("\n\treverse-auto-suffix:");
  if (reverseAutoSuffixes.IsEmpty()) {
    Console::Printf(" []");
  } else {
    for (const StenoOrthographyReverseAutoSuffix &reverseAutoSuffix :
         reverseAutoSuffixes) {
      Console::Printf("\n\t\t- key: \"%t\""
                      "\n\t\t  suppressMask: \"%t\""
                      "\n\t\t  pattern: \"%J\""
                      "\n\t\t  replacement: \"%J\"",
                      &reverseAutoSuffix.autoSuffix->stroke,
                      &reverseAutoSuffix.suppressMask,
                      reverseAutoSuffix.testPattern,
                      reverseAutoSuffix.replacement);
    }
  }
  Console::Printf("\n\n");
}

//---------------------------------------------------------------------------

StenoCompiledOrthography::StenoCompiledOrthography(
    const StenoOrthography &orthography)
    : data(orthography), patterns(CreatePatterns(orthography)) {
#if USE_ORTHOGRAPHY_CACHE
  Mem::Clear(cache);
#endif
}

const Pattern *
StenoCompiledOrthography::CreatePatterns(const StenoOrthography &orthography) {
  Pattern *patterns =
      (Pattern *)malloc(sizeof(Pattern) * orthography.rules.GetCount());
  Pattern *p = patterns;
  for (const StenoOrthographyRule &rule : orthography.rules) {
    *p++ = Pattern::Compile(rule.testPattern);
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
  const size_t wordLength = Str::Length(word);
  if (wordLength >= MAXIMUM_CACHEABLE_WORD_LENGTH) {
    return AddSuffixInternal(word, wordLength, suffix);
  }

  const size_t suffixLength = Str::Length(suffix);
  const uint32_t crc =
      Crc32::Hash(word, wordLength) ^ Crc32::Hash(suffix, suffixLength);

  const size_t blockIndex = crc % CACHE_BLOCK_COUNT;
  char *cachedResult = cache[blockIndex].Lookup(crc, word, suffix);
  if (cachedResult) {
#if RECORD_ORTHOGRAPHY_CACHE_STATS
    cacheHits++;
#endif
    return cachedResult;
  }

#if RECORD_ORTHOGRAPHY_CACHE_STATS
  cacheMisses++;
#endif

  char *result = AddSuffixInternal(word, wordLength, suffix);
  cache[blockIndex].AddEntry(crc, word, wordLength, suffix, suffixLength,
                             result);
  return result;
}

#else
char *StenoCompiledOrthography::AddSuffix(const char *word,
                                          const char *suffix) const {
  return AddSuffixInternal(word, Str::Length(word), suffix);
}
#endif

char *StenoCompiledOrthography::AddSuffixInternal(const char *word,
                                                  size_t wordLength,
                                                  const char *suffix) const {
  BestCandidate bestCandidate;

  for (const StenoOrthographyAlias &alias : data.aliases) {
    if (Str::Eq(suffix, alias.text)) {
      AddCandidates(bestCandidate, word, wordLength, alias.alias,
                    BestCandidate::EXCLUDE_WORD_SCORE);
    }
  }

  char *simple = Str::Join(word, suffix);
  const int score =
      WordList::GetWordRank(simple, BestCandidate::FALLBACK_SCORE);
  bestCandidate.Add(simple, score);

  AddCandidates(bestCandidate, word, wordLength, suffix,
                BestCandidate::NOT_IN_WORD_LIST_SCORE);

  return bestCandidate.GetResult();
}

char *StenoCompiledOrthography::AddSuffixToPhrase(const char *phrase,
                                                  const char *suffix) const {
  const char *lastWord = phrase;
  const uint8_t *p = (const uint8_t *)phrase;
  for (;;) {
    const uint32_t c = *p++;
    if (c == '\0') {
      break;
    }
    if (Unicode::IsWhitespace(c)) {
      lastWord = (const char *)p;
    }
  }

  if (lastWord == phrase) {
    return AddSuffix(phrase, suffix);
  }

  char *suffixedLastWord = AddSuffix(lastWord, suffix);
  const size_t suffixedLastWordLength = Str::Length(suffixedLastWord);
  const size_t prefixLength = lastWord - phrase;
  char *result = (char *)malloc(prefixLength + suffixedLastWordLength + 1);
  memcpy(result, phrase, prefixLength);
  memcpy(result + prefixLength, suffixedLastWord, suffixedLastWordLength + 1);
  free(suffixedLastWord);
  return result;
}

void StenoCompiledOrthography::AddCandidates(BestCandidate &bestCandidate,
                                             const char *word,
                                             size_t wordLength,
                                             const char *suffix,
                                             int defaultScore) const {
  char *text = Str::Join(word, " ^", suffix);
  const size_t textLength = Str::Length(text);

  const PatternQuickReject inputQuickReject(text);

  for (size_t i = 0; i < data.rules.GetCount(); ++i) {
    const Pattern &pattern = patterns[i];
    if (!pattern.IsPossibleMatch(inputQuickReject)) {
      continue;
    }

    const PatternMatch match =
        pattern.MatchBypassingQuickReject(text, textLength);
    if (!match.match) {
      continue;
    }

    XipPointer<StenoOrthographyRule> rules = begin(data.rules);
    char *candidate = match.Replace(rules[i].replacement);
    const int score = WordList::GetWordRank(candidate, defaultScore);
    bestCandidate.Add(candidate, score);
  }
  free(text);
}

//---------------------------------------------------------------------------

#if USE_ORTHOGRAPHY_CACHE
void StenoCompiledOrthography::ResetCache() {
  for (CacheBlock &block : cache) {
    block.Reset();
  }
}

size_t StenoCompiledOrthography::GetCacheMemoryUsage() const {
  size_t total = 0;
  for (const CacheBlock &block : cache) {
    total += block.GetMemoryUsage();
  }
  return total;
}

#endif

void StenoCompiledOrthography::PrintInfo() const {
  const ExternalFlashSentry sentry;

  Console::Printf("    Orthography\n");
  Console::Printf("      Rules: %zu\n", data.rules.GetCount());
  Console::Printf("      Aliases: %zu\n", data.aliases.GetCount());
  Console::Printf("      Auto-suffixes: %zu\n", data.autoSuffixes.GetCount());
  Console::Printf("      Reverse suffixes: %zu\n",
                  data.reverseSuffixes.GetCount());
  Console::Printf("      Reverse auto-suffixes: %zu\n",
                  data.reverseAutoSuffixes.GetCount());
#if RECORD_ORTHOGRAPHY_CACHE_STATS
  Console::Printf("      Cache hits: %zu/%zu\n", cacheHits,
                  cacheHits + cacheMisses);
#endif
#if USE_ORTHOGRAPHY_CACHE
  // Console::Printf("      Cache memory usage: %zu bytes\n",
  //                 GetCacheMemoryUsage());
#endif
}

//---------------------------------------------------------------------------
