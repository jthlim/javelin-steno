//---------------------------------------------------------------------------

#include "stroke_history.h"
#include "dictionary/dictionary.h"
#include "orthography.h"
#include "segment.h"
#include "str.h"

//---------------------------------------------------------------------------

BuildSegmentContext::BuildSegmentContext(
    StenoSegmentList &segmentList, const StenoDictionary &dictionary,
    const StenoCompiledOrthography &orthography)
    : segmentList(segmentList), dictionary(dictionary),
      maximumOutlineLength(dictionary.GetMaximumOutlineLength()),
      orthography(orthography) {}

//---------------------------------------------------------------------------

void StenoStrokeHistory::Shift() {
  if (count == 0) {
    return;
  }
  --count;
  memmove(strokes, strokes + 1, count * sizeof(StenoStroke));
  memmove(states, states + 1, count * sizeof(StenoState));
}

size_t StenoStrokeHistory::GetUndoCount(size_t maxCount) const {
  if (count == 0) {
    return 0;
  }

  size_t result = 1;
  while (result < maxCount && result < count &&
         states[count - result].shouldCombineUndo) {
    ++result;
  }
  return result;
}

void StenoStrokeHistory::TransferFrom(const StenoStrokeHistory &source,
                                      size_t sourceStrokeCount,
                                      size_t maxCount) {
  size_t offset =
      sourceStrokeCount <= maxCount ? 0 : sourceStrokeCount - maxCount;
  count = sourceStrokeCount - offset;

  memcpy(strokes, source.strokes + offset, count * sizeof(StenoStroke));
  memcpy(states, source.states + offset, count * sizeof(StenoState));
}

void StenoStrokeHistory::CreateSegments(BuildSegmentContext &context,
                                        size_t minimumStartOffset) {
  AddSegments(context, minimumStartOffset);
}

void StenoStrokeHistory::AddSegments(BuildSegmentContext &context,
                                     size_t offset) {
  char buffer[32];

  while (offset < count) {
    if (DirectLookup(context, offset)) {
      continue;
    }
    if (AutoSuffixLookup(context, offset)) {
      continue;
    }

    strokes[offset].ToString(buffer);
    context.segmentList.Add(StenoSegment(
        1, states + offset,
        StenoDictionaryLookupResult::CreateDynamicString(Str::Dup(buffer))));
    ++offset;
  }
}

bool StenoStrokeHistory::DirectLookup(BuildSegmentContext &context,
                                      size_t &offset) {
  size_t startLength = count - offset;
  if (startLength > context.maximumOutlineLength) {
    startLength = context.maximumOutlineLength;
  }

  for (size_t length = startLength; length > 0; --length) {
    StenoDictionaryLookupResult lookup =
        context.dictionary.Lookup(strokes + offset, length);

    if (lookup.IsValid()) {
      const char *lookupText = lookup.GetText();
      if (lookupText[0] == '{' && lookupText[1] == '*') {
        if (lookupText[2] == '?' && lookupText[3] == '}') { // {*?}
          lookup.Destroy();
          RemoveOffset(context, offset, length);
          HandleRetroactiveInsertSpace(context, offset);
          ReevaluateSegments(context, offset);
          return true;
        } else if (lookupText[2] == '}') { // {*}
          lookup.Destroy();
          RemoveOffset(context, offset, length);
          HandleRetroactiveToggleAsterisk(context, offset);
          ReevaluateSegments(context, offset);
          return true;
        } else if (lookupText[2] == '+' && lookupText[3] == '}') { // {*+}
          StenoState state = states[offset];
          lookup.Destroy();
          RemoveOffset(context, offset, length);
          HandleRepeatLastStroke(context, offset, state);
          ReevaluateSegments(context, offset);
          return true;
        }
      }

      context.segmentList.Add(StenoSegment(length, states + offset, lookup));
      offset += length;
      return true;
    }
  }

  return false;
}

