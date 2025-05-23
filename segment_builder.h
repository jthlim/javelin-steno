//---------------------------------------------------------------------------

#pragma once
#include "segment.h"
#include "state.h"
#include "stroke.h"
#include <assert.h>

//---------------------------------------------------------------------------

struct StenoSegment;
class BufferWriter;
class StenoCompiledOrthography;
class StenoDictionary;
class StenoEngine;
class StenoStrokeHistory;

//---------------------------------------------------------------------------

struct BuildSegmentContext {
  BuildSegmentContext(StenoSegmentList &segments, StenoEngine &engine);
  ~BuildSegmentContext() { free(setValueText); }

  const char *lastSegmentCommand = nullptr;
  int setValueIndex;
  char *setValueText = nullptr;
  StenoSegmentList &segments;
  StenoEngine &engine;
  const StenoDictionary &dictionary;
  const StenoCompiledOrthography &orthography;
  const size_t maximumOutlineLength;
};

//---------------------------------------------------------------------------

class StenoSegmentBuilder {
public:
  bool IsNotEmpty() const { return count != 0; }

  void Reset() {
    hasRawStroke = false;
    count = 0;
  }

  void Add(StenoStroke stroke, StenoState state) {
    strokes[count] = stroke;
    states[count] = state;
    ++count;
  }

  void Add(const StenoStroke *strokes, size_t length) {
    count = length;

    strokes->CopyTo(this->strokes, length);

    StenoState emptyState;
    emptyState.Reset();

    for (size_t i = 0; i < length; ++i) {
      states[i] = emptyState;
    }
  }

  void TransferStartFrom(const StenoSegmentBuilder &source, size_t count);
  void TransferFrom(const StenoStrokeHistory &source, size_t sourceStrokeCount,
                    size_t maxCount);

  void CreateSegments(BuildSegmentContext &context,
                      size_t minimumStartOffset = 0) {
    AddSegments(context, minimumStartOffset);
    if (context.lastSegmentCommand) {
      UpdateLastSegmentWithCommand(context, context.lastSegmentCommand);
    }
  }

  size_t GetStateIndex(const StenoState *v) const { return v - states; }
  const StenoState *GetStatePointer(size_t index) const {
    return &states[index];
  }

  const StenoStroke *GetStrokes(size_t index) const { return &strokes[index]; }

  bool HasModifiedStrokeHistory() const { return hasModifiedStrokeHistory; }
  bool HasRawStroke() const { return hasRawStroke; }

  static constexpr size_t BUFFER_SIZE = 256;

private:
  bool hasModifiedStrokeHistory = false;
  bool hasRawStroke;
  size_t count = 0;
  const char *lastSegmentCommand;
  StenoStroke strokes[BUFFER_SIZE];
  StenoState states[BUFFER_SIZE];

  char *EscapeCommand(const char *p);
  void EscapeCommand(BufferWriter &writer, const char *p);

  void AddSegments(BuildSegmentContext &context, size_t &offset);

  void ResetStrokes(size_t offset, size_t length);

  void ReevaluateSegments(BuildSegmentContext &context, size_t &offset);

  void HandleRetroTransform(BuildSegmentContext &context, const char *command,
                            size_t currentOffset, size_t length);
  void HandleRetroInsertSpace(BuildSegmentContext &context,
                              size_t currentOffset, size_t length);
  void HandleRetroToggleAsterisk(BuildSegmentContext &context,
                                 size_t currentOffset, size_t length);
  void HandleRepeatLastStroke(BuildSegmentContext &context,
                              size_t currentOffset, size_t length);
  void HandleRetroSetValue(BuildSegmentContext &context, const char *command,
                           size_t currentOffset, size_t length);
  void HandleTransform(BuildSegmentContext &context, size_t currentOffset,
                       size_t length);
  void CreateTransformString(BufferWriter &bufferWriter,
                             BuildSegmentContext &context,
                             const char *format) const;

  void WriteRetroTransform(const StenoSegmentList &segments,
                           size_t startingSegmentIndex, const char *format,
                           BufferWriter &output) const;

  bool DirectLookup(BuildSegmentContext &context, size_t &offset);

  bool AutoSuffixLookup(BuildSegmentContext &context, size_t &offset);
  void AddRawStroke(BuildSegmentContext &context, size_t &offset);

  void UpdateLastSegmentWithCommand(BuildSegmentContext &context,
                                    const char *command);

  StenoSegment AutoSuffixTest(BuildSegmentContext &context, size_t offset,
                              size_t startLength, size_t minimumLength);

  StenoSegment AutoSuffixTest(BuildSegmentContext &context,
                              const StenoSegment &segment, size_t offset);

  size_t GetFirstDefinitionBoundaryLength(size_t offset, size_t length) const;
};

//---------------------------------------------------------------------------
