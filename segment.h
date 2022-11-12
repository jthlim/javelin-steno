//---------------------------------------------------------------------------

#pragma once
#include "dictionary/dictionary.h"
#include "list.h"
#include "state.h"

//---------------------------------------------------------------------------

// A steno segment is a mapping of List<Chord> -> Translation.
struct StenoSegment {
  StenoSegment();
  StenoSegment(size_t chordLength, const StenoState *state,
               StenoDictionaryLookupResult lookup)
      : chordLength(chordLength), state(state), lookup(lookup) {}

  static StenoSegment CreateInvalid() {
    StenoSegment segment;
    segment.chordLength = 0;
    return segment;
  }

  size_t chordLength;
  const StenoState *state;
  StenoDictionaryLookupResult lookup;

  bool IsValid() const { return chordLength != 0; }
};

//---------------------------------------------------------------------------

struct StenoToken {
  StenoToken(const char *text, const StenoState *state)
      : text(text), state(state) {}

  const char *text;

  // This can be null -- meaning that the state should be inferred.
  const StenoState *state;
};

class StenoTokenizer {
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

  StenoTokenizer *CreateTokenizer();
};

//---------------------------------------------------------------------------
