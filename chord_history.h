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
                      const StenoCompiledOrthography &orthography);

  StenoSegmentList &segmentList;
  const StenoDictionary &dictionary;
  const size_t maximumMatchLength;
  const StenoCompiledOrthography &orthography;
};

//---------------------------------------------------------------------------

class ChordHistory {
public:
  bool IsEmpty() const { return count == 0; }
  bool IsNotEmpty() const { return count != 0; }
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

  // When undo is pressed, returns how many items should be removed
  // from the list up to maxCount.
  size_t GetUndoCount(size_t maxCount) const;

  void PopCount(size_t popCount) { count -= popCount; }

  void TransferFrom(const ChordHistory &source, size_t sourceChordCount,
                    size_t maxCount);

  void SetBackCombineUndo() { states[count - 1].shouldCombineUndo = true; }
  void SetBackHasManualStateChange() {
    states[count - 1].isManualStateChange = true;
  }

  const StenoState &BackState(size_t fromEnd = 1) const {
    return states[count - fromEnd];
  }

  void CreateSegments(BuildSegmentContext &context,
                      size_t minimumStartOffset = 0);

  const StenoChord &GetChord(size_t i) const { return chords[i]; }

  static const size_t BUFFER_SIZE = 256;

private:
  size_t count = 0;
  StenoChord chords[BUFFER_SIZE];
  StenoState states[BUFFER_SIZE];

  void AddSegments(BuildSegmentContext &context, size_t offset);

  void RemoveOffset(BuildSegmentContext &context, size_t &offset,
                    size_t length);
  void ReevaluateSegments(BuildSegmentContext &context, size_t &offset);
  void HandleRetroactiveInsertSpace(BuildSegmentContext &context,
                                    size_t currentOffset);

  void HandleRetroactiveToggleAsterisk(BuildSegmentContext &context,
                                       size_t currentOffset);

  void HandleRepeatLastStroke(BuildSegmentContext &context,
                              size_t currentOffset, const StenoState &state);

  bool DirectLookup(BuildSegmentContext &context, size_t &offset);

  bool AutoSuffixLookup(BuildSegmentContext &context, size_t &offset);

  StenoSegment AutoSuffixTest(BuildSegmentContext &context, size_t offset,
                              size_t startLength, size_t minimumLength);

  StenoSegment AutoSuffixTest(BuildSegmentContext &context,
                              const StenoSegment &segment, size_t offset);
};

//---------------------------------------------------------------------------
