//---------------------------------------------------------------------------

#pragma once
#include "dictionary/dictionary.h"
#include "list.h"
#include "malloc_allocate.h"
#include "state.h"

//---------------------------------------------------------------------------

// A steno segment is a mapping of List<Stroke> -> Translation.
struct StenoSegment {
  StenoSegment(size_t strokeLength, const StenoState *state,
               StenoDictionaryLookupResult lookup)
      : strokeLength(strokeLength), state(state), lookup(lookup) {}

  bool ContainsKeyCode() const;

  size_t strokeLength;
  const StenoState *state;
  union {
    size_t _suppressLookupConstructor;
    StenoDictionaryLookupResult lookup;
  };

  bool IsValid() const { return strokeLength != 0; }
  bool HasCommand() const;
  const StenoState *GetEndStenoState() const { return state + strokeLength; }

  static StenoSegment CreateInvalid() { return StenoSegment(); }

private:
  StenoSegment() : strokeLength(0) {}
};

//---------------------------------------------------------------------------

struct StenoToken {
  StenoToken(const char *text, const StenoState *state)
      : text(text), state(state) {}

  const char *text;

  // This can be null -- meaning that the state should be inferred.
  const StenoState *state;
};

class StenoTokenizer : public JavelinMallocAllocate {
public:
  virtual ~StenoTokenizer() {}

  virtual bool HasMore() const = 0;
  virtual StenoToken GetNext() = 0;
};

//---------------------------------------------------------------------------

class StenoSegmentList : public List<StenoSegment> {
public:
  StenoSegmentList() = default;
  StenoSegmentList(StenoSegmentList &&other)
      : List((List<StenoSegment> &&) other) {}
  ~StenoSegmentList();

  StenoTokenizer *CreateTokenizer(size_t startingOffset = 0);

  static size_t GetCommonStartingSegmentsCount(StenoSegmentList &a,
                                               StenoSegmentList &b);
};

//---------------------------------------------------------------------------
