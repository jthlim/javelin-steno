//---------------------------------------------------------------------------

#pragma once
#include "container/list.h"
#include "dictionary/dictionary.h"
#include "malloc_allocate.h"
#include "state.h"

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
    size_t _suppressLookupConstructor;
    StenoDictionaryLookupResult lookup;
  };

  bool IsValid() const { return lookupType != SegmentLookupType::UNKNOWN; }
  bool HasCommand() const;
  const StenoState *GetEndStenoState() const { return state + strokeLength; }

  size_t GetStrokeIndex(const StenoState *firstState) const {
    return state - firstState;
  }

  size_t GetEndStrokeIndex(const StenoState *firstState) const {
    return (state - firstState) + strokeLength;
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

struct StenoToken {
  StenoToken(const char *text, size_t length, const StenoState *state)
      : text(text), length(length), state(state) {}

  const char *text;
  size_t length;

  // This can be null -- meaning that the state should be inferred.
  const StenoState *state;
};

class StenoTokenizer : public JavelinMallocAllocate {
public:
  virtual ~StenoTokenizer() {}

  virtual bool HasMore() const = 0;
  virtual StenoToken GetNext() = 0;

  static StenoTokenizer *Create(const List<StenoSegment> &segments,
                                size_t startingOffset = 0);
};

//---------------------------------------------------------------------------

// StenoSegmentList without automatic destruction of segments.
class StenoSegmentList : public List<StenoSegment> {
public:
  StenoSegmentList() = default;
  StenoSegmentList(List &&other) : List((List &&)other) {}
  ~StenoSegmentList();

  static size_t GetCommonStartingSegmentsCount(const List<StenoSegment> &a,
                                               const List<StenoSegment> &b);

  // Returns the starting index of a word.
  //
  // A word is defined as either finger spelling start or
  // a cluster of prefixes + lookup + suffixes.
  size_t GetWordStartingSegmentIndex(size_t endIndex) const;
};

//---------------------------------------------------------------------------
