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
class StenoStrokeHistory;

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

class StenoSegmentBuilder {
public:
  bool IsNotEmpty() const { return count != 0; }

  void Add(StenoStroke stroke, StenoState state) {
    strokes[count] = stroke;
    states[count] = state;
    ++count;
  }

  void TransferStartFrom(const StenoSegmentBuilder &source, size_t count);
  void TransferFrom(const StenoStrokeHistory &source, size_t sourceStrokeCount,
                    size_t maxCount);

  void CreateSegments(BuildSegmentContext &context,
                      size_t minimumStartOffset = 0);

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

  void ResetStrokes(size_t offset, size_t length);

  void ReevaluateSegments(BuildSegmentContext &context, size_t &offset);

  void HandleRetroTransform(BuildSegmentContext &context, const char *format,
                            size_t currentOffset);
  void HandleRetroInsertSpace(BuildSegmentContext &context,
                              size_t currentOffset, size_t length);
  void HandleRetroToggleAsterisk(BuildSegmentContext &context,
                                 size_t currentOffset, size_t length);
  void HandleRepeatLastStroke(BuildSegmentContext &context,
                              size_t currentOffset, size_t length);

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

  size_t GetFirstDefinitionBoundaryLength(size_t offset, size_t length) const;
};

//---------------------------------------------------------------------------