void StenoStrokeHistory::RemoveOffset(BuildSegmentContext &context,
                                      size_t &offset, size_t length) {
  assert(offset + length <= count);
  size_t remaining = count - offset - length;
  memmove(strokes + offset, strokes + offset + length,
          sizeof(StenoStroke) * remaining);
  memmove(states + offset, states + offset + length,
          sizeof(StenoState) * remaining);
  count -= length;
}

bool StenoStrokeHistory::AutoSuffixLookup(BuildSegmentContext &context,
                                          size_t &offset) {

  // See which historical translations can be extended with auto-suffixes.
  const StenoState *oldestState =
      offset < context.maximumOutlineLength
          ? states
          : states + offset + 1 - context.maximumOutlineLength;
  for (size_t segmentIndex = 0; segmentIndex < context.segmentList.GetCount();
       ++segmentIndex) {
    if (context.segmentList[segmentIndex].state < oldestState) {
      continue;
    }

    StenoSegment segment =
        AutoSuffixTest(context, context.segmentList[segmentIndex], offset);

    if (segment.IsValid()) {
      // There's a match! Pop off history.
      while (context.segmentList.GetCount() > segmentIndex + 1) {
        context.segmentList.Back().lookup.Destroy();
        context.segmentList.Pop();
      }
      context.segmentList.Back().lookup.Destroy();
      context.segmentList.Back() = segment;
      offset = segment.state - states + segment.strokeLength;
      return true;
    }
  }

  // No historical translations matched. Try auto suffixing at this offset.
  StenoSegment segment =
      AutoSuffixTest(context, offset,
                     count - offset > context.maximumOutlineLength
                         ? context.maximumOutlineLength
                         : count - offset,
                     1);

  if (!segment.IsValid()) {
    return false;
  }

  // Auto-suffix worked. Add it to the list.
  context.segmentList.Add(segment);
  offset += segment.strokeLength;
  return true;
}

StenoSegment StenoStrokeHistory::AutoSuffixTest(BuildSegmentContext &context,
                                                const StenoSegment &segment,
                                                size_t offset) {
  size_t lastStrokeOffset = segment.state - states;
  size_t startLength = count - lastStrokeOffset > context.maximumOutlineLength
                           ? context.maximumOutlineLength
                           : count - lastStrokeOffset;
  size_t minimumLength = offset - lastStrokeOffset + 1;
  return AutoSuffixTest(context, lastStrokeOffset, startLength, minimumLength);
}

StenoSegment StenoStrokeHistory::AutoSuffixTest(BuildSegmentContext &context,
                                                size_t offset,
                                                size_t startLength,
                                                size_t minimumLength) {
  StenoStroke *localStrokes =
      (StenoStroke *)alloca(sizeof(StenoStroke) * startLength);
  memcpy(localStrokes, strokes + offset, sizeof(StenoStroke) * startLength);

  const StenoOrthography &orthography = context.orthography.data;

  for (size_t length = startLength; length >= minimumLength; --length) {
    if ((strokes[offset + length - 1] & orthography.autoSuffixMask).IsEmpty()) {
      continue;
    }

    for (size_t i = 0; i < orthography.autoSuffixCount; ++i) {
      const StenoOrthographyAutoSuffix &suffix = orthography.autoSuffixes[i];
      if ((strokes[offset + length - 1] & suffix.stroke).IsEmpty()) {
        continue;
      }
      localStrokes[length - 1] = strokes[offset + length - 1] & ~suffix.stroke;

      StenoDictionaryLookupResult lookup =
          context.dictionary.Lookup(localStrokes, length);

      if (lookup.IsValid()) {
        const char *text = lookup.GetText();
        const char *result = Str::Join(text, suffix.text, nullptr);
        lookup.Destroy();
        return StenoSegment(
            length, states + offset,
            StenoDictionaryLookupResult::CreateDynamicString(result));
      }
    }
  }

  return StenoSegment(0, nullptr, StenoDictionaryLookupResult::CreateInvalid());
}

