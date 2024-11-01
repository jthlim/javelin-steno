//---------------------------------------------------------------------------

#include "orthography.h"
#include "console.h"
#include "crc.h"
#include "mem.h"
#include "str.h"
#include "unicode.h"
#include "utf8_pointer.h"
#include "word_list.h"

//---------------------------------------------------------------------------

const size_t MAXIMUM_CACHEABLE_WORD_LENGTH = 64;

//---------------------------------------------------------------------------

class StenoCompiledOrthography::BestCandidate {
public:
  void Add(char *newCandidate, int score);

  char *GetResult() const { return candidate; }

  bool IsEmpty() const { return candidate == nullptr; }
  bool IsNotEmpty() const { return candidate != nullptr; }

  static const int NOT_IN_WORD_LIST_SCORE = WordList::MAX_SCORE + 1;
  static const int FALLBACK_SCORE = WordList::MAX_SCORE + 2;
  static const int EXCLUDE_WORD_SCORE = WordList::MAX_SCORE + 4;

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

inline char *StenoCompiledOrthography::CacheEntry::DupResult() const {
  return Str::DupN(GetResult(), resultLength);
}

void StenoCompiledOrthography::CacheEntry::Set(const char *word,
                                               const char *suffix,
                                               const char *result) {
  free(base);

  const size_t wordLength = Str::Length(word);
  const size_t suffixLength = Str::Length(suffix);
  const size_t resultLength = Str::Length(result);
  this->resultLength = resultLength;
  base = (char *)malloc(wordLength + suffixLength + resultLength + 3);

  char *p = base;
  memcpy(p, word, wordLength + 1);
  p += wordLength + 1;

  suffixOffset = p - base;
  memcpy(p, suffix, suffixLength + 1);
  p += suffixLength + 1;

  resultOffset = p - base;
  memcpy(p, result, resultLength + 1);
}

bool StenoCompiledOrthography::CacheEntry::IsEqual(const char *word,
                                                   const char *suffix) const {
  return base != nullptr && Str::Eq(word, GetWord()) &&
         Str::Eq(suffix, GetSuffix());
}

char *StenoCompiledOrthography::CacheBlock::Lookup(const char *word,
                                                   const char *suffix) const {
  LockCache();
  for (size_t i = 0; i < CACHE_ASSOCIATIVITY; ++i) {
    const CacheEntry &entry = entries[i];
    if (entry.IsEqual(word, suffix)) {
      char *result = entry.DupResult();
      UnlockCache();
      return result;
    }
  }

  UnlockCache();
  return nullptr;
}

void StenoCompiledOrthography::CacheBlock::AddEntry(const char *word,
                                                    const char *suffix,
                                                    const char *result) {
  LockCache();

  const size_t entryIndex = entries[0].blockIndex++ & (CACHE_ASSOCIATIVITY - 1);
  CacheEntry &entry = entries[entryIndex];
  entry.Set(word, suffix, result);

  UnlockCache();
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
  Console::Printf("{"
                  "\n\t\"rules\": [");
  for (size_t i = 0; i < rules.GetCount(); ++i) {
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
  for (size_t i = 0; i < aliases.GetCount(); ++i) {
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
  for (size_t i = 0; i < autoSuffixes.GetCount(); ++i) {
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
  for (size_t i = 0; i < reverseAutoSuffixes.GetCount(); ++i) {
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

void StenoOrthography::Print_Binding(void *context, const char *commandLine) {
  ((StenoOrthography *)context)->Print();
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
  for (size_t i = 0; i < orthography.rules.GetCount(); ++i) {
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
  const size_t wordLength = Str::Length(word);
  if (wordLength >= MAXIMUM_CACHEABLE_WORD_LENGTH) {
    return AddSuffixInternal(word, suffix);
  }

  const uint32_t crc =
      Crc32(word, wordLength) ^ Crc32(suffix, Str::Length(suffix));

  const size_t blockIndex = crc & (CACHE_BLOCK_COUNT - 1);
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
  cache[blockIndex].AddEntry(word, suffix, result);
  return result;
}

char *StenoCompiledOrthography::AddSuffixInternal(const char *word,
                                                  const char *suffix) const {
#else
char *StenoCompiledOrthography::AddSuffix(const char *word,
                                          const char *suffix) const {
#endif
  BestCandidate bestCandidate;

  for (const StenoOrthographyAlias &alias : data.aliases) {
    if (Str::Eq(suffix, alias.text)) {
      AddCandidates(bestCandidate, word, alias.alias,
                    BestCandidate::EXCLUDE_WORD_SCORE);
    }
  }

  char *simple = Str::Join(word, suffix);
  const int score =
      WordList::GetWordRank(simple, BestCandidate::FALLBACK_SCORE);
  bestCandidate.Add(simple, score);

  AddCandidates(bestCandidate, word, suffix,
                BestCandidate::NOT_IN_WORD_LIST_SCORE);

  return bestCandidate.GetResult();
}

char *StenoCompiledOrthography::AddSuffixToPhrase(const char *phrase,
                                                  const char *suffix) const {
  const char *lastWord = phrase;
  Utf8Pointer p = phrase;
  for (;;) {
    const uint32_t c = *p++;
    if (c == '\0') {
      break;
    }
    if (Unicode::IsWhitespace(c)) {
      lastWord = p.GetRawPointer();
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
                                             const char *suffix,
                                             int defaultScore) const {
  const size_t MAXIMUM_PREFIX_LENGTH = 8;

  const size_t wordLength = Str::Length(word);
  const size_t offset = wordLength > MAXIMUM_PREFIX_LENGTH
                            ? wordLength - MAXIMUM_PREFIX_LENGTH
                            : 0;
  char *text = Str::Join(word + offset, " ^", suffix);

  const PatternQuickReject inputQuickReject(text);

  for (size_t i = 0; i < data.rules.GetCount(); ++i) {
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
      const size_t candidateWithNulLength = Str::Length(candidate) + 1;
      char *fullCandidate = (char *)malloc(offset + candidateWithNulLength);
      memcpy(fullCandidate, word, offset);
      memcpy(fullCandidate + offset, candidate, candidateWithNulLength);
      free(candidate);
      candidate = fullCandidate;
    }

    const int score = WordList::GetWordRank(candidate, defaultScore);
    bestCandidate.Add(candidate, score);
  }
  free(text);
}

//---------------------------------------------------------------------------

void StenoCompiledOrthography::PrintInfo() const {
  Console::Printf("    Orthography\n");
  Console::Printf("      Rules: %zu\n", data.rules.GetCount());
  Console::Printf("      Aliases: %zu\n", data.aliases.GetCount());
  Console::Printf("      Auto-suffixes: %zu\n", data.autoSuffixes.GetCount());
  Console::Printf("      Reverse auto-suffixes: %zu\n",
                  data.reverseAutoSuffixes.GetCount());
#if RECORD_ORTHOGRAPHY_CACHE_STATS
  Console::Printf("      Cache hits: %zu/%zu\n", cacheHits,
                  cacheHits + cacheMisses);
#endif
}

//---------------------------------------------------------------------------
