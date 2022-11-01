//---------------------------------------------------------------------------

#pragma once
#include "chord.h"
#include "dictionary/dictionary.h"
#include "state.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

struct StenoSegment;
class StenoSegmentList;

//---------------------------------------------------------------------------

class ChordHistory {
public:
  bool IsEmpty() const { return count == 0; }
  bool IsFull() const { return count == BUFFER_SIZE; }
  size_t GetCount() const { return count; }

  void Shift();

  void ShiftIfFull() {
    if (count == BUFFER_SIZE) {
      Shift();
    }
  }

  void Add(StenoChord chord, StenoState state) {
    ShiftIfFull();
    chords[count] = chord;
    states[count] = state;
    ++count;
  }

  void Pop() {
    assert(count != 0);
    --count;
  }

  void Reset() { count = 0; }

  const StenoState &BackState() const { return states[count - 1]; }

  StenoSegmentList CreateSegments(size_t maximumChordCount,
                                  const StenoDictionary &dictionary,
                                  size_t maximumChordLength,
                                  size_t minimumStartOffset = 0);

  const StenoChord &GetChord(size_t i) const { return chords[i]; }

private:
  static const size_t BUFFER_SIZE = 256;

  size_t count = 0;
  StenoChord chords[BUFFER_SIZE];
  StenoState states[BUFFER_SIZE];

  void AddSegments(StenoSegmentList &list, size_t offset, size_t endOffset,
                   const StenoDictionary &dictionary);

  void HandleRetroactiveInsertSpace(StenoSegmentList &list,
                                    const StenoDictionary &dictionary);

  bool DirectLookup(StenoSegmentList &list, size_t &offset, size_t endOffset,
                    const StenoDictionary &dictionary,
                    size_t maximumMatchLength);

  bool AutoSuffixLookup(StenoSegmentList &list, size_t &offset,
                        size_t endOffset, const StenoDictionary &dictionary,
                        size_t maximumMatchLength);

  StenoSegment AutoSuffixTest(size_t offset, size_t startLength,
                              size_t minimumLength,
                              const StenoDictionary &dictionary);

  StenoSegment AutoSuffixTest(const StenoSegment &segment, size_t offset,
                              const StenoDictionary &dictionary,
                              size_t maximumMatchLength);
};

//---------------------------------------------------------------------------
