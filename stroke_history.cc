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

size_t StenoStrokeHistory::GetUndoCount() const {
  const size_t count = GetCount();
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
    const size_t strokeIndex = segment.GetStrokeIndex(firstState);
    StenoState &state = (*this)[startingOffset + strokeIndex].state;
    state.lookupType = segment.lookupType;

    const char *lookupText = segment.lookup.GetText();
    if (lookupText[0] == '{') {
      state.requestsHistoryExtending =
          lookupText[1] == ':' && lookupText[2] == '=';
      state.isSpace = Str::IsSpace(lookupText);
      state.isHistoryExtending = Str::IsFingerSpellingCommand(lookupText);
      state.isSuffix =
          lookupText[1] == '^'; // Str::HasPrefix(lookupText, "{^");
      state.isNonAffixCommand =
          !state.isSuffix && lookupText[Str::Length(lookupText) - 2] != '^';
    } else {
      state.requestsHistoryExtending = false;
      state.isSpace = false;
      state.isHistoryExtending = false;
      state.isSuffix = false;
      state.isNonAffixCommand = false;
    }
  }
}

void StenoStrokeHistory::MarkLastLookupTypeAsUnknown() {
  for (size_t i = 1; i <= GetCount(); ++i) {
    StenoState &state = Back(i).state;
    if (state.IsDefinitionStart()) {
      state.lookupType = SegmentLookupType::UNKNOWN;
      return;
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

size_t StenoStrokeHistory::GetStartingStrokeAfterUndo(size_t undoCount) const {
  const size_t count = GetCount();
  for (size_t i = undoCount; i < count; ++i) {
    const StenoState state = Back(i + 1).state;
    if (state.IsDefinitionStart() && !state.isSuffix) {
      return count - (i + 1);
    }
  }
  return 0;
}

size_t StenoStrokeHistory::GetIndexOfWordStart(size_t index) const {
  size_t i = index;
  while (i > 0 && !(*this)[i].state.IsDefinitionStart()) {
    --i;
  }
  return i;
}

//---------------------------------------------------------------------------