void StenoStrokeHistory::ReevaluateSegments(BuildSegmentContext &context,
                                            size_t &offset) {
  size_t currentOffset = offset;
  while (context.segmentList.IsNotEmpty()) {
    StenoSegment &lastSegment = context.segmentList.Back();
    size_t lastOffset = lastSegment.state - states;
    if (lastOffset + context.maximumOutlineLength < currentOffset) {
      return;
    }
    lastSegment.lookup.Destroy();
    context.segmentList.Pop();
    offset = lastOffset;
  }
}

void StenoStrokeHistory::HandleRetroactiveInsertSpace(
    BuildSegmentContext &context, size_t currentOffset) {
  if (currentOffset == 0) {
    return;
  }

  ++count;
  size_t remaining = count - currentOffset;
  memmove(strokes + currentOffset, strokes + currentOffset - 1,
          sizeof(StenoStroke) * remaining);
  memmove(states + currentOffset, states + currentOffset - 1,
          sizeof(StenoState) * remaining);

  strokes[currentOffset - 1] = StenoStroke(0);
}

void StenoStrokeHistory::HandleRetroactiveToggleAsterisk(
    BuildSegmentContext &context, size_t currentOffset) {
  if (currentOffset == 0) {
    return;
  }

  strokes[currentOffset - 1] ^= StrokeMask::STAR;
}

void StenoStrokeHistory::HandleRepeatLastStroke(BuildSegmentContext &context,
                                                size_t currentOffset,
                                                const StenoState &state) {
  if (currentOffset == 0) {
    return;
  }

  size_t remaining = count - currentOffset;
  memmove(strokes + currentOffset + 1, strokes + currentOffset,
          sizeof(StenoStroke) * remaining);
  memmove(states + currentOffset + 1, states + currentOffset,
          sizeof(StenoState) * remaining);

  strokes[currentOffset] = strokes[currentOffset - 1];
  states[currentOffset] = state;
  ++count;
}

//---------------------------------------------------------------------------

#include "dictionary/debug_dictionary.h"
#include "dictionary/dictionary_list.h"
#include "dictionary/emily_symbols_dictionary.h"
#include "dictionary/main_dictionary.h"
#include "dictionary/map_dictionary.h"
#include "orthography.h"
#include "str.h"
#include "unit_test.h"

constexpr StenoMapDictionary mainDictionary(MainDictionary::definition);

const StenoDictionary *const DICTIONARIES[] = {
    &StenoEmilySymbolsDictionary::instance,
    &mainDictionary,
};

TEST_BEGIN("StrokeHistory: Test single segment") {
  const StenoDictionaryList dictionary(DICTIONARIES, 2);

  StenoStrokeHistory history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary, orthography);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 1);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
}
TEST_END

TEST_BEGIN("StrokeHistory: Test two segments, with multi-stroke") {
  const StenoDictionaryList dictionary(DICTIONARIES, 2);

  StenoStrokeHistory history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("-D"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary, orthography);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 2);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), "tested"));
}
TEST_END

TEST_BEGIN("StrokeHistory: Test *? splits strokes") {
  const StenoDictionaryList dictionary(DICTIONARIES, 2);

  StenoStrokeHistory history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("-D"), StenoState());
  history.Add(StenoStroke("SKWHU"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary, orthography);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 3);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "test"));
  assert(Str::Eq(segmentList[1].lookup.GetText(), ""));
  assert(Str::Eq(segmentList[2].lookup.GetText(), "-D"));
}
TEST_END

TEST_BEGIN("StrokeHistory: Test * toggles ") {
  StenoDebugDictionary dictionary;
  dictionary.SetResponse("{*}");

  StenoStrokeHistory history;
  // spellchecker: disable
  history.Add(StenoStroke("TEFT"), StenoState());
  history.Add(StenoStroke("#EU"), StenoState());
  // spellchecker: enable

  StenoSegmentList segmentList;
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);

  BuildSegmentContext context(segmentList, dictionary, orthography);
  history.CreateSegments(context);

  assert(segmentList.GetCount() == 1);
  assert(Str::Eq(segmentList[0].lookup.GetText(), "T*EFT"));
}
TEST_END

//---------------------------------------------------------------------------
