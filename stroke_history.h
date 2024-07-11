//---------------------------------------------------------------------------

#pragma once
#include "cyclic_queue.h"
#include "state.h"
#include "stroke.h"
#include <assert.h>

//---------------------------------------------------------------------------

class StenoSegmentList;

//---------------------------------------------------------------------------

struct StenoStrokeHistoryEntry {
  StenoStroke stroke;
  StenoState state;
  size_t conversionCount;

  void Set(StenoStroke stroke, StenoState state, size_t conversionCount) {
    this->stroke = stroke;
    this->state = state;
    this->conversionCount = conversionCount;
  }
};

//---------------------------------------------------------------------------

const size_t STENO_STROKE_HISTORY_SIZE = 256;
class StenoStrokeHistory
    : public CyclicQueue<StenoStrokeHistoryEntry, STENO_STROKE_HISTORY_SIZE> {
public:
  void PruneIfFull() {
    if (IsFull()) {
      Prune();
    }
  }

  void Add(StenoStroke stroke, StenoState state, size_t conversionCount) {
    assert(IsNotFull());
    CyclicQueue::Add().Set(stroke, state, conversionCount);
  }

  // When undo is pressed, returns how many items should be removed
  // from the list up to maxCount.
  size_t GetUndoCount(size_t maxCount) const;

  void UpdateDefinitionBoundaries(size_t startingOffset,
                                  const StenoSegmentList &segments);
  size_t GetStartingStroke(size_t maximumCount) const;

  // Returns the offset of the word start. result <= index.
  size_t GetIndexOfWordStart(size_t index) const;

  void SetBackCombineUndo() { Back().state.shouldCombineUndo = true; }
  void SetBackNoCombineUndo() { Back().state.shouldCombineUndo = false; }
  void SetBackHasManualStateChange() {
    Back().state.isManualStateChange = true;
  }

private:
  void Prune();
};

//---------------------------------------------------------------------------
