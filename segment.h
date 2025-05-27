//---------------------------------------------------------------------------

#pragma once
#include "container/sized_list.h"
#include "dictionary/dictionary.h"
#include "state.h"

//---------------------------------------------------------------------------

class StenoSegmentList;

//---------------------------------------------------------------------------

// A steno segment is a mapping of List<Stroke> -> Translation.
struct StenoSegment {
  StenoSegment(size_t strokeLength, SegmentLookupType lookupType,
               const StenoState *state, StenoDictionaryLookupResult lookup)
      : lookupType(lookupType), strokeLength(strokeLength), state(state),
        lookup(lookup) {}

  bool ContainsKeyCode() const;

  SegmentLookupType lookupType;
  size_t strokeLength;
  const StenoState *state;
  union {
    // Suppress constructor.
    StenoDictionaryLookupResult lookup;
  };

  bool IsValid() const { return lookupType != SegmentLookupType::UNKNOWN; }
  bool HasCommand() const;
  const StenoState *GetEndStenoState() const { return state + strokeLength; }

  size_t GetStrokeIndex(const StenoState *firstState) const {
    return state - firstState;
  }

  size_t GetEndStrokeIndex(const StenoState *firstState) const {
    return GetStrokeIndex(firstState) + strokeLength;
  }

  SegmentHistoryRequirements GetHistoryRequirements() const;

  static StenoSegment CreateInvalid() { return StenoSegment(); }

private:
  StenoSegment() : lookupType(SegmentLookupType::UNKNOWN) {}

  static bool IsPunctuationCommand(const char *start, const char *end);
  static bool IsPrefixCommand(const char *start, const char *end);
  static bool IsSuffixCommand(const char *start, const char *end);
  static bool IsCarryCapitalizationCommand(const char *start, const char *end);
  static bool IsCaseModifierCommand(const char *start, const char *end);
  static bool IsFingerSpellingCommand(const char *start, const char *end);
};

//---------------------------------------------------------------------------

// A StenoToken are the individual parts of a definition.
//
// e.g. "{^}test{.}" has three tokens: "{^}", "test" and "{.}"
struct StenoToken {
  StenoToken(const char *text, size_t length, const StenoState *state,
             bool isLastSegment)
      : text(text), length(length), state(state), isLastSegment(isLastSegment) {
  }

  // This text is *not* null terminated.
  const char *const text;
  size_t length;

  // This can be null -- meaning that the state should be inferred.
  const StenoState *const state;

  bool isLastSegment;

  // Returns text as a null-terminated string.
  char *DupText() const { return Str::DupN(text, length); }
};

class StenoTokenizerIterator;

class StenoTokenizer {
public:
  StenoTokenizer(const StenoSegmentList &list, size_t startingOffset = 0);

  bool HasMore() const { return p != nullptr; }
  StenoToken GetNext();

  friend StenoTokenizerIterator begin(StenoTokenizer &tokenizer);
  friend void *end(StenoTokenizer &tokenizer) { return nullptr; }

private:
  const StenoSegmentList &list;
  size_t elementIndex;

  const char *p;
  const StenoState *nextState = nullptr;

  void PrepareNextP();
};

class StenoTokenizerIterator {
public:
  StenoTokenizerIterator(StenoTokenizer &tokenizer) : tokenizer(tokenizer) {}

  StenoToken operator*() { return tokenizer.GetNext(); }
  StenoTokenizerIterator &operator++() { return *this; }
  bool operator!=(const void *) const { return tokenizer.HasMore(); }

private:
  StenoTokenizer &tokenizer;
};

inline StenoTokenizerIterator begin(StenoTokenizer &tokenizer) {
  return StenoTokenizerIterator(tokenizer);
}

//---------------------------------------------------------------------------

// StenoSegmentList with automatic destruction of segments.
class StenoSegmentList : public SizedList<StenoSegment> {
public:
  StenoSegmentList(size_t maximumSize);
  ~StenoSegmentList();

  static size_t GetCommonStartingSegmentsCount(const StenoSegmentList &a,
                                               const StenoSegmentList &b);

  // Returns the starting index of a word.
  //
  // A word is defined as either finger spelling start or
  // a cluster of prefixes + lookup + suffixes.
  size_t GetWordStartingSegmentIndex(size_t endIndex) const;

  bool HasManualStateChange(size_t startIndex) const;
};

//---------------------------------------------------------------------------
