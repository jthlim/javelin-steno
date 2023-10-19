//---------------------------------------------------------------------------

#pragma once
#include "dictionary/dictionary.h"
#include "segment.h"
#include "state.h"
#include "stroke.h"
#include <assert.h>

//---------------------------------------------------------------------------

struct StenoSegment;
class IWriter;
class StenoCompiledOrthography;

//---------------------------------------------------------------------------

struct BuildSegmentContext {
  BuildSegmentContext(StenoSegmentList &segmentList,
                      const StenoDictionary &dictionary,
                      const StenoCompiledOrthography &orthography);

  StenoSegmentList &segmentList;
  const StenoDictionary &dictionary;
  const size_t maximumOutlineLength;
  const StenoCompiledOrthography &orthography;
};

//---------------------------------------------------------------------------

class StenoStrokeHistory {
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

  void Add(StenoStroke stroke, StenoState state) {
    ShiftIfFull();
    strokes[count] = stroke;
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

  void TransferStartFrom(const StenoStrokeHistory &source, size_t count);
  void TransferFrom(const StenoStrokeHistory &source, size_t sourceStrokeCount,
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

  const StenoStroke &GetStroke(size_t i) const { return strokes[i]; }

  size_t GetStateIndex(const StenoState *v) const { return v - states; }
  const StenoState *GetStatePointer(size_t index) const {
    return &states[index];
  }

  bool HasModifiedStrokeHistory() const { return hasModifiedStrokeHistory; }

  static const size_t BUFFER_SIZE = 256;

private:
  bool hasModifiedStrokeHistory;
  size_t count = 0;
  StenoStroke strokes[BUFFER_SIZE];
  StenoState states[BUFFER_SIZE];

  void AddSegments(BuildSegmentContext &context, size_t offset);

  void RemoveOffset(BuildSegmentContext &context, size_t &offset,
                    size_t length);
  void ReevaluateSegments(BuildSegmentContext &context, size_t &offset);

  void HandleRetroTransform(BuildSegmentContext &context, const char *format,
                            size_t currentOffset);
  void HandleRetroInsertSpace(BuildSegmentContext &context,
                              size_t currentOffset);
  void HandleRetroToggleAsterisk(BuildSegmentContext &context,
                                 size_t currentOffset);
  void HandleRepeatLastStroke(BuildSegmentContext &context,
                              size_t currentOffset, const StenoState &state);

  void WriteRetroTransform(const StenoSegmentList &segments,
                           size_t startingSegmentIndex, const char *format,
                           IWriter &output) const;

  bool DirectLookup(BuildSegmentContext &context, size_t &offset);

  bool AutoSuffixLookup(BuildSegmentContext &context, size_t &offset);
  void AddRawStroke(BuildSegmentContext &context, size_t &offset);

  StenoSegment AutoSuffixTest(BuildSegmentContext &context, size_t offset,
                              size_t startLength, size_t minimumLength);

  StenoSegment AutoSuffixTest(BuildSegmentContext &context,
                              const StenoSegment &segment, size_t offset);
};

//---------------------------------------------------------------------------
