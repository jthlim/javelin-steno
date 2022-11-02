//---------------------------------------------------------------------------

#pragma once
#include "chord.h"
#include "dictionary/dictionary.h"
#include "segment.h"
#include "state.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

struct StenoSegment;
class StenoCompiledOrthography;

//---------------------------------------------------------------------------

struct BuildSegmentContext {
  BuildSegmentContext(StenoSegmentList &segmentList,
                      const StenoDictionary &dictionary,
                      size_t maximumMatchLength,
                      const StenoCompiledOrthography &orthography,
                      size_t endOffset)
      : segmentList(segmentList), dictionary(dictionary),
        maximumMatchLength(maximumMatchLength), orthography(orthography),
        endOffset(endOffset) {}

  StenoSegmentList &segmentList;
  const StenoDictionary &dictionary;
  const size_t maximumMatchLength;
  const StenoCompiledOrthography &orthography;
  const size_t endOffset;
};

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

  void CreateSegments(BuildSegmentContext &context, size_t maximumChordLength,
                      size_t minimumStartOffset = 0);

  const StenoChord &GetChord(size_t i) const { return chords[i]; }

private:
  static const size_t BUFFER_SIZE = 256;

  size_t count = 0;
  StenoChord chords[BUFFER_SIZE];
  StenoState states[BUFFER_SIZE];

  void AddSegments(BuildSegmentContext &context, size_t offset);

  void HandleRetroactiveInsertSpace(BuildSegmentContext &context,
                                    size_t currentOffset);

  bool DirectLookup(BuildSegmentContext &context, size_t &offset);

  bool AutoSuffixLookup(BuildSegmentContext &context, size_t &offset);

  StenoSegment AutoSuffixTest(BuildSegmentContext &context, size_t offset,
                              size_t startLength, size_t minimumLength);

  StenoSegment AutoSuffixTest(BuildSegmentContext &context,
                              const StenoSegment &segment, size_t offset);
};

//---------------------------------------------------------------------------
