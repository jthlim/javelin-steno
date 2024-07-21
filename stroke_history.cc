//---------------------------------------------------------------------------

#include "stroke_history.h"
#include "segment.h"

//---------------------------------------------------------------------------

void StenoStrokeHistory::Prune() {
  RemoveFront();
  while (IsNotEmpty() && !Front().state.IsDefinitionStart()) {
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

  const size_t count = GetCount();
  for (size_t i = startingOffset; i < count; ++i) {
    (*this)[i].state.lookupType = SegmentLookupType::UNKNOWN;
  }
  const StenoState *firstState = segments[0].state;
  for (const StenoSegment &segment : segments) {
    StenoState &state =
        (*this)[startingOffset + segment.GetStrokeIndex(firstState)].state;
    state.lookupType = segment.lookupType;

    const char *lookupText = segment.lookup.GetText();
    if (lookupText[0] == '{') {
      state.requestsHistoryExtending =
          lookupText[1] == ':' &&
          (Str::HasPrefix(lookupText, "{:==set_value") ||
           Str::HasPrefix(lookupText, "{:==retro_transform"));
      state.isSpace = Str::IsSpace(lookupText);
      state.isHistoryExtending =
          Str::IsFingerSpellingCommand(lookupText) ||
          segment.lookup == StenoDictionaryLookupResult::NO_OP;
      state.isSuffix = Str::HasPrefix(lookupText, "{^");
    } else {
      state.requestsHistoryExtending = false;
      state.isSpace = false;
      state.isHistoryExtending = false;
      state.isSuffix = false;
    }
  }
}

size_t StenoStrokeHistory::GetStartingStroke(size_t maximumCount) const {
  const size_t count = GetCount();
  if (maximumCount >= count) {
    return 0;
  }
  for (size_t i = count - maximumCount; i < count; ++i) {
    if ((*this)[i].state.IsDefinitionStart()) {
      return i;
    }
  }
  return count - maximumCount;
}

size_t StenoStrokeHistory::GetIndexOfWordStart(size_t index) const {
  size_t i = index;
  while (i > 0 && !(*this)[i].state.IsDefinitionStart()) {
    --i;
  }
  return i;
}

//---------------------------------------------------------------------------
