//---------------------------------------------------------------------------

#include "stroke_history.h"
#include "segment.h"

//---------------------------------------------------------------------------

void StenoStrokeHistory::Prune() {
  RemoveFront();
  while (IsNotEmpty() && !Front().state.isDefinitionStart) {
    RemoveFront();
  }
}

//---------------------------------------------------------------------------

size_t StenoStrokeHistory::GetUndoCount(size_t maxCount) const {
  size_t count = GetCount();
  if (maxCount < count) {
    count = maxCount;
  }
  if (count == 0) {
    return 0;
  }

  size_t result = 1;
  while (result < count && Back(result).state.shouldCombineUndo) {
    ++result;
  }
  return result;
}

//---------------------------------------------------------------------------

void StenoStrokeHistory::UpdateDefinitionBoundaries(
    size_t startingOffset, const StenoSegmentList &segments) {
  if (segments.IsEmpty()) {
    return;
  }

  size_t count = GetCount();
  for (size_t i = startingOffset; i < count; ++i) {
    (*this)[i].state.isDefinitionStart = false;
  }
  const StenoState *firstState = segments[0].state;
  for (const StenoSegment &segment : segments) {
    (*this)[startingOffset + (segment.state - firstState)]
        .state.isDefinitionStart = true;
  }
}

size_t StenoStrokeHistory::GetStartingStroke(size_t maximumCount) const {
  size_t count = GetCount();
  if (maximumCount >= count) {
    return 0;
  }
  for (size_t i = count - maximumCount; i < count; ++i) {
    if ((*this)[i].state.isDefinitionStart) {
      return i;
    }
  }
  return count - maximumCount;
}

//---------------------------------------------------------------------------
